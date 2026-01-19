# Use a lightweight Ubuntu base
FROM ubuntu:22.04

# Avoid interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# 1. Install Dependencies
# - build-essential: g++ and make
# - libnetfilter-queue-dev: The firewall library
# - iptables: To redirect traffic to our app
# - netcat: To test the connection
# - iproute2: For network debugging (ip addr)
RUN apt-get update && apt-get install -y \
    build-essential \
    libnetfilter-queue-dev \
    libnfnetlink-dev \
    iptables \
    netcat \
    iproute2 \
    && rm -rf /var/lib/apt/lists/*

# 2. Set working directory
WORKDIR /app

# 3. Copy source code
COPY firewall.cpp .

# 4. Compile the application
RUN g++ -o firewall firewall.cpp -lnetfilter_queue

# 5. Create a startup script
# We need this because we must run the iptables command *at runtime*, 
# not build time.
RUN echo '#!/bin/bash\n\
# Redirect TCP traffic on port 8080 to Queue 0\n\
iptables -A INPUT -p tcp --dport 8080 -j NFQUEUE --queue-num 0\n\
echo "IPtables rules configured."\n\
# Run the firewall app\n\
./firewall' > entrypoint.sh && chmod +x entrypoint.sh

# 6. Default Command
CMD ["./entrypoint.sh"]