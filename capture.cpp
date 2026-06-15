#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <string>

bool parsePacket(unsigned char* buffer, int bytes, const std::string& target_protocol, const std::string& target_source);

int createSocket() {
    int sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sd == -1) {
        perror("Socket could not be opened");
        exit(1);
    }
    return sd;
}

void startCapture(int sd, int limit, const std::string& target_protocol, const std::string& target_source) {
    unsigned char buffer[65536];
    int packet_count = 0;

    while (true) {
        int bytes = recvfrom(sd, buffer, sizeof(buffer), 0, NULL, NULL);

        if (parsePacket(buffer, bytes, target_protocol, target_source)) {
            packet_count++;
            std::cout << "Packet received: " << bytes << " bytes (" << packet_count << ")" << std::endl;

            if (limit > 0 && packet_count >= limit) {
                break;
            }
        }
    }
}