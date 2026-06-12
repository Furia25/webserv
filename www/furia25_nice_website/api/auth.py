#!/usr/bin/env python3
"""
auth.py — CGI d'authentification pour le Liminal Registry
Actions : login | logout | verify | register

Accounts stored permanently in SQLite (DB_PATH).
No cgi/cgitb (deprecated Python 3.11+, removed 3.13).
"""

import hashlib
import hmac
import http.cookies
import json
import os
import sqlite3
import sys
import time
import urllib.parse

# ── Config ────────────────────────────────────────────────
SECRET_KEY      = os.environ.get("SECRET_KEY", "dev-secret-change-me").encode()
ALLOWED_ORIGINS = os.environ.get("ALLOWED_ORIGINS", "http://localhost:8080")
SESSION_TTL     = 3600   # seconds
COOKIE_NAME     = "lreg_session"
DB_PATH         = os.path.join(os.path.dirname(__file__), "..", "data", "registry.db")

# ── Built-in fallback accounts (kept for demo / bootstrap) ────
_BUILTIN: dict[str, str] = {
    "admin": hashlib.sha256(b"admin1234").hexdigest(),
    "guest": hashlib.sha256(b"guest").hexdigest(),
}

# ── Database ───────────────────────────────────────────────

def get_db() -> sqlite3.Connection:
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA journal_mode=WAL")
    conn.execute("""
        CREATE TABLE IF NOT EXISTS users (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            username   TEXT    NOT NULL UNIQUE COLLATE NOCASE,
            pw_hash    TEXT    NOT NULL,
            email      TEXT    DEFAULT '',
            avatar_url TEXT    DEFAULT '',
            created_at TEXT    DEFAULT (datetime('now')),
            last_login TEXT
        )
    """)
    # Migrate: add avatar_url if it doesn't exist yet (safe on existing DBs)
    try:
        conn.execute("ALTER TABLE users ADD COLUMN avatar_url TEXT DEFAULT ''")
        conn.commit()
    except Exception:
        pass  # column already exists
    # Seed built-in accounts if they don't exist yet
    for uname, pwhash in _BUILTIN.items():
        conn.execute(
            "INSERT OR IGNORE INTO users (username, pw_hash) VALUES (?,?)",
            (uname, pwhash)
        )
    conn.commit()
    return conn

# ── CGI form reader ────────────────────────────────────────

def read_form() -> dict[str, str]:
    qs_raw = os.environ.get("QUERY_STRING", "")
    params = dict(urllib.parse.parse_qsl(qs_raw, keep_blank_values=True))
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    if method == "POST":
        length = int(os.environ.get("CONTENT_LENGTH", "0") or 0)
        if length > 0:
            body = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
            params.update(dict(urllib.parse.parse_qsl(body, keep_blank_values=True)))
    return params

# ── HMAC tokens ────────────────────────────────────────────

def make_token(username: str) -> str:
    ts      = str(int(time.time()))
    payload = f"{username}|{ts}"
    sig     = hmac.new(SECRET_KEY, payload.encode(), hashlib.sha256).hexdigest()
    return f"{payload}|{sig}"

def verify_token(token: str) -> tuple[str, int] | None:
    try:
        parts = token.split("|")
        if len(parts) != 3:
            return None
        username, ts, sig = parts
        payload  = f"{username}|{ts}"
        expected = hmac.new(SECRET_KEY, payload.encode(), hashlib.sha256).hexdigest()
        if not hmac.compare_digest(expected, sig):
            return None
        if int(time.time()) - int(ts) > SESSION_TTL:
            return None
        return username, int(ts)
    except Exception:
        return None

# ── Cookies ────────────────────────────────────────────────

def get_cookie_token() -> str | None:
    raw = os.environ.get("HTTP_COOKIE", "")
    c   = http.cookies.SimpleCookie()
    c.load(raw)
    return c[COOKIE_NAME].value if COOKIE_NAME in c else None

def _build_cookie(name: str, value: str, max_age: int) -> str:
    c = http.cookies.SimpleCookie()
    c[name] = value
    c[name]["httponly"] = True
    c[name]["samesite"] = "Lax"
    c[name]["path"]     = "/"
    c[name]["max-age"]  = max_age
    if os.environ.get("HTTPS", "off").lower() in ("on", "1", "true"):
        c[name]["secure"] = True
    return "Set-Cookie: " + c.output(header="").strip()

def set_cookie_header(token: str) -> str:
    return _build_cookie(COOKIE_NAME, token, SESSION_TTL)

def clear_cookie_header() -> str:
    return _build_cookie(COOKIE_NAME, "", 0)

# ── HTTP responses ─────────────────────────────────────────

def send_json(data: dict, status: str = "200 OK",
              extra_headers: list[str] | None = None) -> None:
    body = json.dumps(data, ensure_ascii=False)
    out  = sys.stdout
    out.write(f"Status: {status}\r\n")
    out.write("Content-Type: application/json; charset=utf-8\r\n")
    out.write(f"Access-Control-Allow-Origin: {ALLOWED_ORIGINS}\r\n")
    out.write("Access-Control-Allow-Credentials: true\r\n")
    if extra_headers:
        for h in extra_headers:
            out.write(h + "\r\n")
    out.write("\r\n")
    out.write(body)
    out.flush()

def send_error(msg: str, status: str = "400 Bad Request") -> None:
    send_json({"ok": False, "error": msg}, status=status)

# ── Actions ────────────────────────────────────────────────

def action_login(form: dict) -> None:
    username = form.get("username", "").strip()
    password = form.get("password", "")
    if not username or not password:
        return send_error("Identifiant et mot de passe requis.")
    candidate = hashlib.sha256(password.encode()).hexdigest()
    try:
        with get_db() as conn:
            row = conn.execute(
                "SELECT pw_hash FROM users WHERE username=? COLLATE NOCASE",
                (username,)
            ).fetchone()
            if not row or not hmac.compare_digest(row["pw_hash"], candidate):
                return send_error("Identifiants incorrects.")
            conn.execute(
                "UPDATE users SET last_login=datetime('now') WHERE username=? COLLATE NOCASE",
                (username,)
            )
    except Exception as exc:
        return send_error(f"DB error: {exc}", "500 Internal Server Error")
    token = make_token(username)
    send_json(
        {"ok": True, "username": username, "expires_in": SESSION_TTL},
        extra_headers=[set_cookie_header(token)],
    )


def action_register(form: dict) -> None:
    username = form.get("username", "").strip()
    password = form.get("password", "")
    email    = form.get("email",    "").strip()[:254]

    if not username or not password:
        return send_error("Username and password are required.")
    if len(username) < 2 or len(username) > 32:
        return send_error("Username must be 2–32 characters.")
    if len(password) < 4:
        return send_error("Password must be at least 4 characters.")
    # Reject characters that would break the HMAC token format
    if "|" in username:
        return send_error("Username may not contain '|'.")

    pw_hash = hashlib.sha256(password.encode()).hexdigest()
    try:
        with get_db() as conn:
            existing = conn.execute(
                "SELECT id FROM users WHERE username=? COLLATE NOCASE",
                (username,)
            ).fetchone()
            if existing:
                return send_error("Username already taken.")
            conn.execute(
                "INSERT INTO users (username, pw_hash, email) VALUES (?,?,?)",
                (username, pw_hash, email)
            )
    except sqlite3.IntegrityError:
        return send_error("Username already taken.")
    except Exception as exc:
        return send_error(f"DB error: {exc}", "500 Internal Server Error")

    token = make_token(username)
    send_json(
        {"ok": True, "username": username, "expires_in": SESSION_TTL},
        extra_headers=[set_cookie_header(token)],
    )


def action_verify(form: dict) -> None:
    token = get_cookie_token()
    if not token:
        return send_error("Non authentifié.", "401 Unauthorized")
    result = verify_token(token)
    if not result:
        return send_error("Session expirée ou invalide.", "401 Unauthorized")
    username, ts = result
    remaining = SESSION_TTL - (int(time.time()) - ts)
    send_json({"ok": True, "username": username, "expires_in": remaining})


def action_logout(form: dict) -> None:
    send_json({"ok": True}, extra_headers=[clear_cookie_header()])


def action_set_avatar(form: dict) -> None:
    """Save the RoboHash avatar URL for the authenticated user."""
    token = get_cookie_token()
    if not token:
        return send_error("Non authentifié.", "401 Unauthorized")
    result = verify_token(token)
    if not result:
        return send_error("Session expirée ou invalide.", "401 Unauthorized")
    username, _ = result

    avatar_url = form.get("avatar_url", "").strip()[:512]
    # Only allow robohash URLs
    if avatar_url and not avatar_url.startswith("https://robohash.org/"):
        return send_error("Invalid avatar URL.")

    try:
        with get_db() as conn:
            conn.execute(
                "UPDATE users SET avatar_url=? WHERE username=? COLLATE NOCASE",
                (avatar_url, username)
            )
    except Exception as exc:
        return send_error(f"DB error: {exc}", "500 Internal Server Error")

    send_json({"ok": True, "avatar_url": avatar_url})


def action_get_profile(form: dict) -> None:
    """Return profile info (including avatar_url) for the authenticated user."""
    token = get_cookie_token()
    if not token:
        return send_error("Non authentifié.", "401 Unauthorized")
    result = verify_token(token)
    if not result:
        return send_error("Session expirée ou invalide.", "401 Unauthorized")
    username, ts = result

    try:
        with get_db() as conn:
            row = conn.execute(
                "SELECT username, email, avatar_url, created_at, last_login FROM users WHERE username=? COLLATE NOCASE",
                (username,)
            ).fetchone()
            if not row:
                return send_error("User not found.", "404 Not Found")
    except Exception as exc:
        return send_error(f"DB error: {exc}", "500 Internal Server Error")

    send_json({
        "ok":         True,
        "username":   row["username"],
        "email":      row["email"] or "",
        "avatar_url": row["avatar_url"] or "",
        "created_at": row["created_at"] or "",
        "last_login": row["last_login"] or "",
        "expires_in": SESSION_TTL - (int(time.time()) - ts),
    })

# ── Dispatch ───────────────────────────────────────────────

def main() -> None:
    form   = read_form()
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    action = form.get("action", "").strip().lower()

    if method == "POST" and action == "login":
        action_login(form)
    elif method == "POST" and action == "register":
        action_register(form)
    elif method == "POST" and action == "set_avatar":
        action_set_avatar(form)
    elif action == "get_profile":
        action_get_profile(form)
    elif action == "verify":
        action_verify(form)
    elif action == "logout":
        action_logout(form)
    else:
        send_error(f"Action inconnue : '{action}'")

if __name__ == "__main__":
    main()
