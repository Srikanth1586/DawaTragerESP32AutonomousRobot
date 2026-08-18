const http = require("http");
const fs = require("fs");
const path = require("path");
const crypto = require("crypto");

const PORT = Number(process.env.PORT || 3000);
const PUBLIC_DIR = path.join(__dirname, "public");
const DATA_FILE = path.join(__dirname, "tasks.json");

const robots = [
  { id: "RBT-001", name: "Robot 1", status: "available" },
  { id: "RBT-002", name: "Robot 2", status: "available" },
  { id: "RBT-003", name: "Robot 3", status: "maintenance" }
];

const roomNfcTags = {
  "NFC-A101": "A101",
  "NFC-A102": "A102",
  "NFC-B201": "B201",
  "NFC-B202": "B202"
};

const mimeTypes = {
  ".html": "text/html; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".js": "application/javascript; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".svg": "image/svg+xml"
};

function readTasks() {
  try {
    return JSON.parse(fs.readFileSync(DATA_FILE, "utf8"));
  } catch (error) {
    return [];
  }
}

function writeTasks(tasks) {
  fs.writeFileSync(DATA_FILE, JSON.stringify(tasks, null, 2));
}

function sendJson(res, statusCode, data) {
  res.writeHead(statusCode, {
    "Content-Type": "application/json; charset=utf-8",
    "Cache-Control": "no-store"
  });
  res.end(JSON.stringify(data));
}

function readBody(req) {
  return new Promise((resolve, reject) => {
    let body = "";
    req.on("data", chunk => {
      body += chunk;
      if (body.length > 1_000_000) {
        req.destroy();
        reject(new Error("Request body too large"));
      }
    });
    req.on("end", () => {
      if (!body) {
        resolve({});
        return;
      }

      try {
        resolve(JSON.parse(body));
      } catch (error) {
        reject(new Error("Invalid JSON body"));
      }
    });
  });
}

function requiredFields(body, fields) {
  return fields.filter(field => !String(body[field] || "").trim());
}

function updateRobotStatuses(tasks) {
  const activeRobotIds = new Set(
    tasks
      .filter(task => !["delivered", "cancelled"].includes(task.status))
      .map(task => task.robotId)
  );

  return robots.map(robot => {
    if (robot.status === "maintenance") return robot;
    return {
      ...robot,
      status: activeRobotIds.has(robot.id) ? "busy" : "available"
    };
  });
}

async function handleApi(req, res) {
  const url = new URL(req.url, `http://${req.headers.host}`);
  const tasks = readTasks();

  if (req.method === "GET" && url.pathname === "/api/bootstrap") {
    sendJson(res, 200, {
      robots: updateRobotStatuses(tasks),
      roomNfcTags,
      tasks
    });
    return;
  }

  if (req.method === "GET" && url.pathname === "/api/tasks") {
    sendJson(res, 200, tasks);
    return;
  }

  if (req.method === "POST" && url.pathname === "/api/tasks") {
    const body = await readBody(req);
    const missing = requiredFields(body, [
      "patientName",
      "patientId",
      "roomNumber",
      "medicine",
      "dosage",
      "doctorName",
      "robotId"
    ]);

    if (missing.length) {
      sendJson(res, 400, { error: `Missing fields: ${missing.join(", ")}` });
      return;
    }

    const robot = robots.find(item => item.id === body.robotId);
    if (!robot || robot.status === "maintenance") {
      sendJson(res, 400, { error: "Selected robot is unavailable" });
      return;
    }

    const busyRobot = tasks.find(
      task => task.robotId === body.robotId && !["delivered", "cancelled"].includes(task.status)
    );
    if (busyRobot) {
      sendJson(res, 409, { error: "Selected robot already has an active task" });
      return;
    }

    const task = {
      id: crypto.randomUUID(),
      patientName: String(body.patientName).trim(),
      patientId: String(body.patientId).trim(),
      roomNumber: String(body.roomNumber).trim().toUpperCase(),
      medicine: String(body.medicine).trim(),
      dosage: String(body.dosage).trim(),
      notes: String(body.notes || "").trim(),
      doctorName: String(body.doctorName).trim(),
      robotId: String(body.robotId).trim(),
      status: "moving_to_pharmacy",
      route: [
        { status: "assigned", at: new Date().toISOString(), note: "Order placed by doctor" },
        { status: "moving_to_pharmacy", at: new Date().toISOString(), note: "Robot moving to pharmacy" }
      ],
      createdAt: new Date().toISOString(),
      updatedAt: new Date().toISOString()
    };

    tasks.unshift(task);
    writeTasks(tasks);
    sendJson(res, 201, task);
    return;
  }

  const dispatchMatch = url.pathname.match(/^\/api\/tasks\/([^/]+)\/dispatch$/);
  if (req.method === "POST" && dispatchMatch) {
    const body = await readBody(req);
    const task = tasks.find(item => item.id === dispatchMatch[1]);
    if (!task) {
      sendJson(res, 404, { error: "Task not found" });
      return;
    }

    if (task.status !== "assigned" && task.status !== "moving_to_pharmacy" && task.status !== "at_pharmacy") {
      sendJson(res, 409, { error: `Task cannot be dispatched from status ${task.status}` });
      return;
    }

    task.status = "dispatched";
    task.pharmacistName = String(body.pharmacistName || "Pharmacist").trim();
    task.dispatchedAt = new Date().toISOString();
    task.updatedAt = task.dispatchedAt;
    task.route.push({
      status: "dispatched",
      at: task.dispatchedAt,
      note: `Medicine loaded by ${task.pharmacistName}`
    });

    writeTasks(tasks);
    sendJson(res, 200, task);
    return;
  }

  const robotTaskMatch = url.pathname.match(/^\/api\/robots\/([^/]+)\/task$/);
  if (req.method === "GET" && robotTaskMatch) {
    const robotId = decodeURIComponent(robotTaskMatch[1]);
    const task = tasks.find(
      item => item.robotId === robotId && !["delivered", "cancelled"].includes(item.status)
    );
    sendJson(res, 200, { robotId, task: task || null });
    return;
  }

  const nfcMatch = url.pathname.match(/^\/api\/robots\/([^/]+)\/nfc$/);
  if (req.method === "POST" && nfcMatch) {
    const robotId = decodeURIComponent(nfcMatch[1]);
    const body = await readBody(req);
    const tagId = String(body.tagId || "").trim().toUpperCase();
    const roomNumber = roomNfcTags[tagId];
    const task = tasks.find(
      item => item.robotId === robotId && !["delivered", "cancelled"].includes(item.status)
    );

    if (!task) {
      sendJson(res, 404, { error: "No active task for robot" });
      return;
    }

    if (!roomNumber) {
      sendJson(res, 404, { error: "Unknown NFC tag" });
      return;
    }

    const scannedAt = new Date().toISOString();
    task.lastScannedTag = tagId;
    task.lastScannedRoom = roomNumber;
    task.updatedAt = scannedAt;

    if (roomNumber === task.roomNumber) {
      task.status = "arrived";
      task.arrivedAt = scannedAt;
      task.route.push({ status: "arrived", at: scannedAt, note: `Robot reached room ${roomNumber}` });
    } else {
      task.status = "in_transit";
      task.route.push({ status: "in_transit", at: scannedAt, note: `Robot passed room ${roomNumber}` });
    }

    writeTasks(tasks);
    sendJson(res, 200, { task, matchedDestination: roomNumber === task.roomNumber });
    return;
  }

  const deliverMatch = url.pathname.match(/^\/api\/tasks\/([^/]+)\/deliver$/);
  if (req.method === "POST" && deliverMatch) {
    const task = tasks.find(item => item.id === deliverMatch[1]);
    if (!task) {
      sendJson(res, 404, { error: "Task not found" });
      return;
    }

    if (task.status !== "arrived") {
      sendJson(res, 409, { error: "Robot must arrive at the destination before delivery" });
      return;
    }

    task.status = "delivered";
    task.deliveredAt = new Date().toISOString();
    task.updatedAt = task.deliveredAt;
    task.route.push({
      status: "delivered",
      at: task.deliveredAt,
      note: "Medicine compartment released"
    });

    writeTasks(tasks);
    sendJson(res, 200, task);
    return;
  }

  sendJson(res, 404, { error: "API route not found" });
}

function serveStatic(req, res) {
  const url = new URL(req.url, `http://${req.headers.host}`);
  const safePath = path.normalize(url.pathname).replace(/^(\.\.[/\\])+/, "");
  const requestedPath = safePath === "/" ? "/index.html" : safePath;
  const filePath = path.join(PUBLIC_DIR, requestedPath);

  if (!filePath.startsWith(PUBLIC_DIR)) {
    res.writeHead(403);
    res.end("Forbidden");
    return;
  }

  fs.readFile(filePath, (error, content) => {
    if (error) {
      res.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
      res.end("Not found");
      return;
    }

    const ext = path.extname(filePath);
    res.writeHead(200, { "Content-Type": mimeTypes[ext] || "application/octet-stream" });
    res.end(content);
  });
}

const server = http.createServer((req, res) => {
  if (req.url.startsWith("/api/")) {
    handleApi(req, res).catch(error => {
      sendJson(res, 500, { error: error.message || "Server error" });
    });
    return;
  }

  serveStatic(req, res);
});

server.listen(PORT, () => {
  console.log(`Medicine delivery robot server running at http://localhost:${PORT}`);
});
