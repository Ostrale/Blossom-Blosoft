const sqlite3 = require('sqlite3').verbose();
const dbPath = '../BlosoftDB.db';

function getCategories() {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT * FROM "categories"', (err, rows) => {
            if (err) {
                console.error(err.message);
                reject(err);
            } else {
                const categories = rows.map(row => {
                    return {
                        category_id: row.id,
                        categories: row.category
                    };
                });
                if (categories.length === 0) {
                    console.log("No categories found");
                    resolve(null);
                } else {
                    resolve(categories);
                }
            }
            db.close();
        });
    });
}

function getCategoryTotals() {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath,sqlite3.OPEN_READONLY);
        db.all('SELECT categories.category AS categorie, SUM(data_entries.data_quantity) AS total_quantity FROM data_entries JOIN categories ON data_entries.category_id = categories.id GROUP BY data_entries.category_id', (err, rows) => {
            if (err) {
                console.error(err.message);
                reject(err);
            } else {
                resolve(rows);
            }
            db.close();
        });
    });
}

function getCategoryTotalsBetweenTimestamps(startTimestamp, endTimestamp) {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT categories.category AS categorie, SUM(data_entries.data_quantity) AS total_quantity FROM data_entries JOIN categories ON data_entries.category_id = categories.id WHERE data_entries.timestamp >= ? AND data_entries.timestamp <= ? GROUP BY data_entries.category_id', [startTimestamp, endTimestamp], (err, rows) => {
            if (err) {
                console.error(err.message);
                reject(err);
            } else {
                resolve(rows);
            }
            db.close();
        });
    });
}

// Fonction pour convertir une date en timestamp
async function dateToTimestamp(date) {
    return Math.floor(date.getTime() / 1000);
}

// Fonction pour convertir un timestamp en date
async function timestampToDate(timestamp) {
    return new Date(timestamp * 1000);
}

module.exports = {
    getCategories,
    getCategoryTotals,
    getCategoryTotalsBetweenTimestamps,
};
