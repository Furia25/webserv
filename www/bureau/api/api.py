#!/usr/bin/env python3
"""
I.B.O.T. — Interdimensional Bureau of Odd Things
CGI API Backend — Cookie Auth Edition (Python 3, no third-party deps)

Authentication: HttpOnly, Secure, SameSite=Strict session cookie.
The JWT never touches JavaScript — the browser sends it automatically
on every request to the same origin.

Routes:
  POST /api/auth/login
  POST /api/auth/logout
  POST /api/auth/validate
  POST /api/auth/refresh
  GET  /api/users/profile
  GET  /api/reports
  GET  /api/bestiary
  GET  /api/artifacts
"""

import json
import os
import sys
import hmac
import hashlib
import base64
import time
from http.cookies import SimpleCookie
from pathlib import Path

# ── Config ────────────────────────────────────────────────────────────────────
SECRET_KEY     = b"ibot-secret-key-dimensional-rift-42"   # change in production!
TOKEN_LIFETIME = 3600 * 8    # 8 hours
COOKIE_NAME    = "ibot_session"
DATA_DIR       = Path(__file__).parent.parent / "data"

# Set to True once you have HTTPS on your webserver (enables Secure flag)
HTTPS = os.environ.get("HTTPS", "off").lower() in ("on", "1", "true")

# ── Tiny HS256 JWT (zero deps) ────────────────────────────────────────────────
def _b64url_enc(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode()

def _b64url_dec(s: str) -> bytes:
    pad = (4 - len(s) % 4) % 4   # 0, 1, 2, or 3 padding chars
    return base64.urlsafe_b64decode(s + "=" * pad)

def create_token(payload: dict) -> str:
    header = _b64url_enc(json.dumps({"alg": "HS256", "typ": "JWT"}).encode())
    payload = {**payload, "iat": int(time.time()), "exp": int(time.time()) + TOKEN_LIFETIME}
    body    = _b64url_enc(json.dumps(payload).encode())
    sig     = hmac.new(SECRET_KEY, f"{header}.{body}".encode(), hashlib.sha256).digest()
    return f"{header}.{body}.{_b64url_enc(sig)}"

def verify_token(token: str) -> dict | None:
    try:
        h, b, s = token.split(".")
        expected = hmac.new(SECRET_KEY, f"{h}.{b}".encode(), hashlib.sha256).digest()
        if not hmac.compare_digest(_b64url_enc(expected), s):
            return None
        payload = json.loads(_b64url_dec(b))
        if payload.get("exp", 0) < time.time():
            return None
        return payload
    except Exception:
        return None

# ── Cookie helpers ─────────────────────────────────────────────────────────────
def get_cookie_token() -> str | None:
    """Read the session token from the incoming Cookie header."""
    raw = os.environ.get("HTTP_COOKIE", "")
    if not raw:
        return None
    c = SimpleCookie()
    c.load(raw)
    morsel = c.get(COOKIE_NAME)
    return morsel.value if morsel else None

def make_set_cookie(token: str) -> str:
    """Build a Set-Cookie header value for login."""
    flags = [
        f"{COOKIE_NAME}={token}",
        f"Max-Age={TOKEN_LIFETIME}",
        "Path=/",
        "HttpOnly",
        "SameSite=Strict",
    ]
    if HTTPS:
        flags.append("Secure")
    return "; ".join(flags)

def make_clear_cookie() -> str:
    """Build a Set-Cookie header that immediately expires the cookie."""
    flags = [
        f"{COOKIE_NAME}=",
        "Max-Age=0",
        "Path=/",
        "HttpOnly",
        "SameSite=Strict",
    ]
    if HTTPS:
        flags.append("Secure")
    return "; ".join(flags)

# ── Response helpers ──────────────────────────────────────────────────────────
STATUS_TEXT = {
    200: "OK", 201: "Created", 204: "No Content",
    400: "Bad Request", 401: "Unauthorized", 403: "Forbidden",
    404: "Not Found", 405: "Method Not Allowed", 500: "Internal Server Error",
}

def send_response(status: int, data: dict, extra_headers=None):
    print(f"Status: {status} {STATUS_TEXT.get(status, 'Unknown')}")
    print("Content-Type: application/json")
    origin = os.environ.get("HTTP_ORIGIN", "")
    if origin:
        print(f"Access-Control-Allow-Origin: {origin}")
        print("Access-Control-Allow-Credentials: true")
    print("Access-Control-Allow-Headers: Content-Type, X-CSRF-Token")
    print("Access-Control-Allow-Methods: GET, POST, OPTIONS")
    for h in (extra_headers or []):
        print(h)
    print()
    print(json.dumps(data))

def require_auth() -> dict | None:
    token = get_cookie_token()
    if not token:
        return None
    return verify_token(token)

def read_body() -> dict:
    try:
        length = int(os.environ.get("CONTENT_LENGTH", 0))
        raw    = sys.stdin.read(length)
        return json.loads(raw) if raw else {}
    except Exception:
        return {}

def load_json(filename: str):
    with open(DATA_DIR / filename) as f:
        return json.load(f)

# ── Route handlers ────────────────────────────────────────────────────────────

def handle_login():
    body     = read_body()
    username = (body.get("username") or "").strip().lower()
    password = body.get("password") or ""

    if not username or not password:
        return send_response(400, {"success": False, "message": "Username and password required."})

    try:
        users_db = load_json("users.json")
    except Exception:
        return send_response(500, {"success": False, "message": "Internal database error."})

    user = next(
        (u for u in users_db["users"] if u["username"] == username and u["password"] == password),
        None
    )

    if not user:
        return send_response(401, {
            "success": False,
            "message": "ACCESS DENIED. Credentials not recognised. "
                       "If you believe this is an error, please complete "
                       "Form 17-B (Wrongful Denial of Existence)."
        })

    token = create_token({"username": user["username"], "clearance": user["clearance"]})

    # Token goes into the HttpOnly cookie — NOT the response body.
    # JS only receives the user object, never the raw token.
    send_response(200,
        {
            "success":   True,
            "expiresIn": TOKEN_LIFETIME,
            "user": {
                "username": user["username"],
                "name":     user["name"],
                "clearance":user["clearance"],
                "role":     user["role"],
            }
        },
        extra_headers=[f"Set-Cookie: {make_set_cookie(token)}"]
    )


def handle_logout():
    # Overwrite cookie with an expired one — gone from browser immediately.
    send_response(200,
        {"success": True, "message": "Session terminated. Dimension door sealed."},
        extra_headers=[f"Set-Cookie: {make_clear_cookie()}"]
    )


def handle_validate():
    payload = require_auth()
    if not payload:
        return send_response(401, {"valid": False, "message": "Token invalid or expired."})
    send_response(200, {"valid": True, "username": payload["username"]})


def handle_refresh():
    payload = require_auth()
    if not payload:
        return send_response(401, {"success": False, "message": "Cannot refresh: token invalid."})
    new_token = create_token({"username": payload["username"], "clearance": payload["clearance"]})
    send_response(200,
        {"success": True, "expiresIn": TOKEN_LIFETIME},
        extra_headers=[f"Set-Cookie: {make_set_cookie(new_token)}"]
    )


def handle_profile():
    payload = require_auth()
    if not payload:
        return send_response(401, {"success": False, "message": "Not authenticated."})
    try:
        users_db = load_json("users.json")
        user = next((u for u in users_db["users"] if u["username"] == payload["username"]), None)
        if not user:
            return send_response(404, {"success": False, "message": "Agent record not found."})
        send_response(200, {
            "username": user["username"],
            "name":     user["name"],
            "clearance":user["clearance"],
            "role":     user["role"],
        })
    except Exception as e:
        send_response(500, {"success": False, "message": str(e)})


def handle_reports():
    if not require_auth():
        return send_response(401, {"success": False, "message": "Clearance required."})
    try:
        send_response(200, load_json("reports.json"))
    except Exception as e:
        send_response(500, {"success": False, "message": str(e)})


def handle_bestiary():
    if not require_auth():
        return send_response(401, {"success": False, "message": "Clearance required."})
    try:
        send_response(200, load_json("bestiary.json"))
    except Exception as e:
        send_response(500, {"success": False, "message": str(e)})


def handle_artifacts():
    if not require_auth():
        return send_response(401, {"success": False, "message": "Clearance required."})
    try:
        send_response(200, load_json("artifacts.json"))
    except Exception as e:
        send_response(500, {"success": False, "message": str(e)})


# ── Router ────────────────────────────────────────────────────────────────────

def main():
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    path   = os.environ.get("PATH_INFO", os.environ.get("SCRIPT_NAME", ""))

    if path.startswith("/api"):
        path = path[4:]

    if method == "OPTIONS":
        origin = os.environ.get("HTTP_ORIGIN", "")
        print("Status: 204 No Content")
        if origin:
            print(f"Access-Control-Allow-Origin: {origin}")
            print("Access-Control-Allow-Credentials: true")
        print("Access-Control-Allow-Headers: Content-Type, X-CSRF-Token")
        print("Access-Control-Allow-Methods: GET, POST, OPTIONS")
        print()
        return

    routes = {
        ("POST", "/auth/login"):    handle_login,
        ("POST", "/auth/logout"):   handle_logout,
        ("POST", "/auth/validate"): handle_validate,
        ("POST", "/auth/refresh"):  handle_refresh,
        ("GET",  "/users/profile"): handle_profile,
        ("GET",  "/reports"):       handle_reports,
        ("GET",  "/bestiary"):      handle_bestiary,
        ("GET",  "/artifacts"):     handle_artifacts,
    }

    handler = routes.get((method, path))
    if handler:
        handler()
    else:
        send_response(404, {
            "success": False,
            "message": f"Route '{method} {path}' not found. "
                       "Please consult Form 404-B (Missing Endpoint, Presumed Dimensional)."
        })

if __name__ == "__main__":
    main()
