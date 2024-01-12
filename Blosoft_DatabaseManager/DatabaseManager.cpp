// DatabaseManager.cpp

#include <iostream>
#include "sqlite3.h"

class DatabaseManager {
public:
    static void createTableAndViews() {
        sqlite3* db;
        char* errMsg = 0;

        int rc = sqlite3_open("example.db", &db);

        if (rc) {
            std::cerr << "Erreur lors de l'ouverture de la base de données: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        // Création de la table principale
        const char* createTableSQL = "CREATE TABLE Data (id INTEGER PRIMARY KEY, category TEXT, value INTEGER, timestamp DATETIME);";

        rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);

        if (rc != SQLITE_OK) {
            std::cerr << "Erreur lors de la création de la table : " << errMsg << std::endl;
            sqlite3_free(errMsg);
        } else {
            std::cout << "Table créée avec succès." << std::endl;
        }

        // Création de la vue pour l'agrégation par minute
        const char* createViewByMinuteSQL = "CREATE VIEW DataByMinute AS \
                                            SELECT category, SUM(value) AS total_value, \
                                            strftime('%Y-%m-%d %H:%M', timestamp) AS minute_timestamp \
                                            FROM Data GROUP BY category, minute_timestamp;";

        rc = sqlite3_exec(db, createViewByMinuteSQL, 0, 0, &errMsg);

        // Création de la vue pour l'agrégation par heure
        const char* createViewByHourSQL = "CREATE VIEW DataByHour AS \
                                          SELECT category, SUM(total_value) AS total_value, \
                                          strftime('%Y-%m-%d %H:00', minute_timestamp) AS hour_timestamp \
                                          FROM DataByMinute GROUP BY category, hour_timestamp;";

        rc = sqlite3_exec(db, createViewByHourSQL, 0, 0, &errMsg);

        // Création de la vue pour l'agrégation par jour
        const char* createViewByDaySQL = "CREATE VIEW DataByDay AS \
                                         SELECT category, SUM(total_value) AS total_value, \
                                         strftime('%Y-%m-%d', hour_timestamp) AS day_timestamp \
                                         FROM DataByHour GROUP BY category, day_timestamp;";

        rc = sqlite3_exec(db, createViewByDaySQL, 0, 0, &errMsg);

        // Création de la vue pour l'agrégation par semaine
        const char* createViewByWeekSQL = "CREATE VIEW DataByWeek AS \
                                          SELECT category, SUM(total_value) AS total_value, \
                                          strftime('%Y-%W', day_timestamp) AS week_timestamp \
                                          FROM DataByDay GROUP BY category, week_timestamp;";

        rc = sqlite3_exec(db, createViewByWeekSQL, 0, 0, &errMsg);

        // Création de la vue pour l'agrégation par mois
        const char* createViewByMonthSQL = "CREATE VIEW DataByMonth AS \
                                           SELECT category, SUM(total_value) AS total_value, \
                                           strftime('%Y-%m', day_timestamp) AS month_timestamp \
                                           FROM DataByDay GROUP BY category, month_timestamp;";

        rc = sqlite3_exec(db, createViewByMonthSQL, 0, 0, &errMsg);

        // Création de la vue pour l'agrégation par an
        const char* createViewByYearSQL = "CREATE VIEW DataByYear AS \
                                          SELECT category, SUM(total_value) AS total_value, \
                                          strftime('%Y', day_timestamp) AS year_timestamp \
                                          FROM DataByDay GROUP BY category, year_timestamp;";

        rc = sqlite3_exec(db, createViewByYearSQL, 0, 0, &errMsg);

        // Fermeture de la base de données
        sqlite3_close(db);
    }

    static void insertTestData() {
        sqlite3* db;
        char* errMsg = 0;

        int rc = sqlite3_open("example.db", &db);

        if (rc) {
            std::cerr << "Erreur lors de l'ouverture de la base de données: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        // Insertion de données de test
        const char* insertDataSQL = "INSERT INTO Data (category, value, timestamp) VALUES ('example', 10, CURRENT_TIMESTAMP);";

        rc = sqlite3_exec(db, insertDataSQL, 0, 0, &errMsg);

        if (rc != SQLITE_OK) {
            std::cerr << "Erreur lors de l'insertion de données : " << errMsg << std::endl;
            sqlite3_free(errMsg);
        } else {
            std::cout << "Données insérées avec succès." << std::endl;
        }

        // Fermeture de la base de données
        sqlite3_close(db);
    }
};
