# webserv

> A high-performance, non-blocking HTTP/1.1 server written from scratch in C++98 — no threads, no frameworks, no shortcuts.

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
6. [Getting Started](#getting-started)
5. [Configuration](#configuration)
3. [Architecture](#architecture)
4. [Custom Data Structures](#custom-data-structures)
7. [Resources](#resources)

---

## Overview

`webserv` is a fully functional HTTP/1.1 server inspired by NGINX's vision. It handles thousands of concurrent connections using a single-threaded, event-driven model backed by Linux's `epoll` : zero threads, zero blocking. Every component, from the HTTP parser to the timeout manager, was built by hand under strict C++98 constraints, zero external libraries.

---

## Features

| Category | Details |
|---|---|
| **Protocol** | Full HTTP/1.1 compliance — 35 status codes from `100 Continue` to `505 HTTP Version Not Supported` |
| **Methods** | GET, POST, DELETE, HEAD, PUT |
| **I/O Model** | Non-blocking sockets, `epoll`-based event loop |
| **CGI** | Python, PHP, and custom interpreter execution via `fork`/`execve` with pipe-based async I/O |
| **Body Handling** | Chunked Transfer-Encoding, streaming large bodies to disk |
| **Routing** | Virtual servers, per-route handlers, `alias`, `autoindex` |
| **File Serving** | Static files, directory listing, upload handler |
| **Cookies** | Per-server and per-route cookie generation with `HttpOnly`, `SameSite`, `Max-Age` |
| **Configuration** | Custom TOML parser — strict format, human-readable, fully validated at startup |
| **Logging** | Leveled logger (`DEBUG` / `INFO` / `FATAL`) with configurable tick interval and file output |

---

## Getting Started

### Prerequisites

- `g++` or `clang++` with C++98 support <br> *(Non-standard compiler extensions are used gcc/clang recommended)*
- `make`
- Linux (requires `epoll`)

### Build & Run

```bash
git clone https://github.com/Furia25/webserv.git
cd webserv
make
./webserv config/exemple_config.toml
```

---

## Configuration

`webserv` uses a [**TOML/1.1.0**](https://toml.io/en/) configuration format, parsed by a hand-written tokenizer and parser that strictly validates the standards.

The full reference with every available key, its type, valid range, and default is available on the wiki page [`Configuration`](https://github.com/Furia25/webserv/wiki/Configuration). Below is a quick overview of the structure.

### Top-level sections

```toml
[engine]          # epoll tuning — max_events, timeouts, read buffer sizes
[logging]         # log level (DEBUG|INFO|FATAL), output file, tick interval
[[servers]]       # one block per virtual server; at least one required
```

### Per-server keys

```toml
[[servers]]
server_name   = "mysite"          # resolved against the Host header
root          = "www/mysite"      # filesystem root (default: "./")
max_body_size = 52428800          # bytes; inherited by routes (default: 10 MB)

  [[servers.bindings]]            # one or more host/port pairs
  host    = "0.0.0.0"
  service = "8080"

  [servers.errors]                # custom error pages, keyed by HTTP code
  404 = "errors/404.html"
  500 = "errors/500.html"

  [servers.cookies.session_id]    # server-wide cookie, inherited by all routes
  generate          = true
  generation_length = 32
  max_age           = 86400
  http_only         = true
  same_site         = "LAX"       # LAX | STRICT | NONE
```

### Route handlers

Each `[[servers.routes]]` block declares one handler. Five types are available:

| Handler | Key fields |
|---|---|
| `STATIC` | `index`, `autoindex`, `alias` |
| `CGI` | `interpreters`, `default_bin`, `cgi_timeout`, `env` |
| `UPLOAD` | `upload_store`, `allowed_extensions`, `allow_overwrite` |
| `REDIRECT` | `location`, `status` |
| `STATUS` | `show_start_time`, `show_server_info`, `show_connection_info`, `show_request_info`, `show_timestamp` |

All routes share: `path`, `methods`, `root`, `alias`, `max_body_size`, and an optional `[servers.routes.cookies.*]` block.

Multiple `[[servers]]` blocks enable **virtual hosting** — the server matches the `Host` header against each server's bindings and routes accordingly.

---

## Architecture

### Event Loop

The heart of `webserv` is a tight `epoll_wait` loop in `TCPServer::run()`. Every file descriptor — client sockets, CGI pipes, listener sockets — is registered with `epoll` and dispatched through the `IEpollHandler` interface. There is no blocking operation anywhere on the hot path.

```
epoll_wait()
  └─ IEpollHandler::handleEvent()
       ├─ Listener     → accept new connection → FreeList::acquire()
       ├─ Connection   → read/write → RequestBuilder → HTTPHandler → Router → Handler
       └─ CGIHandler   → read CGI pipe → stream response back to client
```

### Request Parsing — State Machine

`RequestBuilder` incrementally parses HTTP requests as data arrives over non-blocking sockets. It accumulates bytes across multiple `read()` calls, locates the header boundary, validates the request line and each header, then builds an immutable `Request` object. Chunked bodies and large streaming bodies are handled transparently.

### Handler Pipeline

Once a request is routed, a dedicated handler takes over:

- **StaticHandler** - serves files, resolves indexes, generates directory listings
- **CGIHandler** - forks a child process, pipes request body in, reads response back asynchronously via `epoll`
- **UploadHandler** - receives and stores files with extension filtering and overwrite control
- **RedirectHandler** - issues HTTP redirects
- **ErrorHandler** - serves custom error pages per status code
- **StatusHandler** - returns a live server status page (connections, uptime, request info)

### Memory Management

Connections are allocated from a **FreeList** memory pool pre-sized at startup. This eliminates per-connection heap allocations entirely on the hot path, preventing fragmentation under heavy load and keeping `malloc` out of the critical section. RAII is applied consistently — every file descriptor and heap allocation has a clear owner and is released deterministically.

### CGI — Fully Asynchronous

CGI execution never blocks the event loop. After `fork`/`execve`, both the stdin pipe (write to child) and stdout pipe (read from child) are set to non-blocking and registered with `epoll`. Body streaming from disk to the CGI process, and piped output from the child back to the client, are all driven by `epoll` events. A per-CGI timeout is enforced via the timing wheel (see below), and orphaned child processes are reaped via `SIGCHLD`.

---

## Custom Data Structures

The 42 curriculum forbids the C++ STL containers for the core server logic. Every data structure below was implemented from scratch in C++98.

### Radix Tree — Route Matching

Routes are stored in a compressed prefix tree (Radix Tree / Patricia Trie). Matching a request path against all configured routes is a single traversal in **O(k)** — proportional to the URL length, not the number of routes. No linear scan, no `std::map`.

```
/          → StaticHandler
/api       → CGIHandler
/upload    → UploadHandler
/status    → StatusHandler
```

### Hashed Timing Wheel — O(1) Timeout Management

Idle and stalled connections are dropped by a Hashed Timing Wheel. Rather than sorting a priority queue of timers, expiry slots are distributed across a circular array of buckets. Scheduling, cancelling, and firing an alarm are all **O(1)**. The wheel advances once per `epoll_wait` timeout, making connection cleanup essentially free.

```
resolution = EPOLL_TIMEOUT ms
WHEEL_SIZE = 128 buckets
schedule(alarm, ticks) → O(1)
cancel(alarm)          → O(1)
tick()                 → fires expired alarms, advances cursor
```

### Custom HashMap — Header Lookups

HTTP headers are stored in a custom open-addressing `HashMap` with a hand-rolled FNV-based hash. Used for request headers, CGI environment variables, and connection tracking. Constant-time average lookup without the overhead of `std::map`.

### Optional\<T\> — Safe Null Handling

A C++98-compatible `Optional<T>` (backed by aligned storage) replaces pointer-based nullable values throughout the codebase. No raw `NULL` checks, no accidental dereferences.

### Macrosplosion — Compile-Time Enum Metaprogramming

`EnumClass` and the `macrosplosion` macro library implement type-safe enum-to-string conversion, iteration, and validation purely at compile time in C++98. This powers HTTP method and status code handling without `switch` sprawl or runtime maps.

---

## Resources

**HTTP/1.1 specification**
- [RFC 7230 - Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 7231 - Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231)
- [MDN - HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)

**Design & systems**
- [Refactoring Guru - RAII, State Machine, Factory](https://refactoring.guru/)
- [Free List allocator (CppCon)](https://www.youtube.com/watch?v=MxgnS9Lwv0k)

---
*This project has been created as part of the 42 curriculum by [vdurand](https://github.com/vdurand) and [antbonin](https://github.com/Antoine-Bonin).*

---

*We used AI primarily as a technical mentor to understand design patterns and systems philosophy, as a peer reviewer for HTTP RFC compliance, and as a debugging assistant during refactors particularly for tracking memory leaks, time consuming work like README writing as been first generated and then fully reviewed and completed*
