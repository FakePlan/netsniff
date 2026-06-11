#include <iostream>
#include <sqlite3.h>
#include <string>

void generateReport() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("packets.db", &db);
    if (rc) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::cout << "=== NetSniff Report ===" << std::endl;

    const char* sqlTotal = "SELECT COUNT(*) FROM packets;";

    if (sqlite3_prepare_v2(db, sqlTotal, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int totalPackets = sqlite3_column_int(stmt, 0);
            std::cout << "Total packets: " << totalPackets << std::endl;
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "Top 5 Source IPs:" << std::endl;
    const char* sqlTopIps = "SELECT src_ip, COUNT(*) as count FROM packets GROUP BY src_ip ORDER BY count DESC LIMIT 5;";

    if (sqlite3_prepare_v2(db, sqlTopIps, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* ip = sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);

            if (ip) {
                std::cout << "  " << ip << "  ->  " << count << " packets" << std::endl;
            }
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "Protocol breakdown:" << std::endl;
    const char* sqlProto = "SELECT protocol, COUNT(*) as count FROM packets GROUP BY protocol ORDER BY count DESC;";

    if (sqlite3_prepare_v2(db, sqlProto, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* protocol = sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);

            if (protocol) {
                std::cout << "  " << protocol << "   ->  " << count << std::endl;
            }
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "======================" << std::endl;

    sqlite3_close(db);
}


