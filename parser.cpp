#include <iostream>
#include <string>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

void savePacket(const std::string& src_ip, const std::string& dst_ip, const std::string& protocol, int size);

void parsePacket(unsigned char* buffer, int bytes) {

    if (bytes < 34) {
        std::cerr << "Error: Packet is too small!" << std::endl;
        return;
    }

    struct ethhdr* eth = (struct ethhdr*)buffer;
    if (ntohs(eth->h_proto) != ETH_P_IP) {
        return;
    }

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

    std::cout << "--- New Packet ---" << std::endl;
    std::cout << "Size:     " << bytes << " bytes" << std::endl;
    std::cout << "Source:   " << src_ip_str << std::endl;
    std::cout << "Dest:     " << dst_ip_str << std::endl;
    std::cout << "Protocol: " << proto << std::endl;
    std::cout << "------------------\n" << std::endl;

    savePacket(src_ip_str, dst_ip_str, proto, bytes);
}