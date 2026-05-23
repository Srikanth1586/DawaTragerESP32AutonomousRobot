const els = {
  serverStatus: document.querySelector("#serverStatus"),
  robotAliveStatus: document.querySelector("#robotAliveStatus"),
  logoutButton: document.querySelector("#logoutButton"),
  robotChatList: document.querySelector("#robotChatList"),
  chatWindow: document.querySelector("#chatWindow"),
  chatForm: document.querySelector("#chatForm"),
  selectedRobotLabel: document.querySelector("#selectedRobotLabel"),
  messageInput: document.querySelector("#messageInput"),
  tabButtons: document.querySelectorAll("[data-tab]"),
  tabPanels: document.querySelectorAll(".admin-tab-panel")
};

const state = {
  robots: [],
  messages: [],
  selectedRobotId: "RBT-001",
  pulseRobotId: ""
};

let socket = null;
let reconnectTimer = null;
let previousVisibleMessageCount = 0;

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

function setServerStatus(message, tone = "online") {
  els.serverStatus.textContent = message;
  els.serverStatus.style.background = tone === "error" ? "#fee4e2" : "#e9f8f2";
  els.serverStatus.style.color = tone === "error" ? "#b42318" : "#12805c";
}

function setRobotStatus() {
  const selectedRobot = state.robots.find(robot => robot.id === state.selectedRobotId);
  if (selectedRobot?.alive) {
    els.robotAliveStatus.textContent = `${selectedRobot.id} alive`;
    els.robotAliveStatus.style.background = "#e9f8f2";
    els.robotAliveStatus.style.color = "#12805c";
  } else {
    els.robotAliveStatus.textContent = `${state.selectedRobotId} waiting`;
    els.robotAliveStatus.style.background = "#fff6e6";
    els.robotAliveStatus.style.color = "#a15c00";
  }
}

function renderRobotList() {
  if (!state.robots.length) {
    els.robotChatList.innerHTML = '<div class="empty-state">No robots configured.</div>';
    return;
  }

  els.robotChatList.innerHTML = state.robots
    .map(robot => {
      const selectedClass = robot.id === state.selectedRobotId ? "selected" : "";
      const aliveClass = robot.alive ? "alive" : "";
      const pulseClass = robot.id === state.pulseRobotId ? "pulse" : "";
      const messageCount = state.messages.filter(message => message.robotId === robot.id).length;
      return `
        <button class="robot-chat-item ${selectedClass}" type="button" data-robot-id="${escapeHtml(robot.id)}">
          <span class="robot-chat-dot ${aliveClass} ${pulseClass}"></span>
          <span>
            <strong>${escapeHtml(robot.id)}</strong>
            <small>${escapeHtml(robot.name)} | ${messageCount} messages</small>
          </span>
        </button>
      `;
    })
    .join("");
}

function renderMessages() {
  const visibleMessages = state.messages.filter(item => item.robotId === state.selectedRobotId);

  if (!visibleMessages.length) {
    els.chatWindow.innerHTML = `<div class="empty-state">No messages for ${escapeHtml(state.selectedRobotId)} yet.</div>`;
    previousVisibleMessageCount = 0;
    return;
  }

  els.chatWindow.innerHTML = visibleMessages
    .map(item => {
      const side = item.sender === "admin" ? "outgoing" : "incoming";
      const name = item.sender === "admin"
        ? "Admin"
        : item.sender === "system" || item.sender === "server"
          ? "Server"
          : item.robotId;
      return `
        <article class="chat-message ${side}">
          <div class="chat-bubble">
            <div class="chat-meta">
              <strong>${escapeHtml(name)}</strong>
              <span>${formatTime(item.at)}</span>
            </div>
            <p>${escapeHtml(item.message)}</p>
          </div>
        </article>
      `;
    })
    .join("");

  if (visibleMessages.length !== previousVisibleMessageCount) {
    els.chatWindow.scrollTop = els.chatWindow.scrollHeight;
    previousVisibleMessageCount = visibleMessages.length;
  }
}

function renderChat() {
  if (!state.robots.find(robot => robot.id === state.selectedRobotId) && state.robots.length) {
    state.selectedRobotId = state.robots[0].id;
  }
  els.selectedRobotLabel.textContent = state.selectedRobotId;
  renderRobotList();
  renderMessages();
  setRobotStatus();
}

function applyChatSnapshot(data) {
  if (data.messages) {
    state.messages = data.messages;
  }
  if (data.robots) {
    state.robots = data.robots;
  }
  if (data.type === "heartbeat" && data.robotId) {
    state.pulseRobotId = data.robotId;
    setTimeout(() => {
      if (state.pulseRobotId === data.robotId) {
        state.pulseRobotId = "";
        renderChat();
      }
    }, 900);
  }
  renderChat();
}

async function refreshChatFallback() {
  const data = await api("/api/admin/robot-chat");
  applyChatSnapshot(data);
}

function connectWebSocket() {
  clearTimeout(reconnectTimer);

  const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
  socket = new WebSocket(`${protocol}//${window.location.host}/ws/robot-chat`);
  setServerStatus("Connecting", "warning");

  socket.addEventListener("open", () => {
    setServerStatus("WebSocket online");
  });

  socket.addEventListener("message", event => {
    const data = JSON.parse(event.data);
    if (["snapshot", "message", "robot_order", "heartbeat"].includes(data.type)) {
      applyChatSnapshot(data);
    }
  });

  socket.addEventListener("close", () => {
    setServerStatus("Reconnecting", "error");
    reconnectTimer = setTimeout(connectWebSocket, 1500);
  });

  socket.addEventListener("error", () => {
    setServerStatus("Socket error", "error");
  });
}

function sendSocketMessage(payload) {
  if (!socket || socket.readyState !== WebSocket.OPEN) {
    throw new Error("WebSocket is not connected");
  }
  socket.send(JSON.stringify(payload));
}

els.chatForm.addEventListener("submit", async event => {
  event.preventDefault();
  const message = els.messageInput.value.trim();
  if (!message) return;

  try {
    sendSocketMessage({
      type: "admin_message",
      robotId: state.selectedRobotId,
      message
    });
  } catch (error) {
    await api("/api/admin/robot-chat/send", {
      method: "POST",
      body: JSON.stringify({
        robotId: state.selectedRobotId,
        message
      })
    });
    await refreshChatFallback();
  }

  els.messageInput.value = "";
});

els.robotChatList.addEventListener("click", event => {
  const item = event.target.closest("[data-robot-id]");
  if (!item) return;
  state.selectedRobotId = item.dataset.robotId;
  previousVisibleMessageCount = 0;
  renderChat();
});

els.tabButtons.forEach(button => {
  button.addEventListener("click", () => {
    const target = button.dataset.tab;
    els.tabButtons.forEach(item => item.classList.toggle("active", item === button));
    els.tabPanels.forEach(panel => {
      panel.classList.toggle("active", panel.id === `tab-${target}`);
    });
  });
});

els.logoutButton.addEventListener("click", async () => {
  await api("/api/logout", { method: "POST", body: "{}" });
  window.location.href = "/";
});

api("/api/me")
  .then(data => {
    if (data.user.role !== "admin") {
      window.location.href = data.user.home || "/";
      return;
    }
    connectWebSocket();
  })
  .catch(() => {
    window.location.href = "/";
  });
