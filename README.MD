# C++ Networking Projects Collection

This repository houses two distinct C++ applications that demonstrate networking capabilities at different layers of the OSI model. While both projects utilize C++ for high-performance data handling, they serve vastly different purposes:

1.  **Deep Packet Inspection (DPI) Firewall (Layer 3/4):** A security tool for packet interception and filtering.
2.  **Twitch Stream Aggregator (Layer 7):** A multimedia web server for content aggregation.

# 1. Deep Packet Inspection (DPI) Firewall (Layer 3/4)

This project implements a simple userspace firewall using the Linux `libnetfilter_queue` library. It inspects TCP packets destined for a specific port and drops them if the payload contains a specific signature (in this case, the string `"virus"`).

It demonstrates how to move packet filtering logic from kernel space to user space, allowing for complex logic (like string matching) using C++.

## Prerequisites

To compile and run this application, you need a Linux environment with the following libraries and tools installed:

* **g++** (C++ Compiler)
* **libnetfilter-queue-dev** (Development headers for Netfilter Queue)
* **libnfnetlink-dev** (Development headers for Netlink)
* **iptables** (To direct traffic to the userspace queue)
* **netcat (nc)** (For testing traffic)

### Installation & running

```bash
# build the image
docker image build -t cpp-firewall .

# run the container
docker run --name my-firewall --cap-add=NET_ADMIN cpp-firewall

# run the server from terminal-1
nc -lk -p 8080

# run the client from terminal-2
echo "This should be okay" | nc -w 1 -q 0 127.0.0.1 8080
echo "virus is not okay" | nc -w 1 -q 0 127.0.0.1 8080
```

# 2. Twitch Stream Aggregator (Layer 7 Application)

**File:** `main.cpp`

This project is a high-performance, multi-threaded web server designed to aggregate live streams from Twitch. Unlike standard embedded players, this application resolves the raw video stream URLs (HLS/m3u8) on the backend and serves a custom, lightweight HTML dashboard.

It demonstrates **OSI Layer 7 (Application Layer)** concepts by integrating system shell commands, C++ logic, and HTTP serving into a single executable.

---

## 🚀 Features

* **Backend URL Resolution:** Bypasses the need for heavy Twitch embed scripts by fetching raw stream URLs directly using `streamlink`.
* **In-Memory Caching:** Stores resolved URLs in a thread-safe `std::map` to serve requests instantly without re-fetching data.
* **Embedded HLS Player:** Dynamically generates an HTML5 dashboard using `hls.js`, ensuring playback compatibility across all modern browsers (Chrome, Firefox, Safari).
* **Concurrency:** Utilizes C++ `std::mutex` to ensure safe data access in a multi-threaded web server environment.

---

## 🛠️ Prerequisites

Before compiling, ensure your development environment has the following tools and libraries installed.

### 1. System Dependencies
* **g++**: A C++ compiler with C++11 support or higher.
* **Python 3 & Pip**: Required to install the stream resolver.

### 2. Streamlink (The Engine)
This application relies on the `streamlink` CLI tool to handle the complex handshake with Twitch servers and extract the raw `.m3u8` playlist URL.

```bash
pip install streamlink
```