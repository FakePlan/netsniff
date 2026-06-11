#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <iostream>
#include <csignal>
#include <cstdlib>

int createSocket();
void startCapture(int sd);
void initDatabase();
void generateReport();

void signalHandler(int signum) {
    std::cout << "\n\n[!] Capture stopped. Preparing report...\n" << std::endl;
    generateReport();
    exit(signum);
}

int main() {
    signal(SIGINT, signalHandler);

    std::cout << "NetSniff starting..." << std::endl;

    initDatabase();

    int sd = createSocket();
    if (sd < 0) {
        std::cerr << "Failed to create socket. (Make sure you run with root privileges!)" << std::endl;
        return 1;
    }

    std::cout << "Socket opened! ID: " << sd << std::endl;
    std::cout << "Listening for packets... (Press Ctrl+C to stop and see the report)" << std::endl;

    startCapture(sd);

    generateReport();

    return 0;
}