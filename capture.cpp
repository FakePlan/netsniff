#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

void parsePacket(unsigned char* buffer, int bytes);

int createSocket() {
	int sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sd == -1) {
		perror("Socket could not be opened");
		exit(1);

	}
	return sd;
}

void startCapture(int sd) {
	unsigned char buffer[65536];

	while (true) {
		int bytes = recvfrom(sd, buffer, sizeof(buffer), 0, NULL, NULL);
		std::cout << "Packet received: " << bytes << " bytes" << std::endl;
		parsePacket(buffer, bytes);
	}
}