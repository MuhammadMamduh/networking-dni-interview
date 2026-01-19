# C++ Deep Packet Inspection (DPI) Firewall

This project implements a simple userspace firewall using the Linux `libnetfilter_queue` library. It inspects TCP packets destined for a specific port and drops them if the payload contains a specific signature (in this case, the string `"virus"`).

It demonstrates how to move packet filtering logic from kernel space to user space, allowing for complex logic (like string matching) using C++.

## Prerequisites

To compile and run this application, you need a Linux environment with the following libraries and tools installed:

* **g++** (C++ Compiler)
* **libnetfilter-queue-dev** (Development headers for Netfilter Queue)
* **libnfnetlink-dev** (Development headers for Netlink)
* **iptables** (To direct traffic to the userspace queue)
* **netcat (nc)** (For testing traffic)

### Installation (Debian/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install build-essential libnetfilter-queue-dev libnfnetlink-dev iptables netcat