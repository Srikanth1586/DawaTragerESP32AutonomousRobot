const form = document.querySelector("#loginForm");
const message = document.querySelector("#loginMessage");

form.addEventListener("submit", async event => {
  event.preventDefault();
  message.textContent = "";

  const payload = Object.fromEntries(new FormData(form).entries());

  try {
    const response = await fetch("/api/login", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    const data = await response.json();

    if (!response.ok) {
      throw new Error(data.error || "Login failed");
    }

    window.location.href = data.redirect;
  } catch (error) {
    message.textContent = error.message;
  }
});
