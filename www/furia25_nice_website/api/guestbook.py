#!/usr/bin/env python3
"""
guestbook.py — CGI du livre d'or permanent pour le Liminal Registry
Actions : list | post | react

Stores entries + reactions in the shared SQLite database (registry.db).
No cgi/cgitb (deprecated Python 3.11+, removed 3.13).
"""

import html
import http.cookies
import json
import os
import sqlite3
import sys
import time
import urllib.parse
import hmac
import hashlib

# ── Config ────────────────────────────────────────────────
ALLOWED_ORIGINS = os.environ.get("ALLOWED_ORIGINS", "http://localhost:8080")
SECRET_KEY      = os.environ.get("SECRET_KEY", "dev-secret-change-me").encode()
SESSION_TTL     = 3600
COOKIE_NAME     = "lreg_session"
DB_PATH         = os.path.join(os.path.dirname(__file__), "..", "data", "registry.db")

MAX_TEXT_LEN    = 1000
MAX_NAME_LEN    = 64
MAX_WEBSITE_LEN = 128
MAX_ENTRIES     = 500   # cap to keep the list sane
VALID_REACTIONS = {"seen", "wtf", "heart"}

# ── Database ───────────────────────────────────────────────

def get_db() -> sqlite3.Connection:
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA journal_mode=WAL")
    conn.execute("""
        CREATE TABLE IF NOT EXISTS guestbook (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            name       TEXT    NOT NULL DEFAULT 'anonymous',
            website    TEXT    NOT NULL DEFAULT '',
            text       TEXT    NOT NULL DEFAULT '',
            created_at TEXT    NOT NULL DEFAULT (datetime('now')),
            username   TEXT    DEFAULT NULL
        )
    """)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS gb_reactions (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            entry_id   INTEGER NOT NULL,
            reaction   TEXT    NOT NULL,
            count      INTEGER NOT NULL DEFAULT 0,
            UNIQUE(entry_id, reaction)
        )
    """)
    # users table (created by auth.py, but ensure it exists here too)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS users (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            username   TEXT    NOT NULL UNIQUE COLLATE NOCASE,
            pw_hash    TEXT    NOT NULL,
            email      TEXT    DEFAULT '',
            created_at TEXT    DEFAULT (datetime('now')),
            last_login TEXT
        )
    """)
    conn.commit()
    return conn

# ── Auth helpers (token verify, no full import of auth.py) ─

def verify_token(token: str) -> str | None:
    """Return username if token is valid, else None."""
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
        return username
    except Exception:
        return None

def get_authed_user() -> str | None:
    raw = os.environ.get("HTTP_COOKIE", "")
    c   = http.cookies.SimpleCookie()
    c.load(raw)
    token = c[COOKIE_NAME].value if COOKIE_NAME in c else None
    if not token:
        return None
    return verify_token(token)

# ── CGI form reader ────────────────────────────────────────

def read_form() -> dict[str, str]:
    qs_raw = os.environ.get("QUERY_STRING", "")
    params = dict(urllib.parse.parse_qsl(qs_raw, keep_blank_values=True))
    if os.environ.get("REQUEST_METHOD", "GET").upper() == "POST":
        length = int(os.environ.get("CONTENT_LENGTH", "0") or 0)
        if length > 0:
            body = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
            params.update(dict(urllib.parse.parse_qsl(body, keep_blank_values=True)))
    return params

# ── HTTP responses ─────────────────────────────────────────

def send_json(data: dict, status: str = "200 OK") -> None:
    body = json.dumps(data, ensure_ascii=False)
    out  = sys.stdout
    out.write(f"Status: {status}\r\n")
    out.write("Content-Type: application/json; charset=utf-8\r\n")
    out.write(f"Access-Control-Allow-Origin: {ALLOWED_ORIGINS}\r\n")
    out.write("Access-Control-Allow-Credentials: true\r\n")
    out.write("\r\n")
    out.write(body)
    out.flush()

def send_error(msg: str, status: str = "400 Bad Request") -> None:
    send_json({"ok": False, "error": msg}, status=status)

# ── Actions ────────────────────────────────────────────────

def action_list(form: dict) -> None:
    try:
        with get_db() as conn:
            rows = conn.execute("""
                SELECT id, name, website, text, created_at, username
                FROM guestbook
                ORDER BY id ASC
                LIMIT ?
            """, (MAX_ENTRIES,)).fetchall()

            entries = []
            for row in rows:
                eid = row["id"]
                # load reactions
                reacts = {"seen": 0, "wtf": 0, "heart": 0}
                for rrow in conn.execute(
                    "SELECT reaction, count FROM gb_reactions WHERE entry_id=?", (eid,)
                ):
                    if rrow["reaction"] in reacts:
                        reacts[rrow["reaction"]] = rrow["count"]
                # is this user a verified (registered) account?
                verified = False
                avatar_url = ""
                if row["username"]:
                    user_row = conn.execute(
                        "SELECT avatar_url FROM users WHERE username=? COLLATE NOCASE", (row["username"],)
                    ).fetchone()
                    if user_row:
                        verified = True
                        avatar_url = user_row["avatar_url"] or ""
                entries.append({
                    "id":         eid,
                    "name":       row["name"],
                    "website":    row["website"],
                    "text":       row["text"],
                    "created_at": row["created_at"],
                    "reactions":  reacts,
                    "verified":   verified,
                    "avatar_url": avatar_url,
                })

            total_accounts = conn.execute("SELECT COUNT(*) FROM users").fetchone()[0]

    except Exception as exc:
        return send_error(f"DB error: {exc}", "500 Internal Server Error")

    send_json({
        "ok":             True,
        "entries":        entries,
        "total_entries":  len(entries),
        "total_accounts": total_accounts,
    })

import datetime

def action_post(form: dict) -> None:
    name    = (form.get("name",    "") or "anonymous").strip()[:MAX_NAME_LEN]
    website = (form.get("website", "") or "").strip()[:MAX_WEBSITE_LEN]
    text    = (form.get("text",    "") or "").strip()[:MAX_TEXT_LEN]

    if not text and not name:
        return send_error("Empty entry not allowed.")

    # Strip dangerous URL schemes from website
    if website and website.lower().startswith(("javascript:", "data:", "vbscript:")):
        website = ""

    # Determine if poster is an authenticated registered user
    username = get_authed_user()  # None if not logged in
    created_at = datetime.datetime.utcnow().strftime("%I:%M %p")

    try:
        with get_db() as conn:
            cur = conn.execute("""
                INSERT INTO guestbook (name, website, text, created_at, username)
                VALUES (?, ?, ?, ?, ?)
            """, (name, website, text, created_at, username))
            new_id = cur.lastrowid
            # seed reaction rows at zero
            for reaction in VALID_REACTIONS:
                conn.execute(
                    "INSERT OR IGNORE INTO gb_reactions (entry_id, reaction, count) VALUES (?,?,0)",
                    (new_id, reaction)
                )
    except Exception as exc:
        return send_error(f"DB error: {exc}", "500 Internal Server Error")

    send_json({"ok": True, "entry_id": new_id})


def action_react(form: dict) -> None:
    try:
        entry_id = int(form.get("entry_id", 0))
    except ValueError:
        return send_error("Invalid entry_id.")
    reaction = form.get("reaction", "").strip().lower()
    if reaction not in VALID_REACTIONS:
        return send_error(f"Unknown reaction '{reaction}'.")

    try:
        with get_db() as conn:
            # ensure row exists
            conn.execute(
                "INSERT OR IGNORE INTO gb_reactions (entry_id, reaction, count) VALUES (?,?,0)",
                (entry_id, reaction)
            )
            conn.execute(
                "UPDATE gb_reactions SET count=count+1 WHERE entry_id=? AND reaction=?",
                (entry_id, reaction)
            )
    except Exception as exc:
        return send_error(f"DB error: {exc}", "500 Internal Server Error")

    send_json({"ok": True})

# ── Dispatch ───────────────────────────────────────────────

def main() -> None:
    form   = read_form()
    action = form.get("action", "list").strip().lower()

    if action == "list":
        action_list(form)
    elif action == "post":
        action_post(form)
    elif action == "react":
        action_react(form)
    else:
        send_error(f"Unknown action: '{action}'")

if __name__ == "__main__":
    main()
