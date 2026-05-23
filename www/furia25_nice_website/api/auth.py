#!/usr/bin/env python3
"""
auth.py — CGI d'authentification pour le Liminal Registry
Actions : login | logout | verify

Sessions : token HMAC-SHA256 (signé avec SECRET_KEY) stocké en cookie.
Stockage : fichier JSON côté serveur (remplacer par DB pour la prod).

Utilisation :
  GET  /api/auth.py?action=verify
  GET  /api/auth.py?action=logout
  POST /api/auth.py        body: action=login&username=...&password=...
"""

import cgi
import cgitb
import hashlib
import hmac
import json
import os
import sys
import time
import urllib.parse
import http.cookies

cgitb.enable()

# ── Config ────────────────────────────────────────────────
SECRET_KEY      = os.environ.get("SECRET_KEY", "dev-secret-change-me").encode()
ALLOWED_ORIGINS = os.environ.get("ALLOWED_ORIGINS", "http://localhost:8080")
SESSION_TTL     = 3600  # secondes (1 h)
COOKIE_NAME     = "lreg_session"

# ── Base d'utilisateurs (à remplacer par une vraie DB) ────
# Mots de passe hashés SHA-256 : echo -n "password" | sha256sum
USERS = {
    "admin": hashlib.sha256(b"admin1234").hexdigest(),
    "guest": hashlib.sha256(b"guest").hexdigest(),
}

# ── Helpers ───────────────────────────────────────────────

def hash_password(pw: str) -> str:
    return hashlib.sha256(pw.encode()).hexdigest()

def make_token(username: str) -> str:
    """Génère un token signé HMAC : username|timestamp|signature"""
    ts      = str(int(time.time()))
    payload = f"{username}|{ts}"
    sig     = hmac.new(SECRET_KEY, payload.encode(), hashlib.sha256).hexdigest()
    return f"{payload}|{sig}"

def verify_token(token: str):
    """
    Vérifie la signature et l'expiration.
    Retourne (username, ts) si valide, sinon None.
    """
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

def get_cookie_token() -> str | None:
    raw = os.environ.get("HTTP_COOKIE", "")
    c   = http.cookies.SimpleCookie()
    c.load(raw)
    if COOKIE_NAME in c:
        return c[COOKIE_NAME].value
    return None

def set_cookie_header(token: str) -> str:
    c = http.cookies.SimpleCookie()
    c[COOKIE_NAME] = token
    c[COOKIE_NAME]["httponly"] = True
    c[COOKIE_NAME]["samesite"] = "Lax"
    c[COOKIE_NAME]["path"]     = "/"
    c[COOKIE_NAME]["max-age"]  = SESSION_TTL
    # Secure uniquement en HTTPS
    if os.environ.get("HTTPS", "off").lower() in ("on", "1", "true"):
        c[COOKIE_NAME]["secure"] = True
    return c.output(header="Set-Cookie")

def clear_cookie_header() -> str:
    c = http.cookies.SimpleCookie()
    c[COOKIE_NAME] = ""
    c[COOKIE_NAME]["httponly"] = True
    c[COOKIE_NAME]["path"]     = "/"
    c[COOKIE_NAME]["max-age"]  = 0
    return c.output(header="Set-Cookie")

def send_json(data: dict, status: str = "200 OK", extra_headers: list[str] = None):
    body = json.dumps(data, ensure_ascii=False)
    print(f"Status: {status}")
    print("Content-Type: application/json; charset=utf-8")
    print(f"Access-Control-Allow-Origin: {ALLOWED_ORIGINS}")
    print("Access-Control-Allow-Credentials: true")
    if extra_headers:
        for h in extra_headers:
            print(h)
    print()
    print(body)

def error(msg: str, status: str = "400 Bad Request"):
    send_json({"ok": False, "error": msg}, status=status)

# ── Actions ───────────────────────────────────────────────

def action_login(form):
    username = form.getvalue("username", "").strip()
    password = form.getvalue("password", "")

    if not username or not password:
        return error("Identifiant et mot de passe requis.")

    stored_hash = USERS.get(username)
    if not stored_hash:
        return error("Identifiants incorrects.")

    if not hmac.compare_digest(stored_hash, hash_password(password)):
        return error("Identifiants incorrects.")

    token       = make_token(username)
    cookie_hdr  = set_cookie_header(token)
    send_json(
        {"ok": True, "username": username, "expires_in": SESSION_TTL},
        extra_headers=[cookie_hdr]
    )

def action_verify(_form):
    token = get_cookie_token()
    if not token:
        return error("Non authentifié.", "401 Unauthorized")
    result = verify_token(token)
    if not result:
        return error("Session expirée ou invalide.", "401 Unauthorized")
    username, ts = result
    remaining = SESSION_TTL - (int(time.time()) - ts)
    send_json({"ok": True, "username": username, "expires_in": remaining})

def action_logout(_form):
    send_json({"ok": True}, extra_headers=[clear_cookie_header()])

# ── Dispatch ──────────────────────────────────────────────

def main():
    method = os.environ.get("REQUEST_METHOD", "GET").upper()

    # Lecture du formulaire (GET params ou POST body)
    form = cgi.FieldStorage()

    action = form.getvalue("action", "").strip().lower()

    if method == "POST" and action == "login":
        action_login(form)
    elif action == "verify":
        action_verify(form)
    elif action == "logout":
        action_logout(form)
    else:
        error(f"Action inconnue : '{action}'")

if __name__ == "__main__":
    main()
