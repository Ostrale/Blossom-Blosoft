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
        db.all('SELECT c.category, SUM(de.data_quantity) AS total_data_quantity FROM Categories c JOIN Protocols p ON c.id = p.category_id JOIN data_entries de ON p.id = de.Protocol_id GROUP BY c.category;', (err, rows) => {
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

function getCategoryTotalsBetweenTimestamps(none = null, startTimestamp, endTimestamp) {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT c.category, SUM(de.data_quantity) AS total_data_quantity FROM Categories c JOIN Protocols p ON c.id = p.category_id JOIN data_entries de ON p.id = de.Protocol_id WHERE de.timestamp >= ? AND de.timestamp <= ? GROUP BY c.category;', [startTimestamp, endTimestamp], (err, rows) => {
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


function getBreeds() {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT * FROM "breeds"', (err, rows) => {
            if (err) {
                console.error(err.message);
                reject(err);
            } else {
                const breeds = rows.map(row => {
                    return {
                        breed_id: row.id,
                        breed: row.breed
                    };
                });
                if (breeds.length === 0) {
                    console.log("No breeds found");
                    resolve(null);
                } else {
                    resolve(breeds);
                }
            }
            db.close();
        });
    });
}

function getProtocols() {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT * FROM "protocols"', (err, rows) => {
            if (err) {
                console.error(err.message);
                reject(err);
            } else {
                const protocols = rows.map(row => {
                    return {
                        protocol_id: row.id,
                        protocol: row.protocol,
                        category_id: row.category_id
                    };
                });
                if (protocols.length === 0) {
                    console.log("No protocols found");
                    resolve(null);
                } else {
                    resolve(protocols);
                }
            }
            db.close();
        });
    });
}

function getProtocolsTotals() {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT p.protocol, SUM(de.data_quantity) AS total_data_quantity FROM Protocols p JOIN data_entries de ON p.id = de.Protocol_id GROUP BY p.protocol;', (err, rows) => {
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

function getProtocolsTotalsBetweenTimestamps(none = null, startTimestamp, endTimestamp) {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT p.protocol, SUM(de.data_quantity) AS total_data_quantity FROM Protocols p JOIN data_entries de ON p.id = de.Protocol_id WHERE de.timestamp >= ? AND de.timestamp <= ? GROUP BY p.protocol;', [startTimestamp, endTimestamp], (err, rows) => {
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

function getDataEntries() {}

function getTotalsBetweenTimestamps(none = null, startTimestamp = 0, endTimestamp = 4294967295) {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT SUM(de.data_quantity) AS total_data_quantity FROM data_entries de WHERE de.timestamp >= ? AND de.timestamp <= ?;', [startTimestamp, endTimestamp], (err, rows) => {
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



getOldestTimestamp = () => {
    return new Promise((resolve, reject) => {
        const db = new sqlite3.Database(dbPath, sqlite3.OPEN_READONLY);
        db.all('SELECT MIN(de.timestamp) AS oldest_timestamp FROM data_entries de;', (err, rows) => {
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
    getBreeds,
    getProtocols,
    getProtocolsTotals,
    getProtocolsTotalsBetweenTimestamps,
    getDataEntries,
    getTotalsBetweenTimestamps,
    getOldestTimestamp,
};