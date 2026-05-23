const state = {
  robots: [],
  roomNfcTags: {},
  tasks: [],
  selectedTrackingRobotId: "",
  user: null
};

const pageName = window.location.pathname.replace("/", "").replace(".html", "") || "login";
const isEmbedded = new URLSearchParams(window.location.search).get("embedded") === "1";
let taskSocket = null;
let taskReconnectTimer = null;

if (isEmbedded) {
  document.body.classList.add("embedded-page");
}

const statusLabels = {
  assigned: "Order placed",
  moving_to_pharmacy: "Moving to pharmacy",
  at_pharmacy: "At pharmacy",
  dispatched: "Order dispatched",
  in_transit: "Medicine on wheels",
  arrived: "Medicine at door",
  delivered: "Delivered",
  cancelled: "Cancelled"
};

const trackingSteps = [
  { key: "assigned", label: "Order placed" },
  { key: "moving_to_pharmacy", label: "Moving to pharmacy" },
  { key: "dispatched", label: "Order dispatched" },
  { key: "in_transit", label: "Medicine on wheels" },
  { key: "arrived", label: "Medicine at door" },
  { key: "delivered", label: "Delivered" }
];

const trackingRank = {
  assigned: 0,
  moving_to_pharmacy: 1,
  at_pharmacy: 1,
  dispatched: 2,
  in_transit: 3,
  arrived: 4,
  delivered: 5
};

const roomPositions = {
  A101: { x: 78, y: 24 },
  A102: { x: 78, y: 48 },
  B201: { x: 78, y: 70 },
  B202: { x: 78, y: 88 }
};

const pageLinks = [
  { role: "doctor", label: "Doctor", href: "/doctor.html" },
  { role: "pharmacy", label: "Pharmacy", href: "/pharmacy.html" },
  { role: "tracking", label: "Tracking", href: "/tracking.html" },
  { role: "robot", label: "Robot", href: "/robot.html" },
  { role: "history", label: "History", href: "/history.html" }
];

const els = {
  serverStatus: document.querySelector("#serverStatus"),
  appNav: document.querySelector("#appNav"),
  logoutButton: document.querySelector("#logoutButton"),
  mapSummary: document.querySelector("#mapSummary"),
  robotMarkers: document.querySelector("#robotMarkers"),
  trackingRobotSelect: document.querySelector("#trackingRobotSelect"),
  trackingSteps: document.querySelector("#trackingSteps"),
  robotStatusList: document.querySelector("#robotStatusList"),
  doctorForm: document.querySelector("#doctorForm"),
  roomSelect: document.querySelector("#roomSelect"),
  robotSelect: document.querySelector("#robotSelect"),
  robotLookup: document.querySelector("#robotLookup"),
  findRobotTask: document.querySelector("#findRobotTask"),
  pharmacyTask: document.querySelector("#pharmacyTask"),
  taskList: document.querySelector("#taskList"),
  refreshTasks: document.querySelector("#refreshTasks"),
  taskTemplate: document.querySelector("#taskTemplate"),
  robotConsoleSelect: document.querySelector("#robotConsoleSelect"),
  nfcTagSelect: document.querySelector("#nfcTagSelect"),
  scanNfc: document.querySelector("#scanNfc"),
  resetRobot: document.querySelector("#resetRobot"),
  robotConsole: document.querySelector("#robotConsole")
};

async function api(path, options = {}) {
  const response = await fetch(path, {
    headers: { "Content-Type": "application/json" },
    ...options
  });
  const data = await response.json();
  if (!response.ok) {
    throw new Error(data.error || "Request failed");
  }
  return data;
}

function escapeHtml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");
}

function formatTime(value) {
  return new Intl.DateTimeFormat(undefined, {
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit"
  }).format(new Date(value));
}

function setStatus(message, tone = "online") {
  if (!els.serverStatus) return;
  els.serverStatus.textContent = message;
  els.serverStatus.style.background = tone === "error" ? "#fee4e2" : "#e9f8f2";
  els.serverStatus.style.color = tone === "error" ? "#b42318" : "#12805c";
}

function renderNav() {
  if (!els.appNav || !state.user || isEmbedded) return;
  const links = state.user.role === "admin"
    ? pageLinks
    : pageLinks.filter(link => link.role === state.user.role);

  els.appNav.innerHTML = links
    .map(link => {
      const active = window.location.pathname === link.href ? "active" : "";
      return `<a class="${active}" href="${link.href}">${link.label}</a>`;
    })
    .join("");
}

function renderRobots() {
  const availableOptions = state.robots
    .map(robot => {
      const disabled = robot.status !== "available" ? "disabled" : "";
      return `<option value="${robot.id}" ${disabled}>${robot.id} - ${robot.name} (${robot.status})</option>`;
    })
    .join("");

  const allOptions = state.robots
    .map(robot => `<option value="${robot.id}">${robot.id} - ${robot.name}</option>`)
    .join("");

  if (els.robotSelect) {
    els.robotSelect.innerHTML = availableOptions;
  }
  if (els.robotConsoleSelect) {
    els.robotConsoleSelect.innerHTML = allOptions;
  }

  if (!state.selectedTrackingRobotId && state.robots.length) {
    const activeTask = state.tasks.find(task => task.status !== "delivered" && task.status !== "cancelled");
    state.selectedTrackingRobotId = activeTask?.robotId || state.robots[0].id;
  }

  if (els.trackingRobotSelect) {
    els.trackingRobotSelect.innerHTML = state.robots
      .map(robot => `<option value="${robot.id}">${robot.id} - ${robot.name}</option>`)
      .join("");
    els.trackingRobotSelect.value = state.selectedTrackingRobotId;
  }
}

function renderTags() {
  const tagEntries = Object.entries(state.roomNfcTags);

  if (els.nfcTagSelect) {
    els.nfcTagSelect.innerHTML = tagEntries
      .map(([tagId, room]) => `<option value="${tagId}">${tagId} - Room ${room}</option>`)
      .join("");
  }

  if (els.roomSelect) {
    const selectedRoom = els.roomSelect.value;
    const rooms = [...new Set(tagEntries.map(([, room]) => room))].sort();
    els.roomSelect.innerHTML = rooms
      .map(room => `<option value="${room}">Room ${room}</option>`)
      .join("");
    if (rooms.includes(selectedRoom)) {
      els.roomSelect.value = selectedRoom;
    }
  }
}

function markerPosition(task) {
  if (task.status === "assigned") return { x: 16, y: 24 };
  if (task.status === "moving_to_pharmacy" || task.status === "at_pharmacy") return { x: 36, y: 24 };
  if (task.status === "dispatched") return { x: 44, y: 50 };
  if (task.status === "in_transit") return { x: 58, y: 58 };
  if (task.status === "arrived" || task.status === "delivered") {
    return roomPositions[task.roomNumber] || { x: 78, y: 50 };
  }
  return { x: 12, y: 84 };
}

function renderTrackingSteps(activeTask) {
  if (!els.trackingSteps) return;
  const activeRank = activeTask ? trackingRank[activeTask.status] ?? 0 : -1;
  els.trackingSteps.innerHTML = trackingSteps
    .map((step, index) => {
      const stateClass = index < activeRank ? "done" : index === activeRank ? "active" : "";
      return `
        <div class="tracking-step ${stateClass}">
          <span class="step-dot">${index + 1}</span>
          <span>${step.label}</span>
        </div>
      `;
    })
    .join("");
}

function markerOffsets(tasks) {
  const groups = new Map();
  for (const task of tasks) {
    const position = markerPosition(task);
    const key = `${Math.round(position.x)}-${Math.round(position.y)}`;
    groups.set(key, [...(groups.get(key) || []), task.id]);
  }

  const offsets = new Map();
  for (const ids of groups.values()) {
    const spacing = 4;
    const start = -((ids.length - 1) * spacing) / 2;
    ids.forEach((id, index) => {
      offsets.set(id, {
        x: start + index * spacing,
        y: index % 2 === 0 ? -1.5 : 1.5
      });
    });
  }
  return offsets;
}

function renderMap() {
  if (!els.robotMarkers || !els.mapSummary) return;

  const activeTasks = state.tasks.filter(task => task.status !== "delivered" && task.status !== "cancelled");
  const selectedTask = state.tasks.find(task => task.robotId === state.selectedTrackingRobotId) || null;
  const focusTask = selectedTask || activeTasks[0] || state.tasks[0] || null;
  const offsets = markerOffsets(activeTasks);

  if (!state.selectedTrackingRobotId && focusTask) {
    state.selectedTrackingRobotId = focusTask.robotId;
    if (els.trackingRobotSelect) {
      els.trackingRobotSelect.value = focusTask.robotId;
    }
  }

  els.mapSummary.textContent = activeTasks.length
    ? `${activeTasks.length} active delivery${activeTasks.length === 1 ? "" : "ies"}`
    : "No active deliveries";

  renderTrackingSteps(focusTask);

  els.robotMarkers.innerHTML = activeTasks
    .map(task => {
      const position = markerPosition(task);
      const offset = offsets.get(task.id) || { x: 0, y: 0 };
      const selectedClass = task.robotId === state.selectedTrackingRobotId ? "selected" : "";
      return `
        <div class="robot-marker status-${task.status} ${selectedClass}" style="left:${position.x + offset.x}%; top:${position.y + offset.y}%;">
          <span class="robot-icon">R</span>
          <span class="robot-bubble">
            ${escapeHtml(task.robotId)}<br>
            ${escapeHtml(statusLabels[task.status] || task.status)}
          </span>
        </div>
      `;
    })
    .join("");

  if (!els.robotStatusList) return;
  els.robotStatusList.innerHTML = state.robots
    .map(robot => {
      const task = state.tasks.find(
        item => item.robotId === robot.id && item.status !== "delivered" && item.status !== "cancelled"
      );
      const selectedClass = robot.id === state.selectedTrackingRobotId ? "selected" : "";
      const statusText = task
        ? `${statusLabels[task.status] || task.status} - Room ${task.roomNumber}`
        : robot.status;
      return `
        <button class="robot-status-row ${selectedClass}" type="button" data-robot-id="${escapeHtml(robot.id)}">
          <strong>${escapeHtml(robot.id)}</strong>
          <span>${escapeHtml(statusText)}</span>
        </button>
      `;
    })
    .join("");
}

function visibleTasks() {
  if (pageName === "history") return state.tasks;
  if (pageName === "pharmacy") {
    return state.tasks.filter(task => ["assigned", "moving_to_pharmacy", "at_pharmacy", "dispatched"].includes(task.status));
  }
  if (pageName === "tracking" || pageName === "robot") {
    return state.tasks.filter(task => task.status !== "delivered" && task.status !== "cancelled");
  }
  return state.tasks.filter(task => task.status !== "cancelled");
}

function emptyTaskMessage() {
  if (pageName === "history") return "No order history yet.";
  if (pageName === "pharmacy") return "No pharmacy orders waiting.";
  return "No tasks assigned yet.";
}

function renderTasks() {
  if (!els.taskList || !els.taskTemplate) return;
  const tasks = visibleTasks();
  els.taskList.innerHTML = "";

  if (!tasks.length) {
    els.taskList.innerHTML = `<div class="empty-state">${emptyTaskMessage()}</div>`;
    return;
  }

  for (const task of tasks) {
    const node = els.taskTemplate.content.cloneNode(true);
    node.querySelector('[data-field="patient"]').textContent = `${task.patientName} - Room ${task.roomNumber}`;

    const status = node.querySelector('[data-field="status"]');
    status.textContent = statusLabels[task.status] || task.status;
    status.classList.add(`status-${task.status}`);

    node.querySelector('[data-field="meta"]').textContent =
      `${task.patientId} | ${task.robotId} | ${task.doctorName}`;
    node.querySelector('[data-field="medicine"]').textContent = `${task.medicine} - ${task.dosage}`;

    const route = node.querySelector('[data-field="route"]');
    route.innerHTML = task.route
      .map(item => `<li>${formatTime(item.at)} - ${escapeHtml(item.note)}</li>`)
      .join("");

    els.taskList.appendChild(node);
  }
}

function taskDetailRows(task) {
  return `
    <div class="detail-row"><span>Patient</span><strong>${escapeHtml(task.patientName)}</strong></div>
    <div class="detail-row"><span>Patient ID</span><strong>${escapeHtml(task.patientId)}</strong></div>
    <div class="detail-row"><span>Destination</span><strong>Room ${escapeHtml(task.roomNumber)}</strong></div>
    <div class="detail-row"><span>Medicine</span><strong>${escapeHtml(task.medicine)}</strong></div>
    <div class="detail-row"><span>Dosage</span><strong>${escapeHtml(task.dosage)}</strong></div>
    <div class="detail-row"><span>Notes</span><strong>${escapeHtml(task.notes || "None")}</strong></div>
    <div class="detail-row"><span>Status</span><strong>${escapeHtml(statusLabels[task.status] || task.status)}</strong></div>
  `;
}

async function loadSession() {
  const data = await api("/api/me");
  state.user = data.user;
  renderNav();
}

async function loadBootstrap() {
  const data = await api("/api/bootstrap");
  applyTaskSnapshot(data);
  setStatus("Online");
}

function applyTaskSnapshot(data) {
  state.robots = data.robots;
  state.roomNfcTags = data.roomNfcTags;
  state.tasks = data.tasks;
  renderRobots();
  renderTags();
  renderMap();
  renderTasks();
}

async function refresh() {
  await loadBootstrap();
}

function connectTaskSocket() {
  clearTimeout(taskReconnectTimer);
  const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
  taskSocket = new WebSocket(`${protocol}//${window.location.host}/ws/robot-chat`);

  taskSocket.addEventListener("open", () => {
    setStatus("Live");
  });

  taskSocket.addEventListener("message", event => {
    const data = JSON.parse(event.data);
    if (data.type === "task_update") {
      applyTaskSnapshot(data);
      setStatus("Live");
    }
  });

  taskSocket.addEventListener("close", () => {
    setStatus("Reconnecting", "error");
    taskReconnectTimer = setTimeout(connectTaskSocket, 1500);
  });

  taskSocket.addEventListener("error", () => {
    setStatus("Socket error", "error");
  });
}

if (els.doctorForm) {
  els.doctorForm.addEventListener("submit", async event => {
    event.preventDefault();
    const payload = Object.fromEntries(new FormData(els.doctorForm).entries());

    try {
      await api("/api/tasks", {
        method: "POST",
        body: JSON.stringify(payload)
      });
      els.doctorForm.reset();
      await refresh();
    } catch (error) {
      alert(error.message);
    }
  });
}

if (els.findRobotTask && els.robotLookup && els.pharmacyTask) {
  els.findRobotTask.addEventListener("click", async () => {
    const robotId = els.robotLookup.value.trim().toUpperCase();
    if (!robotId) return;

    try {
      const data = await api(`/api/robots/${encodeURIComponent(robotId)}/task`);
      if (!data.task) {
        els.pharmacyTask.className = "empty-state";
        els.pharmacyTask.textContent = "No active task found for this robot.";
        return;
      }

      els.pharmacyTask.className = "action-card";
      els.pharmacyTask.innerHTML = `
        ${taskDetailRows(data.task)}
        <label>
          Pharmacist name
          <input id="pharmacistName" value="Pharmacist">
        </label>
        <button id="dispatchTask" type="button" ${data.task.status === "dispatched" ? "disabled" : ""}>
          Mark medicine loaded and dispatch
        </button>
      `;

      document.querySelector("#dispatchTask").addEventListener("click", async () => {
        const pharmacistName = document.querySelector("#pharmacistName").value;
        await api(`/api/tasks/${data.task.id}/dispatch`, {
          method: "POST",
          body: JSON.stringify({ pharmacistName })
        });
        await refresh();
        els.findRobotTask.click();
      });
    } catch (error) {
      alert(error.message);
    }
  });
}

if (els.scanNfc && els.robotConsoleSelect && els.nfcTagSelect && els.robotConsole) {
  els.scanNfc.addEventListener("click", async () => {
    try {
      const robotId = els.robotConsoleSelect.value;
      const tagId = els.nfcTagSelect.value;
      const data = await api(`/api/robots/${encodeURIComponent(robotId)}/nfc`, {
        method: "POST",
        body: JSON.stringify({ tagId })
      });

      const action = data.matchedDestination
        ? `<button id="deliverTask" class="secondary" type="button">Release medicine compartment</button>`
        : "";

      els.robotConsole.className = "action-card";
      els.robotConsole.innerHTML = `
        ${taskDetailRows(data.task)}
        <p class="notice">${data.matchedDestination ? "Destination matched." : "Destination not matched. Continue route."}</p>
        ${action}
      `;

      if (data.matchedDestination) {
        document.querySelector("#deliverTask").addEventListener("click", async () => {
          await api(`/api/tasks/${data.task.id}/deliver`, { method: "POST", body: "{}" });
          els.robotConsole.innerHTML = '<p class="notice">Medicine delivered. Robot can return to base.</p>';
          await refresh();
        });
      }

      await refresh();
    } catch (error) {
      alert(error.message);
    }
  });
}

if (els.resetRobot && els.robotConsoleSelect) {
  els.resetRobot.addEventListener("click", async () => {
    const robotId = els.robotConsoleSelect.value;
    if (!robotId) return;

    try {
      const data = await api(`/api/robots/${encodeURIComponent(robotId)}/reset`, {
        method: "POST",
        body: "{}"
      });
      els.robotConsole.className = "empty-state";
      els.robotConsole.textContent = data.resetTask
        ? `${robotId} reset to available mode.`
        : `${robotId} is already available.`;
      await refresh();
    } catch (error) {
      alert(error.message);
    }
  });
}

if (els.refreshTasks) {
  els.refreshTasks.addEventListener("click", refresh);
}

if (els.trackingRobotSelect) {
  els.trackingRobotSelect.addEventListener("change", () => {
    state.selectedTrackingRobotId = els.trackingRobotSelect.value;
    renderMap();
  });
}

if (els.robotStatusList) {
  els.robotStatusList.addEventListener("click", event => {
    const row = event.target.closest("[data-robot-id]");
    if (!row) return;
    state.selectedTrackingRobotId = row.dataset.robotId;
    if (els.trackingRobotSelect) {
      els.trackingRobotSelect.value = state.selectedTrackingRobotId;
    }
    renderMap();
  });
}

if (els.logoutButton) {
  els.logoutButton.addEventListener("click", async () => {
    await api("/api/logout", { method: "POST", body: "{}" });
    window.location.href = "/";
  });
}

loadSession()
  .then(loadBootstrap)
  .then(connectTaskSocket)
  .catch(error => {
    setStatus("Offline", "error");
    console.error(error);
    window.location.href = "/";
  });
