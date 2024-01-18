import sqlite3

def get_total_consumption_by_category(db_name):
    conn = sqlite3.connect(db_name)
    cursor = conn.cursor()

    cursor.execute("SELECT category, SUM(data_quantity) FROM categories "
                   "JOIN data_entries ON categories.id = data_entries.category_id "
                   "GROUP BY category;")

    results = cursor.fetchall()

    conn.close()

    return results

def main():
    db_name = "example.db"  # Remplace avec le nom de ta base de données

    total_consumption_by_category = get_total_consumption_by_category(db_name)

    print("Consommation totale par catégorie:")
    for category, total_consumption in total_consumption_by_category:
        print(f"{category}: {total_consumption} Mo")

if __name__ == "__main__":
    main()
