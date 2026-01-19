#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>      /* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string>
#include <iostream>

// Check payload for "virus"
bool contains_virus_signature(unsigned char* data, int len) {
    std::string payload(reinterpret_cast<const char*>(data), len);
    if (payload.find("virus") != std::string::npos) {
        return true;
    }
    return false;
}

// Callback function for every packet
static int packet_callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                           struct nfq_data *nfa, void *data) {
    uint32_t id = 0;
    struct nfqnl_msg_packet_hdr *ph;
    unsigned char *raw_data;
    int ret;

    ph = nfq_get_msg_packet_hdr(nfa);
    if (ph) {
        id = ntohl(ph->packet_id);
    }

    ret = nfq_get_payload(nfa, &raw_data);
    if (ret >= 0) {
        struct iphdr *ip_header = (struct iphdr *)raw_data;

        if (ip_header->protocol == IPPROTO_TCP) {
            int ip_header_len = ip_header->ihl * 4;
            struct tcphdr *tcp_header = (struct tcphdr *)(raw_data + ip_header_len);
            int tcp_header_len = tcp_header->doff * 4;
            int total_header_len = ip_header_len + tcp_header_len;
            int payload_len = ret - total_header_len;
            unsigned char *payload = raw_data + total_header_len;

            if (payload_len > 0) {
                if (contains_virus_signature(payload, payload_len)) {
                    std::cout << "[BLOCKED] Packet " << id << " contained 'virus'!" << std::endl;
                    return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
                }
            }
        }
    }
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

int main(int argc, char **argv) {
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    int fd;
    int rv;
    char buf[4096] __attribute__ ((aligned));

    std::cout << "Starting Containerized L4 Firewall..." << std::endl;

    // Open Library Handle
    h = nfq_open();
    if (!h) {
        fprintf(stderr, "error during nfq_open()\n");
        exit(1);
    }

    // Unbind/Bind
    if (nfq_unbind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "error during nfq_unbind_pf()\n");
        exit(1);
    }
    if (nfq_bind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "error during nfq_bind_pf()\n");
        exit(1);
    }

    // Bind to Queue 0
    qh = nfq_create_queue(h,  0, &packet_callback, NULL);
    if (!qh) {
        fprintf(stderr, "error during nfq_create_queue()\n");
        exit(1);
    }

    // Set Copy Mode
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
        fprintf(stderr, "can't set packet_copy mode\n");
        exit(1);
    }

    fd = nfq_fd(h);

    while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
        nfq_handle_packet(h, buf, rv);
    }

    nfq_destroy_queue(qh);
    nfq_close(h);
    return 0;
}