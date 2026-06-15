#include <iostream>
#include <sqlite3.h>
#include <string>

sqlite3* db_conn = nullptr;
int session_start_id = 0;

void initDatabase() {
    int rc = sqlite3_open("/tmp/packets.db", &db_conn);
    if (rc) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db_conn) << std::endl;
        return;
    }

    sqlite3_exec(db_conn, "PRAGMA synchronous = OFF;", nullptr, nullptr, nullptr);
    sqlite3_exec(db_conn, "PRAGMA journal_mode = MEMORY;", nullptr, nullptr, nullptr);

    const char* sql = "CREATE TABLE IF NOT EXISTS packets ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "timestamp TEXT, "
        "src_ip TEXT, "
        "dst_ip TEXT, "
        "protocol TEXT, "
        "size INTEGER);";

    char* zErrMsg = nullptr;
    rc = sqlite3_exec(db_conn, sql, nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    sqlite3_stmt* stmt_id;
    if (sqlite3_prepare_v2(db_conn, "SELECT MAX(id) FROM packets;", -1, &stmt_id, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt_id) == SQLITE_ROW) {
            session_start_id = sqlite3_column_int(stmt_id, 0);
        }
    }
    sqlite3_finalize(stmt_id);

    sqlite3_exec(db_conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
}

void savePacket(const std::string& src_ip, const std::string& dst_ip, const std::string& protocol, int size) {
    if (!db_conn) return;

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO packets (timestamp, src_ip, dst_ip, protocol, size) VALUES (CURRENT_TIMESTAMP, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db_conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement" << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, src_ip.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, dst_ip.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, protocol.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, size);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void commitDatabase() {
    if (db_conn) {
        std::cout << "[!] Saving packets from RAM to disk... Please wait." << std::endl;
        sqlite3_exec(db_conn, "COMMIT;", nullptr, nullptr, nullptr);
        sqlite3_close(db_conn);
    }
}