#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <string>

int createSocket();
void startCapture(int sd, int limit, const std::string& target_protocol, const std::string& target_source);
void initDatabase();
void commitDatabase();
void generateReport();

void signalHandler(int signum) {
    std::cout << "\n\n[!] Capture stopped. Preparing report...\n" << std::endl;
    commitDatabase();
    generateReport();
    exit(signum);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);

    int packet_limit = 0;
    std::string target_protocol = "";
    std::string target_source = "";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--count" && i + 1 < argc) {
            packet_limit = std::atoi(argv[i + 1]);
            i++;
        }
        else if (arg == "--protocol" && i + 1 < argc) {
            target_protocol = argv[i + 1];
            i++;
        }
        else if (arg == "--source" && i + 1 < argc) {
            target_source = argv[i + 1];
            i++;
        }
    }

    std::cout << "NetSniff starting..." << std::endl;

    if (packet_limit > 0) std::cout << "[!] Capture limit set to: " << packet_limit << " packets." << std::endl;
    else std::cout << "[!] Running in unlimited mode." << std::endl;

    if (!target_protocol.empty()) std::cout << "[!] Protocol filter active: " << target_protocol << std::endl;
    if (!target_source.empty()) std::cout << "[!] Source IP filter active: " << target_source << std::endl;

    initDatabase();

    int sd = createSocket();
    if (sd < 0) {
        std::cerr << "Failed to create socket. (Make sure you run with root privileges!)" << std::endl;
        return 1;
    }

    std::cout << "Socket opened! ID: " << sd << std::endl;
    std::cout << "Listening for packets... (Press Ctrl+C to stop)" << std::endl;

    startCapture(sd, packet_limit, target_protocol, target_source);

    if (packet_limit > 0) {
        std::cout << "\n[!] Target packet limit reached. Auto-stopping..." << std::endl;
        commitDatabase();
        generateReport();
    }

    return 0;
}