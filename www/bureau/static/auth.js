// ============================================
// I.B.O.T. Authentication System — Cookie Edition
// The JWT lives in an HttpOnly cookie managed by
// the browser. JS never sees or stores the token.
// ============================================

const API_BASE = "/api";
const AUTH_ENDPOINTS = {
  login:    "/auth/login",
  logout:   "/auth/logout",
  validate: "/auth/validate",
  refresh:  "/auth/refresh",
  profile:  "/users/profile"
};

// Only the user profile is kept in sessionStorage (no token — never accessible to JS).
const SESSION_KEY = "ibot_user";

// ── API helper ────────────────────────────────────────────────────────────────

/**
 * Generic API call.
 * credentials: "same-origin" tells the browser to send the HttpOnly cookie
 * automatically on every request — no manual token handling needed.
 */
function apiCall(endpoint, method = "GET", body = null) {
  const options = {
    method,
    credentials: "same-origin",   // ← sends the ibot_session cookie
    headers: { "Content-Type": "application/json" }
  };

  if (body) {
    options.body = JSON.stringify(body);
  }

  return fetch(`${API_BASE}${endpoint}`, options)
    .then(response => {
      if (response.status === 401) {
        // Cookie expired or missing — bounce to login
        clearSession();
        window.location.href = "index.html";
        throw new Error("Session expired");
      }
      if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
      }
      return response.json();
    });
}

// ── Session (user profile only, no token) ─────────────────────────────────────

function saveSession(user) {
  sessionStorage.setItem(SESSION_KEY, JSON.stringify({
    username:  user.username,
    name:      user.name,
    clearance: user.clearance,
    loginTime: new Date().toISOString()
  }));
}

function getUser() {
  try {
    return JSON.parse(sessionStorage.getItem(SESSION_KEY));
  } catch {
    return null;
  }
}

function clearSession() {
  sessionStorage.removeItem(SESSION_KEY);
  // Note: we cannot clear the HttpOnly cookie from JS.
  // The server clears it via Set-Cookie: Max-Age=0 on /auth/logout.
}

// ── Login ─────────────────────────────────────────────────────────────────────

function doLogin() {
  const username  = document.getElementById("username").value.trim().toLowerCase();
  const password  = document.getElementById("password").value;
  const loginForm = document.getElementById("login-form");
  const loadingScreen = document.getElementById("loading-screen");

  if (!username || !password) {
    showError("All fields required, Agent.");
    return;
  }

  if (loadingScreen) {
    loginForm.style.display = "none";
    loadingScreen.style.display = "block";
  }

  // POST credentials — server responds with Set-Cookie (HttpOnly) + user JSON
  apiCall(AUTH_ENDPOINTS.login, "POST", { username, password })
    .then(data => {
      if (!data.success) throw new Error(data.message || "Authentication failed");
      saveSession(data.user);
      setTimeout(() => { window.location.href = "dashboard.html"; }, 800);
    })
    .catch(error => {
      if (loadingScreen) {
        loadingScreen.style.display = "none";
        loginForm.style.display = "block";
      }
      showError(error.message || "Authentication failed. Invalid credentials.");
      shakeForm();
    });
}

// ── Logout ────────────────────────────────────────────────────────────────────

function logout() {
  // Ask the server to clear the cookie (Max-Age=0), then wipe local state
  apiCall(AUTH_ENDPOINTS.logout, "POST")
    .catch(() => {})
    .finally(() => {
      clearSession();
      window.location.href = "index.html";
    });
}

// ── Protected route guard ─────────────────────────────────────────────────────

/**
 * Call at the top of every protected page.
 * Verifies the cookie is still valid server-side, then populates the welcome msg.
 * Returns false and redirects to login if not authenticated.
 */
function requireAuth() {
  const user = getUser();

  if (!user) {
    // No local session at all — check with server before giving up
    // (handles page reload after session was set)
    apiCall(AUTH_ENDPOINTS.validate, "POST")
      .then(data => {
        if (!data.valid) {
          window.location.href = "index.html";
        } else {
          // Cookie is valid but sessionStorage was cleared (e.g. new tab).
          // Fetch the profile to rebuild the local session.
          return apiCall(AUTH_ENDPOINTS.profile);
        }
      })
      .then(profile => {
        if (profile) {
          saveSession(profile);
          setWelcomeMsg(profile);
        }
      })
      .catch(() => {
        window.location.href = "index.html";
      });

    // Return true optimistically — the fetch above will redirect if invalid
    return true;
  }

  setWelcomeMsg(user);
  return true;
}

function setWelcomeMsg(user) {
  const el = document.getElementById("welcome-msg");
  if (el) {
    el.textContent = `Welcome back, ${user.name}. Clearance level: ${user.clearance}.`;
  }
}

// ── Data fetchers (unchanged interface, cookie sent automatically) ─────────────

function fetchReports()   { return apiCall("/reports"); }
function fetchBestiary()  { return apiCall("/bestiary"); }
function fetchArtifacts() { return apiCall("/artifacts"); }

// ── UI helpers ────────────────────────────────────────────────────────────────

function showError(msg) {
  const el = document.getElementById("error-msg");
  if (el) {
    el.textContent  = msg;
    el.style.opacity = "1";
  }
}

function shakeForm() {
  const container = document.querySelector(".terminal-container");
  if (!container) return;
  container.classList.add("shake");
  setTimeout(() => container.classList.remove("shake"), 500);
}

// Allow Enter key on login page
document.addEventListener("keydown", (e) => {
  if (e.key === "Enter" && document.getElementById("username")) {
    doLogin();
  }
});

// Auto-refresh the cookie 50 minutes before it would expire.
// The server issues a fresh Set-Cookie on /auth/refresh.
setInterval(() => {
  if (getUser()) {
    apiCall(AUTH_ENDPOINTS.refresh, "POST").catch(() => {});
  }
}, 50 * 60 * 1000);
