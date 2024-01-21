#include <iostream>
#include "..\Blosoft_DatabaseManager\include\sqlite\sqlite3.h"
#include <vector>
#include <string>

struct DataEntry {
    unsigned long timestamp;
    int dataQuantity;
};

class DatabaseManager {
private:
    sqlite3* db;

    static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
        for (int i = 0; i < argc; i++) {
            std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << std::endl;
        }
        return 0;
    }

public:
    DatabaseManager(const std::string& dbName) {
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            exit(1);
        }

        // Create tables if not exists
        const char* createCategoriesTableQuery = "CREATE TABLE IF NOT EXISTS categories ("
                                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                 "category TEXT NOT NULL);";

        const char* createDataEntriesTableQuery = "CREATE TABLE IF NOT EXISTS data_entries ("
                                                  "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                  "category_id INTEGER NOT NULL,"
                                                  "timestamp INTEGER NOT NULL,"
                                                  "data_quantity INTEGER NOT NULL,"
                                                  "FOREIGN KEY (category_id) REFERENCES categories(id));";

        rc = sqlite3_exec(db, createCategoriesTableQuery, callback, 0, 0);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            exit(1);
        }

        rc = sqlite3_exec(db, createDataEntriesTableQuery, callback, 0, 0);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            exit(1);
        }
    }

    ~DatabaseManager() {
        sqlite3_close(db);
    }

    int insertCategory(const std::string& category) {
        std::string insertCategoryQuery = "INSERT INTO categories (category) VALUES ('" + category + "');";

        int rc = sqlite3_exec(db, insertCategoryQuery.c_str(), callback, 0, 0);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            return -1; // Return -1 on error
        }

        return sqlite3_last_insert_rowid(db);
    }

    void insertDataEntry(int categoryId, const std::vector<DataEntry>& entries) {
        for (const auto& entry : entries) {
            std::string insertDataEntryQuery = "INSERT INTO data_entries (category_id, timestamp, data_quantity) VALUES ("
                                               + std::to_string(categoryId) + ", "
                                               + std::to_string(entry.timestamp) + ", "
                                               + std::to_string(entry.dataQuantity) + ");";

            int rc = sqlite3_exec(db, insertDataEntryQuery.c_str(), callback, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            }
        }
    }

int getCategoryId(const std::string& category) {
    std::string selectCategoryQuery = "SELECT id FROM categories WHERE category = '" + category + "';";

    int categoryId = -1; // Default value if category is not found

    int rc = sqlite3_exec(db, selectCategoryQuery.c_str(), [](void* data, int argc, char** argv, char** azColName) {
        if (argc > 0 && argv[0]) {
            *static_cast<int*>(data) = std::stoi(argv[0]);
        }
        return 0;
    }, &categoryId, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    return categoryId;
}
};