#include "DatabaseManager.cpp"

#define DEBUG

#ifdef DEBUG
#include <Windows.h> // Include the Windows.h header file
#endif

int main() {
    DatabaseManager dbManager("example.db");

    // Example data insertion for 'games' category
    int categoryId = dbManager.insertCategory("games");

    if (categoryId != -1) {
        std::vector<DataEntry> gameEntries = {{1642531200, 1024}, {1642532400, 512}};
        dbManager.insertDataEntry(categoryId, gameEntries);
    }

    // Example data insertion for 'streaming' category
    int streamingCategoryId = dbManager.insertCategory("streaming");

    if (streamingCategoryId != -1) {
        std::vector<DataEntry> streamingEntries = {{1642533600, 768}, {1642534800, 256}};
        dbManager.insertDataEntry(streamingCategoryId, streamingEntries);
    }

    // Example data insertion for 'games' category again
    categoryId = dbManager.insertCategory("games");

    if (categoryId != -1) {
        std::vector<DataEntry> gameEntries = {{1642536000, 1024}, {1642537200, 512}};
        dbManager.insertDataEntry(categoryId, gameEntries);
    }
}