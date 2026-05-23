/* ── State ─────────────────────────────────────────────── */
let currentSession = null;

/* ── Views ─────────────────────────────────────────────── */
function showView(id) {
  document.querySelectorAll('.view').forEach(v => v.classList.remove('active'));
  document.getElementById(id).classList.add('active');
}

/* ── API helper ────────────────────────────────────────── */
async function apiCall(action, body = {}) {
  const params = new URLSearchParams({ action, ...body });
  const res = await fetch(`/api/auth.py?${params}`, {
    method: 'GET',
    headers: { 'Accept': 'application/json' }
  });
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
  return res.json();
}

async function apiPost(action, body = {}) {
  const res = await fetch('/api/auth.py', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({ action, ...body }).toString()
  });
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
  return res.json();
}

/* ── Login ─────────────────────────────────────────────── */
async function doLogin() {
  const username = document.getElementById('username').value.trim();
  const password = document.getElementById('password').value;
  const errEl    = document.getElementById('login-error');
  const btn      = document.getElementById('login-btn');

  errEl.hidden = true;
  if (!username || !password) {
    showError('Identifiant et mot de passe requis.');
    return;
  }

  btn.classList.add('loading');
  btn.querySelector('.btn-text').textContent = 'Connexion…';

  try {
    const data = await apiPost('login', { username, password });
    if (data.ok) {
      currentSession = data;
      enterDashboard(data.username);
    } else {
      showError(data.error || 'Identifiants incorrects.');
    }
  } catch (e) {
    showError('Erreur serveur — réessayez.');
  } finally {
    btn.classList.remove('loading');
    btn.querySelector('.btn-text').textContent = 'Connexion';
  }
}

function showError(msg) {
  const el = document.getElementById('login-error');
  el.textContent = msg;
  el.hidden = false;
}

/* ── Dashboard ─────────────────────────────────────────── */
function enterDashboard(username) {
  document.getElementById('dash-username').textContent = username;
  document.getElementById('dash-greeting').textContent = username;
  updateTime();
  setInterval(updateTime, 1000);
  showView('view-dashboard');
}

function updateTime() {
  const el = document.getElementById('dash-time');
  if (el) el.textContent = new Date().toLocaleTimeString('fr-FR');
}

/* ── Logout ────────────────────────────────────────────── */
async function doLogout() {
  try { await apiCall('logout'); } catch (_) {}
  currentSession = null;
  document.getElementById('username').value = '';
  document.getElementById('password').value = '';
  document.getElementById('login-error').hidden = true;
  showView('view-login');
}

/* ── Session restore on load ───────────────────────────── */
async function checkSession() {
  try {
    const data = await apiCall('verify');
    if (data.ok) {
      currentSession = data;
      enterDashboard(data.username);
      return;
    }
  } catch (_) {}
  showView('view-login');
}

/* ── Server status ping ────────────────────────────────── */
async function pingStatus() {
  const dot = document.getElementById('server-status');
  try {
    const res = await fetch('/status', { method: 'GET' });
    dot.className = 'status-dot ' + (res.ok ? 'ok' : 'err');
    dot.title = res.ok ? 'Serveur en ligne' : `Erreur ${res.status}`;
  } catch (_) {
    dot.className = 'status-dot err';
    dot.title = 'Serveur inaccessible';
  }
}

/* ── Enter key on form ─────────────────────────────────── */
document.addEventListener('keydown', e => {
  if (e.key === 'Enter' && document.getElementById('view-login').classList.contains('active')) {
    doLogin();
  }
});

/* ── Init ──────────────────────────────────────────────── */
window.addEventListener('DOMContentLoaded', () => {
  pingStatus();
  checkSession();
});
