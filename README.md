*this project has been created as part of the 42 curriculum by < vdurand >, < antbonin >.*

## Table of Contents
1. [Description](#-description)
2. [Features](#-features)
3. [Architecture & Design](#-architecture--design)
4. [Getting Started (Instructions)](#-getting-started)
5. [Resources](#-resources)

## Description

Webserv is a fully functional, non-blocking HTTP/1.1 server written from scratch in **C++98**.\
Inspired by NGINX, it uses multiplexing (`epoll`) to handle thousands of concurrent connections efficiently without threads or processes.

## Features

- Fully compliant with **HTTP/1.1** standard.
- **I/O Multiplexing** using `epoll` for high performance.
- Supports **GET, POST, DELETE, HEAD and PUT** methods.
- **CGI Execution** (e.g., PHP, Python scripts).
- Support for **Chunked Transfer-Encoding** requests.
- Custom routing and virtual servers parsing via a `.conf` file.
- Static file serving with **Autoindex** directory listing.
- Handling 35 HTTP responses code from 100 - Continue to 505 - HTTP Version no supported
- Low level implementation of an HashMap in `c++98` and macrosplosion for easier manipulation of enum.
- Absolute parser of config file in strict format.

## Architecture & design

### Core system
- **State Machine:** Request parsing is built as a state machine to handle partial reads over non-blocking sockets.
- **Memory Pool & Intrusive Free List:** Custom memory management to prevent fragmentation and allocations during heavy loads (handling `wrk` stress tests flawlessly).
- **RAII Concept:** Strict memory and file descriptor management to ensure zero memory leaks.
- **Design Patterns:** Usage of Factory pattern for requests, Handlers for specific logic, etc.

### Advanced customs data structures 
Because of the strict 42 C++98 constraints, we (Vdurand) implemented highly optimized data structures from scratch:
- **Radix Tree (Trie):** Implemented for ultra-fast and optimized HTTP route matching (instead of iterating through vectors).
- **Hashed Timing Wheels:** An $O(1)$ complexity timer management system used to efficiently track and drop timed-out or idle connections (`closing_timeout`).
- **Custom HashMap:** A low-level C++98 implementation used for blazing-fast header lookups.
- **Integer Utils & Macrosplosion:** Deep C++98 metaprogramming and macro architectures for easier manipulation of enums and memory limits.

## Getting started

### Prerequisites
- `g++` or `clang++` compiler
- `make`

**Note:** The project is strictly written in **C++98**. The provided `Makefile` automatically enforces this standard during compilation using the `-std=c++98` flag.

### Installation & run
```bash
git clone https://github.com/Furia25/webserv.git && cd webserv
make
Usage : ./webserv + [config-file.toml]
```
### Configuration preview
```
[engine]
max_events = 1024
max_timeout = 50
closing_timeout = 30
read_size = 4096
max_read_limit = 16384

[logging]
tick_interval = 5
level = "DEBUG"

[[servers]]
server_name = "main"
root = "www/basic_website"
max_body_size = 15000000

	# Static files route
	[[servers.routes]]
	handler = "STATIC"
	path = "/"
	alias = "/static/"
	methods = ["GET"]
	index = "index.html"
	autoindex = false
```

## Resources

--_ANTBONIN Ressources_--\
**HTTP/1.1** :\
*RFC parsing & security norm -- https://datatracker.ietf.org/doc/html/rfc7230*\
*RFC semantics of http msg -- https://datatracker.ietf.org/doc/html/rfc7231*

*Design Pattern (RAII - Machine State - DOD) -- https://refactoring.guru/* \
*HTTP Request -- https://developer.mozilla.org/fr/docs/Web/HTTP*\
*Free List -- https://www.youtube.com/watch?v=MxgnS9Lwv0k*

*I used AI primarily as a technical mentor to learn new design patterns, understand systems philosophy, and optimize my code.*\
*It also acted as a peer reviewer to ensure strict HTTP RFC compliance, and served as an invaluable debugging assistant.*\
*Particularly for tracking down memory leaks after many refacto.*

---

--_VDURAND Ressources_--\
he just need his brain he's a genius fr (he also was my technical mentor on this project with deep deep understanding of low level implementaion and design pattern)