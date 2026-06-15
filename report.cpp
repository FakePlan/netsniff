#include <iostream>
#include <sqlite3.h>
#include <string>

std::string resolveHostname(const std::string& ip_address);
extern int session_start_id;

void generateReport() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (sqlite3_open("/tmp/packets.db", &db)) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::cout << "\n=== NetSniff Session Report ===" << std::endl;

    std::string sqlTotal = "SELECT COUNT(*) FROM packets WHERE id > " + std::to_string(session_start_id) + ";";
    if (sqlite3_prepare_v2(db, sqlTotal.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::cout << "Captured in this session: " << sqlite3_column_int(stmt, 0) << " packets" << std::endl;
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "Top 5 Source IPs (Session):" << std::endl;
    std::string sqlTopIps = "SELECT src_ip, COUNT(*) as count FROM packets WHERE id > " + std::to_string(session_start_id) + " GROUP BY src_ip ORDER BY count DESC LIMIT 5;";
    if (sqlite3_prepare_v2(db, sqlTopIps.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* ip = sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);

            if (ip) {
                std::string ip_str = reinterpret_cast<const char*>(ip);
                std::string hostname = resolveHostname(ip_str);
                std::cout << "  " << ip_str << " (" << hostname << ")  ->  " << count << " packets" << std::endl;
            }
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "Protocol breakdown (Session):" << std::endl;
    std::string sqlProto = "SELECT protocol, COUNT(*) as count FROM packets WHERE id > " + std::to_string(session_start_id) + " GROUP BY protocol ORDER BY count DESC;";
    if (sqlite3_prepare_v2(db, sqlProto.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* protocol = sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);

            if (protocol) {
                std::cout << "  " << protocol << "   ->  " << count << std::endl;
            }
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "===============================\n" << std::endl;
    sqlite3_close(db);
}