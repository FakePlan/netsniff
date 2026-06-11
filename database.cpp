#include <iostream>
#include <sqlite3.h>
#include <string>

void initDatabase() {
    sqlite3* db;
    char* zErrMsg = nullptr;
    int rc;

    rc = sqlite3_open("packets.db", &db);
    if (rc) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS packets ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "timestamp TEXT, "
        "src_ip TEXT, "
        "dst_ip TEXT, "
        "protocol TEXT, "
        "size INTEGER);";

    rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
}

void savePacket(const std::string& src_ip, const std::string& dst_ip, const std::string& protocol, int size) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (sqlite3_open("packets.db", &db) != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql = "INSERT INTO packets (timestamp, src_ip, dst_ip, protocol, size) "
        "VALUES (CURRENT_TIMESTAMP, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, src_ip.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, dst_ip.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, protocol.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, size);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to save packet: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}