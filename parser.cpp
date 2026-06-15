#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

void savePacket(const std::string& src_ip, const std::string& dst_ip, const std::string& protocol, int size);

std::string resolveHostname(const std::string& ip_address) {
    struct sockaddr_in sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;

    if (inet_pton(AF_INET, ip_address.c_str(), &sa.sin_addr) <= 0) {
        return ip_address;
    }

    char host[NI_MAXHOST];
    if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), nullptr, 0, NI_NAMEREQD) == 0) {
        return std::string(host);
    }
    return ip_address;
}

void printHexDump(const unsigned char* payload, int length) {
    if (length <= 0) return;

    std::cout << "Payload (" << length << " bytes):\n";
    for (int i = 0; i < length; i += 16) {
        printf("%04X  ", i);
        for (int j = 0; j < 16; j++) {
            if (i + j < length) printf("%02X ", payload[i + j]);
            else printf("   ");
        }
        printf(" | ");
        for (int j = 0; j < 16; j++) {
            if (i + j < length) {
                unsigned char c = payload[i + j];
                if (c >= 32 && c <= 126) printf("%c", c);
                else printf(".");
            }
        }
        printf("\n");
    }
}

bool parsePacket(unsigned char* buffer, int bytes, const std::string& target_protocol, const std::string& target_source) {
    if (bytes < 34) return false;

    struct ethhdr* eth = (struct ethhdr*)buffer;
    if (ntohs(eth->h_proto) != ETH_P_IP) return false;

    struct iphdr* ip = (struct iphdr*)(buffer + 14);

    struct in_addr source_ip;
    source_ip.s_addr = ip->saddr;

    struct in_addr dest_ip;
    dest_ip.s_addr = ip->daddr;

    std::string proto;
    if (ip->protocol == 6) proto = "TCP";
    else if (ip->protocol == 17) proto = "UDP";
    else if (ip->protocol == 1) proto = "ICMP";
    else proto = "OTHER";

    std::string src_ip_str = inet_ntoa(source_ip);
    std::string dst_ip_str = inet_ntoa(dest_ip);

    if (!target_protocol.empty() && proto != target_protocol) {
        return false;
    }
    if (!target_source.empty() && src_ip_str != target_source) {
        return false;
    }

    int src_port = 0;
    int dst_port = 0;
    int ip_header_len = ip->ihl * 4;

    if (ip->protocol == 6) {
        struct tcphdr* tcp = (struct tcphdr*)(buffer + 14 + ip_header_len);
        src_port = ntohs(tcp->source);
        dst_port = ntohs(tcp->dest);
    }
    else if (ip->protocol == 17) {
        struct udphdr* udp = (struct udphdr*)(buffer + 14 + ip_header_len);
        src_port = ntohs(udp->source);
        dst_port = ntohs(udp->dest);
    }

    int header_size = 14 + ip_header_len;
    if (ip->protocol == 6) {
        struct tcphdr* tcp = (struct tcphdr*)(buffer + 14 + ip_header_len);
        header_size += tcp->doff * 4;
    }
    else if (ip->protocol == 17) {
        header_size += 8;
    }

    int payload_size = bytes - header_size;
    unsigned char* payload = buffer + header_size;

    std::cout << "--- New Packet ---" << std::endl;
    std::cout << "Size:     " << bytes << " bytes" << std::endl;
    std::cout << "Protocol: " << proto << std::endl;

    if (src_port != 0 || dst_port != 0) {
        std::cout << "Source:   " << src_ip_str << " : " << src_port << std::endl;
        std::cout << "Dest:     " << dst_ip_str << " : " << dst_port << std::endl;
    }
    else {
        std::cout << "Source:   " << src_ip_str << std::endl;
        std::cout << "Dest:     " << dst_ip_str << std::endl;
    }
    std::cout << std::endl;

    if (payload_size > 0) {
        printHexDump(payload, payload_size);
    }

    std::cout << "------------------\n" << std::endl;

    savePacket(src_ip_str, dst_ip_str, proto, bytes);

    return true;
}