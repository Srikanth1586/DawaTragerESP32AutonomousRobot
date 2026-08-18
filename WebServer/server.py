from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from pathlib import Path
from urllib.parse import unquote, urlparse
import base64
import hashlib
import json
import os
import re
import secrets
import socket
import struct
import threading
import uuid
from datetime import datetime, timezone


ROOT = Path(__file__).parent.resolve()
PUBLIC_DIR = ROOT / "public"
DATA_FILE = ROOT / "tasks.json"
PORT = int(os.environ.get("PORT", "3000"))

ROBOTS = [
    {"id": "RBT-001", "name": "Robot 1", "status": "available"},
    {"id": "RBT-002", "name": "Robot 2", "status": "available"},
    {"id": "RBT-003", "name": "Robot 3", "status": "maintenance"},
]

ROOM_NFC_TAGS = {
    "NFC-A101": "A101",
    "NFC-A102": "A102",
    "NFC-B101": "B-101",
    "NFC-B102": "B-102",
    "NFC-B201": "B201",
    "NFC-B202": "B202",
}

MIME_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".svg": "image/svg+xml",
}

USERS = {
    "doctor": {"password": "doctor123", "role": "doctor", "name": "Doctor", "home": "/doctor.html"},
    "pharmacy": {"password": "pharmacy123", "role": "pharmacy", "name": "Pharmacist", "home": "/pharmacy.html"},
    "tracking": {"password": "tracking123", "role": "tracking", "name": "Tracking Desk", "home": "/tracking.html"},
    "robot": {"password": "robot123", "role": "robot", "name": "Robot Console", "home": "/robot.html"},
    "history": {"password": "history123", "role": "history", "name": "History Viewer", "home": "/history.html"},
    "admin": {"password": "admin123", "role": "admin", "name": "Admin", "home": "/admin.html"},
}

PROTECTED_PAGES = {
    "/doctor.html": {"doctor", "admin"},
    "/pharmacy.html": {"pharmacy", "admin"},
    "/tracking.html": {"tracking", "admin"},
    "/robot.html": {"robot", "admin"},
    "/history.html": {"history", "admin"},
    "/admin.html": {"admin"},
}

SESSIONS = {}
ROBOT_CHAT_MESSAGES = []
ROBOT_LAST_SEEN = {}
WS_CLIENTS = {}
WS_LOCK = threading.Lock()


def now_iso():
    return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")


def read_tasks():
    try:
        return json.loads(DATA_FILE.read_text(encoding="utf-8"))
    except Exception:
        return []


def write_tasks(tasks):
    DATA_FILE.write_text(json.dumps(tasks, indent=2), encoding="utf-8")


def add_robot_chat_message(sender, message, robot_id="RBT-001"):
    item = {
        "id": str(uuid.uuid4()),
        "sender": sender,
        "robotId": robot_id,
        "message": str(message).strip(),
        "at": now_iso(),
    }
    ROBOT_CHAT_MESSAGES.append(item)
    del ROBOT_CHAT_MESSAGES[:-100]
    return item


def robot_chat_snapshot():
    return {
        "messages": ROBOT_CHAT_MESSAGES,
        "robots": [
            {
                "id": robot["id"],
                "name": robot["name"],
                "alive": robot["id"] in ROBOT_LAST_SEEN,
                "lastSeen": ROBOT_LAST_SEEN.get(robot["id"]),
            }
            for robot in ROBOTS
        ],
    }


def websocket_accept_key(client_key):
    magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
    digest = hashlib.sha1((client_key + magic).encode("ascii")).digest()
    return base64.b64encode(digest).decode("ascii")


def encode_ws_message(payload):
    data = json.dumps(payload).encode("utf-8")
    length = len(data)
    if length < 126:
        header = struct.pack("!BB", 0x81, length)
    elif length < 65536:
        header = struct.pack("!BBH", 0x81, 126, length)
    else:
        header = struct.pack("!BBQ", 0x81, 127, length)
    return header + data


def read_exact(connection, length):
    chunks = bytearray()
    while len(chunks) < length:
        chunk = connection.recv(length - len(chunks))
        if not chunk:
            return None
        chunks.extend(chunk)
    return bytes(chunks)


def read_ws_message(connection):
    header = read_exact(connection, 2)
    if not header:
        return None

    first, second = header
    opcode = first & 0x0F
    masked = second & 0x80
    length = second & 0x7F

    if opcode == 0x8:
        return {"type": "_close"}
    if opcode == 0x9:
        return {"type": "_ping"}
    if opcode != 0x1:
        return None

    if length == 126:
        length = struct.unpack("!H", read_exact(connection, 2))[0]
    elif length == 127:
        length = struct.unpack("!Q", read_exact(connection, 8))[0]

    mask = read_exact(connection, 4) if masked else b"\x00\x00\x00\x00"
    payload = read_exact(connection, length)
    if payload is None:
        return None

    decoded = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
    return json.loads(decoded.decode("utf-8"))


def send_ws(connection, payload):
    connection.sendall(encode_ws_message(payload))


def broadcast_ws(payload, target_robot_id=None, audience="all"):
    message = encode_ws_message(payload)
    stale_clients = []
    with WS_LOCK:
        clients = list(WS_CLIENTS.items())

    for client, info in clients:
        kind = info.get("kind", "unknown")
        robot_id = info.get("robotId")
        should_send = True

        if target_robot_id is not None:
            should_send = kind == "admin" or (kind == "robot_device" and robot_id == target_robot_id)
        elif audience == "admin":
            should_send = kind == "admin"
        elif audience == "ui":
            should_send = kind in ["admin", "ui"]

        if not should_send:
            continue

        try:
            client.sendall(message)
        except OSError:
            stale_clients.append(client)

    if stale_clients:
        with WS_LOCK:
            for client in stale_clients:
                WS_CLIENTS.pop(client, None)


def record_robot_heartbeat(robot_id):
    ROBOT_LAST_SEEN[robot_id] = now_iso()
    item = add_robot_chat_message("system", "Robot alive", robot_id)
    broadcast_ws({"type": "heartbeat", "robotId": robot_id, "message": item, **robot_chat_snapshot()}, audience="admin")
    return item


def record_robot_message(robot_id, message):
    ROBOT_LAST_SEEN[robot_id] = now_iso()
    item = add_robot_chat_message("robot", message, robot_id)
    broadcast_ws({"type": "message", "message": item, **robot_chat_snapshot()}, audience="admin")
    return item


def record_admin_message(robot_id, message):
    item = add_robot_chat_message("admin", message, robot_id)
    broadcast_ws({"type": "message", "message": item, **robot_chat_snapshot()}, target_robot_id=robot_id)
    return item


def task_snapshot(tasks):
    return {
        "robots": update_robot_statuses(tasks),
        "roomNfcTags": ROOM_NFC_TAGS,
        "tasks": tasks,
    }


def broadcast_task_update(tasks):
    broadcast_ws({"type": "task_update", **task_snapshot(tasks)}, audience="ui")


def format_order_message(task):
    return (
        f"New order assigned. Task {task['id']} | Patient {task['patientName']} "
        f"({task['patientId']}) | Room {task['roomNumber']} | "
        f"Medicine {task['medicine']} | Dosage {task['dosage']} | Notes {task.get('notes') or 'None'}"
    )


def record_order_for_robot(task):
    item = add_robot_chat_message("server", format_order_message(task), task["robotId"])
    broadcast_ws({
        "type": "robot_order",
        "robotId": task["robotId"],
        "order": task,
        "message": item,
        **robot_chat_snapshot(),
    }, target_robot_id=task["robotId"])
    return item


def update_robot_statuses(tasks):
    active_robot_ids = {
        task["robotId"]
        for task in tasks
        if task.get("status") not in ["delivered", "cancelled"]
    }

    updated = []
    for robot in ROBOTS:
        if robot["status"] == "maintenance":
            updated.append(robot.copy())
        else:
            item = robot.copy()
            item["status"] = "busy" if robot["id"] in active_robot_ids else "available"
            updated.append(item)
    return updated


class RobotServer(BaseHTTPRequestHandler):
    server_version = "MedicineRobotPrototype/0.1"

    def do_GET(self):
        parsed = urlparse(self.path)
        if parsed.path == "/ws/robot-chat":
            self.handle_robot_chat_websocket()
        elif self.path.startswith("/api/"):
            self.handle_api()
        else:
            self.serve_static()

    def do_POST(self):
        if self.path.startswith("/api/"):
            self.handle_api()
        else:
            self.send_json(404, {"error": "Route not found"})

    def log_message(self, format, *args):
        print("%s - %s" % (self.address_string(), format % args))

    def read_json_body(self):
        length = int(self.headers.get("Content-Length", "0"))
        if length == 0:
            return {}
        body = self.rfile.read(length).decode("utf-8")
        return json.loads(body)

    def current_user(self):
        cookie_header = self.headers.get("Cookie", "")
        cookies = {}
        for part in cookie_header.split(";"):
            if "=" in part:
                key, value = part.strip().split("=", 1)
                cookies[key] = value
        session_id = cookies.get("robot_session")
        return SESSIONS.get(session_id)

    def send_redirect(self, location):
        self.send_response(302)
        self.send_header("Location", location)
        self.end_headers()

    def send_json(self, status, data):
        payload = json.dumps(data).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)

    def handle_robot_chat_websocket(self):
        client_key = self.headers.get("Sec-WebSocket-Key")
        if not client_key:
            self.send_json(400, {"error": "Missing WebSocket key"})
            return

        self.send_response(101, "Switching Protocols")
        self.send_header("Upgrade", "websocket")
        self.send_header("Connection", "Upgrade")
        self.send_header("Sec-WebSocket-Accept", websocket_accept_key(client_key))
        self.end_headers()

        connection = self.connection
        connection.settimeout(None)
        user = self.current_user()
        client_info = {
            "kind": "admin" if user is not None and user.get("role") == "admin" else "ui" if user is not None else "unknown",
            "role": user.get("role") if user is not None else None,
            "robotId": None,
        }
        with WS_LOCK:
            WS_CLIENTS[connection] = client_info

        try:
            if client_info["kind"] == "admin":
                send_ws(connection, {"type": "snapshot", **robot_chat_snapshot()})
            while True:
                payload = read_ws_message(connection)
                if payload is None:
                    break
                if payload.get("type") == "_close":
                    connection.sendall(b"\x88\x00")
                    break
                if payload.get("type") == "_ping":
                    continue

                message_type = str(payload.get("type", "")).strip()
                robot_id = str(payload.get("robotId", "RBT-001")).strip() or "RBT-001"

                if message_type in ["register", "heartbeat", "robot_message", "order_ack"]:
                    with WS_LOCK:
                        if connection in WS_CLIENTS:
                            WS_CLIENTS[connection]["kind"] = "robot_device"
                            WS_CLIENTS[connection]["robotId"] = robot_id

                if message_type == "register":
                    send_ws(connection, {"type": "registered", "robotId": robot_id})
                elif message_type == "heartbeat":
                    record_robot_heartbeat(robot_id)
                    send_ws(connection, {"type": "heartbeat_ack", "robotId": robot_id, "at": ROBOT_LAST_SEEN[robot_id]})
                elif message_type == "robot_message":
                    message = str(payload.get("message", "")).strip()
                    if message:
                        record_robot_message(robot_id, message)
                elif message_type == "order_ack":
                    task_id = str(payload.get("taskId", "")).strip()
                    message = str(payload.get("message", "")).strip()
                    ack_message = message or f"Order acknowledged: {task_id or 'latest task'}"
                    record_robot_message(robot_id, ack_message)
                elif message_type == "admin_message":
                    user = self.current_user()
                    if user is not None and user["role"] == "admin":
                        message = str(payload.get("message", "")).strip()
                        if message:
                            record_admin_message(robot_id, message)
        except (ConnectionError, OSError, socket.timeout, json.JSONDecodeError):
            pass
        finally:
            with WS_LOCK:
                WS_CLIENTS.pop(connection, None)

    def handle_api(self):
        parsed = urlparse(self.path)
        path = parsed.path
        method = self.command
        tasks = read_tasks()

        try:
            if method == "GET" and path == "/api/me":
                user = self.current_user()
                if user is None:
                    self.send_json(401, {"error": "Not logged in"})
                    return
                self.send_json(200, {"user": user})
                return

            if method == "POST" and path == "/api/login":
                body = self.read_json_body()
                username = str(body.get("username", "")).strip().lower()
                password = str(body.get("password", ""))
                account = USERS.get(username)

                if account is None or account["password"] != password:
                    self.send_json(401, {"error": "Invalid username or password"})
                    return

                session_id = secrets.token_urlsafe(32)
                user = {
                    "username": username,
                    "role": account["role"],
                    "name": account["name"],
                    "home": account["home"],
                }
                SESSIONS[session_id] = user

                payload = json.dumps({"user": user, "redirect": account["home"]}).encode("utf-8")
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Cache-Control", "no-store")
                self.send_header("Set-Cookie", f"robot_session={session_id}; HttpOnly; SameSite=Lax; Path=/")
                self.send_header("Content-Length", str(len(payload)))
                self.end_headers()
                self.wfile.write(payload)
                return

            if method == "POST" and path == "/api/logout":
                cookie_header = self.headers.get("Cookie", "")
                for part in cookie_header.split(";"):
                    if part.strip().startswith("robot_session="):
                        session_id = part.strip().split("=", 1)[1]
                        SESSIONS.pop(session_id, None)
                payload = json.dumps({"ok": True}).encode("utf-8")
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Set-Cookie", "robot_session=; Max-Age=0; SameSite=Lax; Path=/")
                self.send_header("Content-Length", str(len(payload)))
                self.end_headers()
                self.wfile.write(payload)
                return

            if method == "GET" and path == "/api/admin/robot-chat":
                user = self.current_user()
                if user is None or user["role"] != "admin":
                    self.send_json(403, {"error": "Admin access required"})
                    return

                self.send_json(200, robot_chat_snapshot())
                return

            if method == "POST" and path == "/api/admin/robot-chat/send":
                user = self.current_user()
                if user is None or user["role"] != "admin":
                    self.send_json(403, {"error": "Admin access required"})
                    return

                body = self.read_json_body()
                message = str(body.get("message", "")).strip()
                robot_id = str(body.get("robotId", "RBT-001")).strip() or "RBT-001"
                if not message:
                    self.send_json(400, {"error": "Message is required"})
                    return

                item = record_admin_message(robot_id, message)
                self.send_json(201, item)
                return

            heartbeat_match = re.fullmatch(r"/api/robots/([^/]+)/heartbeat", path)
            if method == "POST" and heartbeat_match:
                robot_id = unquote(heartbeat_match.group(1))
                item = record_robot_heartbeat(robot_id)
                self.send_json(200, {"ok": True, "robotId": robot_id, "message": item})
                return

            robot_message_match = re.fullmatch(r"/api/robots/([^/]+)/message", path)
            if method == "POST" and robot_message_match:
                robot_id = unquote(robot_message_match.group(1))
                body = self.read_json_body()
                message = str(body.get("message", "")).strip()
                if not message:
                    self.send_json(400, {"error": "Message is required"})
                    return

                item = record_robot_message(robot_id, message)
                self.send_json(201, item)
                return

            reset_match = re.fullmatch(r"/api/robots/([^/]+)/reset", path)
            if method == "POST" and reset_match:
                robot_id = unquote(reset_match.group(1))
                task = next(
                    (
                        item for item in tasks
                        if item["robotId"] == robot_id
                        and item.get("status") not in ["delivered", "cancelled"]
                    ),
                    None,
                )

                if task is None:
                    self.send_json(200, {"ok": True, "robotId": robot_id, "resetTask": None})
                    return

                timestamp = now_iso()
                task["status"] = "cancelled"
                task["cancelledAt"] = timestamp
                task["updatedAt"] = timestamp
                task["route"].append({
                    "status": "cancelled",
                    "at": timestamp,
                    "note": "Robot reset to available mode",
                })
                write_tasks(tasks)
                broadcast_task_update(tasks)
                self.send_json(200, {"ok": True, "robotId": robot_id, "resetTask": task})
                return

            if method == "GET" and path == "/api/bootstrap":
                self.send_json(200, task_snapshot(tasks))
                return

            if method == "GET" and path == "/api/tasks":
                self.send_json(200, tasks)
                return

            if method == "POST" and path == "/api/tasks":
                body = self.read_json_body()
                required = [
                    "patientName",
                    "patientId",
                    "roomNumber",
                    "medicine",
                    "dosage",
                    "doctorName",
                    "robotId",
                ]
                missing = [field for field in required if not str(body.get(field, "")).strip()]
                if missing:
                    self.send_json(400, {"error": "Missing fields: " + ", ".join(missing)})
                    return

                robot = next((item for item in ROBOTS if item["id"] == body["robotId"]), None)
                if robot is None or robot["status"] == "maintenance":
                    self.send_json(400, {"error": "Selected robot is unavailable"})
                    return

                busy_robot = next(
                    (
                        task for task in tasks
                        if task["robotId"] == body["robotId"]
                        and task.get("status") not in ["delivered", "cancelled"]
                    ),
                    None,
                )
                if busy_robot:
                    self.send_json(409, {"error": "Selected robot already has an active task"})
                    return

                timestamp = now_iso()
                task = {
                    "id": str(uuid.uuid4()),
                    "patientName": str(body["patientName"]).strip(),
                    "patientId": str(body["patientId"]).strip(),
                    "roomNumber": str(body["roomNumber"]).strip().upper(),
                    "medicine": str(body["medicine"]).strip(),
                    "dosage": str(body["dosage"]).strip(),
                    "notes": str(body.get("notes", "")).strip(),
                    "doctorName": str(body["doctorName"]).strip(),
                    "robotId": str(body["robotId"]).strip(),
                    "status": "moving_to_pharmacy",
                    "route": [
                        {"status": "assigned", "at": timestamp, "note": "Order placed by doctor"},
                        {"status": "moving_to_pharmacy", "at": timestamp, "note": "Robot moving to pharmacy"},
                    ],
                    "createdAt": timestamp,
                    "updatedAt": timestamp,
                }

                tasks.insert(0, task)
                write_tasks(tasks)
                record_order_for_robot(task)
                broadcast_task_update(tasks)
                self.send_json(201, task)
                return

            dispatch_match = re.fullmatch(r"/api/tasks/([^/]+)/dispatch", path)
            if method == "POST" and dispatch_match:
                body = self.read_json_body()
                task = next((item for item in tasks if item["id"] == dispatch_match.group(1)), None)
                if task is None:
                    self.send_json(404, {"error": "Task not found"})
                    return
                if task["status"] not in ["assigned", "moving_to_pharmacy", "at_pharmacy"]:
                    self.send_json(409, {"error": f"Task cannot be dispatched from status {task['status']}"})
                    return

                timestamp = now_iso()
                task["status"] = "dispatched"
                task["pharmacistName"] = str(body.get("pharmacistName", "Pharmacist")).strip()
                task["dispatchedAt"] = timestamp
                task["updatedAt"] = timestamp
                task["route"].append({
                    "status": "dispatched",
                    "at": timestamp,
                    "note": f"Medicine loaded by {task['pharmacistName']}",
                })
                write_tasks(tasks)
                broadcast_task_update(tasks)
                self.send_json(200, task)
                return

            robot_task_match = re.fullmatch(r"/api/robots/([^/]+)/task", path)
            if method == "GET" and robot_task_match:
                robot_id = unquote(robot_task_match.group(1))
                task = next(
                    (
                        item for item in tasks
                        if item["robotId"] == robot_id
                        and item.get("status") not in ["delivered", "cancelled"]
                    ),
                    None,
                )
                self.send_json(200, {"robotId": robot_id, "task": task})
                return

            nfc_match = re.fullmatch(r"/api/robots/([^/]+)/nfc", path)
            if method == "POST" and nfc_match:
                robot_id = unquote(nfc_match.group(1))
                body = self.read_json_body()
                tag_id = str(body.get("tagId", "")).strip().upper()
                room_number = ROOM_NFC_TAGS.get(tag_id)
                task = next(
                    (
                        item for item in tasks
                        if item["robotId"] == robot_id
                        and item.get("status") not in ["delivered", "cancelled"]
                    ),
                    None,
                )

                if task is None:
                    self.send_json(404, {"error": "No active task for robot"})
                    return
                if room_number is None:
                    self.send_json(404, {"error": "Unknown NFC tag"})
                    return

                timestamp = now_iso()
                task["lastScannedTag"] = tag_id
                task["lastScannedRoom"] = room_number
                task["updatedAt"] = timestamp
                matched = room_number == task["roomNumber"]

                if matched:
                    task["status"] = "arrived"
                    task["arrivedAt"] = timestamp
                    task["route"].append({
                        "status": "arrived",
                        "at": timestamp,
                        "note": f"Robot reached room {room_number}",
                    })
                else:
                    task["status"] = "in_transit"
                    task["route"].append({
                        "status": "in_transit",
                        "at": timestamp,
                        "note": f"Robot passed room {room_number}",
                    })

                write_tasks(tasks)
                broadcast_task_update(tasks)
                self.send_json(200, {"task": task, "matchedDestination": matched})
                return

            deliver_match = re.fullmatch(r"/api/tasks/([^/]+)/deliver", path)
            if method == "POST" and deliver_match:
                task = next((item for item in tasks if item["id"] == deliver_match.group(1)), None)
                if task is None:
                    self.send_json(404, {"error": "Task not found"})
                    return
                if task["status"] != "arrived":
                    self.send_json(409, {"error": "Robot must arrive at the destination before delivery"})
                    return

                timestamp = now_iso()
                task["status"] = "delivered"
                task["deliveredAt"] = timestamp
                task["updatedAt"] = timestamp
                task["route"].append({
                    "status": "delivered",
                    "at": timestamp,
                    "note": "Medicine compartment released",
                })
                write_tasks(tasks)
                broadcast_task_update(tasks)
                self.send_json(200, task)
                return

            self.send_json(404, {"error": "API route not found"})
        except json.JSONDecodeError:
            self.send_json(400, {"error": "Invalid JSON body"})
        except Exception as error:
            self.send_json(500, {"error": str(error)})

    def serve_static(self):
        parsed = urlparse(self.path)
        requested = unquote(parsed.path)
        if requested == "/":
            requested = "/index.html"

        protected_path = requested if requested.startswith("/") else "/" + requested
        if protected_path in PROTECTED_PAGES:
            user = self.current_user()
            if user is None:
                self.send_redirect("/")
                return
            allowed_roles = PROTECTED_PAGES[protected_path]
            if user["role"] not in allowed_roles:
                self.send_redirect(user.get("home", "/"))
                return

        file_path = (PUBLIC_DIR / requested.lstrip("/")).resolve()
        if PUBLIC_DIR not in file_path.parents and file_path != PUBLIC_DIR:
            self.send_response(403)
            self.end_headers()
            self.wfile.write(b"Forbidden")
            return

        if not file_path.exists() or not file_path.is_file():
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"Not found")
            return

        content = file_path.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", MIME_TYPES.get(file_path.suffix, "application/octet-stream"))
        self.send_header("Content-Length", str(len(content)))
        self.end_headers()
        self.wfile.write(content)


if __name__ == "__main__":
    server = ThreadingHTTPServer(("0.0.0.0", PORT), RobotServer)
    print(f"Medicine delivery robot server running at http://localhost:{PORT}")
    server.serve_forever()
