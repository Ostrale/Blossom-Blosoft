#include "DatabaseManager.cpp"

#define DEBUG

#ifdef DEBUG
#include <Windows.h> // Include the Windows.h header file
#endif

int main() {
#ifdef DEBUG
    SetConsoleOutputCP(CP_UTF8);
#endif
    // Appeler les fonctions de gestion de la base de données
    DatabaseManager::createTableAndViews();
    DatabaseManager::insertTestData();

    // Le reste de ton code principal ici...

    return 0;
}
