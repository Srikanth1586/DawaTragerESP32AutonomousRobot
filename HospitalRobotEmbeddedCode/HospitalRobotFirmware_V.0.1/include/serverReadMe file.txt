# Medicine Delivery Robot Server Prototype

This is a tiny working prototype for a hospital medicine delivery robot workflow using an ESP32 robot as a client.

## Where to deploy the server

Deploy the server on a hospital PC, Raspberry Pi, local mini-server, or private cloud VM on the hospital network. Do not deploy the main server on the ESP32 robot.

The ESP32 should run the robot control client:

- Poll the server for its assigned task.
- Move to the pharmacy.
- Wait until the pharmacist dispatches the task.
- Navigate corridors.
- Scan NFC tags near rooms.
- Confirm arrival when the scanned room matches the assigned destination.
- Release the medicine compartment after authorization.

## Run

```powershell
python server.py
```

Then open:

```text
http://localhost:3000
```

If you have Node.js and npm installed, you can also run:

```powershell
npm start
```

## Login pages

The app starts with one common login page. Demo credentials route each user to the correct workspace:

| Username | Password | Page |
|---|---|---|
| doctor | doctor123 | Doctor Page |
| pharmacy | pharmacy123 | Pharmacy Page |
| tracking | tracking123 | Tracking Page |
| robot | robot123 | Robot Page |
| history | history123 | Order History Tracking |
| admin | admin123 | Admin view for all pages |

## ESP32 WebSocket test

The admin ESP32 test page uses:

```text
ws://SERVER_IP:3000/ws/robot-chat
```

Robot heartbeat message:

```json
{
  "type": "heartbeat",
  "robotId": "RBT-001"
}
```

The heartbeat also registers the WebSocket connection as that robot, so messages sent to `RBT-001` are delivered only to the `RBT-001` connection.

Robot chat message:

```json
{
  "type": "robot_message",
  "robotId": "RBT-001",
  "message": "Hello from ESP32"
}
```

Robot order acknowledgement:

```json
{
  "type": "order_ack",
  "robotId": "RBT-001",
  "taskId": "TASK_ID_FROM_ORDER",
  "message": "Order acknowledged from ESP32"
}
```

## Prototype workflow

1. Doctor assigns medicine, patient, room number, and robot.
2. Pharmacist enters/scans the robot ID and sees the assigned patient and medicine details.
3. Pharmacist loads the medicine and marks the task as dispatched.
4. Robot receives immediate commands over its WebSocket (and can poll `/api/robots/:robotId/task` as a reconnect fallback).
5. Robot scans NFC room tags using `/api/robots/:robotId/nfc`.
6. When the tag maps to the destination room, the task becomes `arrived`.
7. Robot releases the compartment using `/api/tasks/:taskId/deliver`.

The dashboard includes a live hospital map with these tracking stages:

- Order placed
- Moving to pharmacy
- Order dispatched
- Medicine on wheels
- Medicine at door
- Delivered

## Robot API contract (FreeRTOS / ESP32)

Replace `SERVER_IP` with the IP address of the PC or Raspberry Pi running `server.py`. All HTTP bodies are JSON and use `Content-Type: application/json`.

### 1. Report every route NFC card read

```text
POST http://SERVER_IP:3000/api/robots/RBT-001/nfc
```

```json
{
  "tagId": "NFC-PHARMACY"
}
```

Send this immediately after the reader identifies a card. Valid configured examples are `NFC-PHARMACY`, `NFC-DOCK`, `NFC-A101`, `NFC-A102`, `NFC-B101`, `NFC-B102`, `NFC-B201`, and `NFC-B202`. Configure the value read from each physical card in `ROOM_NFC_TAGS` or `SPECIAL_NFC_TAGS` in `server.py`; for example, a card labelled `ROOM-1` can be configured as `"ROOM-1": "A101"`.

The server updates tracking automatically: Pharmacy makes the task `at_pharmacy`; a non-destination room makes it `in_transit`; the destination room makes it `arrived`; Dock completes it. A successful response is:

```json
{
  "task": {"id": "...", "status": "at_pharmacy", "lastScannedTag": "NFC-PHARMACY"},
  "matchedDestination": true,
  "place": "pharmacy"
}
```

### 2. Receive pharmacy door commands

Keep a WebSocket connected for the full robot session:

```text
ws://SERVER_IP:3000/ws/robot-chat
```

On connection, register the robot:

```json
{"type":"register","robotId":"RBT-001"}
```

When Pharmacy clicks **Open door**, the server sends this frame only to that robot:

```json
{
  "type": "robot_command",
  "commandId": "uuid",
  "robotId": "RBT-001",
  "command": "open_door",
  "task": null,
  "extra": {},
  "at": "2026-07-18T10:00:00Z"
}
```

When **Mark medicine loaded and dispatch** is clicked, the server sends these frames in order: first `close_door`, then `deliver_to_destination` (the latter includes the assigned task). Close the servo door when the first frame arrives; start motion only after it is closed.

After executing any command, acknowledge it:

```json
{"type":"command_ack","robotId":"RBT-001","commandId":"uuid","command":"close_door","result":"ok"}
```

The provided `esp32_firmware/src/main.cpp` already posts NFC reads, processes `open_door` / `close_door`, and sends this acknowledgement.

## Other example ESP32 API calls

```text
GET /api/robots/RBT-001/task
POST /api/robots/RBT-001/nfc
POST /api/tasks/<taskId>/deliver
```

NFC request body:

```json
{
  "tagId": "NFC-A101"
}
```

## Production architecture

- Server: hospital network server with database, authentication, audit logs, and dashboards.
- Doctor UI: prescription assignment.
- Pharmacist UI: scan robot QR, load medicine, dispatch.
- Robot API client: ESP32 Wi-Fi client calling server endpoints.
- Robot controller: ESP32 motor driver logic, NFC reader, obstacle sensors, battery monitor.
- Security: HTTPS, per-robot API tokens, user login, role-based access, audit trail.
- Reliability: task retry, robot heartbeat, emergency stop, low battery return-to-base.
