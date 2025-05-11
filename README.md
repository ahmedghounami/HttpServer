# 🌐 Webserv

> "This is when you finally understand why a URL starts with HTTP."

A high-performance, non-blocking HTTP/1.1 web server written from scratch in **C++98** — built as part of the 1337/42 curriculum.  
Developed in collaboration with **Hamza Boudar** and **Mehdi Kibous**.

---

## 🚀 Project Overview

**Webserv** is a fully functional HTTP server that can:

- Serve static websites
- Handle **GET**, **POST**, and **DELETE** requests
- Process **CGI scripts** (e.g., PHP or Python)
- Support **file uploads**
- Deliver custom **error pages**
- Read from a configuration file similar to NGINX
- Run in a **non-blocking**, poll-driven architecture (1 `poll()` loop only)
- Serve multiple ports and virtual hosts

This project helped us dive deep into:

- HTTP protocol internals
- I/O multiplexing with `poll()`
- Socket programming
- Configuration parsing and dynamic routing
- Process management and CGI execution
- Error handling and resilience

---

## 🛠 Technologies & Constraints

- 🧠 Language: `C++98`
- 📄 No external libraries (only syscalls allowed)
- ⚙️ Network: `socket`, `bind`, `listen`, `accept`, `poll`, `send`, `recv`, etc.
- 🐚 Process: `fork`, `execve`, `pipe`, etc. (used only for CGI)
- 🐙 Compliant with HTTP/1.1 (tested via curl, browsers, and NGINX)

---

## ⚙️ How It Works

Run the server with a config file:

```bash
./webserv config/example.conf
```

The config file lets you:

- Set hosts and ports
- Define multiple `server` blocks (virtual hosts)
- Setup routing with custom rules
- Enable/disable directory listing
- Specify default files, error pages, upload paths
- Configure CGI based on extensions

---

## 🔍 Features Demo

- ✅ Serve a static HTML website
- ✅ Run PHP scripts using CGI
- ✅ Upload files via HTML form
- ✅ Configure different routes with limits and permissions
- ✅ Return proper HTTP response codes
- ✅ Compatible with browsers like Firefox, Chrome

---

### 👥 Contributors

- [**Ahmed Ghounami**](#)
- [**Hamza Boudar**](https://github.com/hboudar)
- [**Mehdi Kibous**](https://github.com/mkibous)

---

<p align="center">
  <a href="https://profile.intra.42.fr/users/aghounam" target="_blank">
    <img src="https://profile.intra.42.fr/users/aghounam/avatar" alt="Ahmed Ghounami" width="96" height="96" style="border-radius: 50%; border: 1px solid #9d99a0;" />
  </a>
  &nbsp;
  <a href="https://profile.intra.42.fr/users/mkibous" target="_blank">
    <img src="https://profile.intra.42.fr/users/mkibous/avatar" alt="Mehdi Kibous" width="96" height="96" style="border-radius: 50%; border: 1px solid #9d99a0;" />
  </a>
  &nbsp;
  <a href="https://profile.intra.42.fr/users/hboudar" target="_blank">
    <img src="https://profile.intra.42.fr/users/hboudar/avatar" alt="Hamza Boudar" width="96" height="96" style="border-radius: 50%; border: 1px solid #9d99a0;" />
  </a>
</p>

---

### 👥 Contributors




## 📸 Screenshots & Demos

> [Add GIFs, screenshots, or terminal recordings here showing file uploads, routing, error pages, etc.]

---
### 📁 Project Structure

```
www.webserv/
│
├── CGI/               # CGI execution logic (PHP/Python)
├── HandlingRequest/   # HTTP request handling logic
├── Makefile
├── ParsingRequest/    # Request parsing and validation
├── config/            # Sample server configuration files
├── errors/            # Default error pages
├── main.cpp           # Main server entry point
├── response/          # HTTP response building and sending
├── server/            # Server class and listener logic
├── server.hpp         # Server header definitions
├── upload/            # File upload directory
└── www/               # Static website root
```


---

## 📚 What We Learned

- Low-level HTTP mechanics
- Non-blocking architecture using `poll()`
- Building robust and secure C++ software
- How NGINX configurations work internally
- Writing safe and efficient socket-based servers

---

## 🧪 Bonus Features (Optional)

- Session and cookie support
- Multiple CGI types
- Chunked transfer decoding
- Stress-tested stability

---

## 📜 License

This project is part of the 1337/42 School curriculum and is for educational purposes only.
