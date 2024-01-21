#include <gtest/gtest.h>
#include "DatabaseManager.hpp"

// Test fixture for DatabaseManager
class DatabaseManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary in-memory database for testing
        int rc = sqlite3_open(":memory:", &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            exit(1);
        }
        manager = new DatabaseManager(db);
    }

    void TearDown() override {
        delete manager;
        sqlite3_close(db);
    }

    sqlite3* db;
    DatabaseManager* manager;
};

// Test case for inserting a category
TEST_F(DatabaseManagerTest, InsertCategory) {
    std::string category = "Test Category";
    int categoryId = manager->insertCategory(category);

    // Check if the category was inserted successfully
    ASSERT_GT(categoryId, 0);

    // Check if the category exists in the database
    int retrievedCategoryId = manager->getCategoryId(category);
    ASSERT_EQ(categoryId, retrievedCategoryId);
}

// Test case for inserting data entries
TEST_F(DatabaseManagerTest, InsertDataEntry) {
    std::string category = "Test Category";
    int categoryId = manager->insertCategory(category);

    std::vector<DataEntry> entries = {
        { 1, 10 },
        { 2, 20 },
        { 3, 30 }
    };

    manager->insertDataEntry(categoryId, entries);

    // TODO: Implement assertions to check if the data entries were inserted correctly
}

// Test case for retrieving a category ID
TEST_F(DatabaseManagerTest, GetCategoryId) {
    std::string category = "Test Category";
    int categoryId = manager->insertCategory(category);

    // Check if the retrieved category ID matches the inserted category ID
    int retrievedCategoryId = manager->getCategoryId(category);
    ASSERT_EQ(categoryId, retrievedCategoryId);
}

// TODO: Add more test cases to cover other functionalities of the DatabaseManager class
