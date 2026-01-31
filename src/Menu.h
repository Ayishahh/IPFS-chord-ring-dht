/**
 * @file Menu.h
 * @brief Menu system for IPFS Ring DHT Simulator
 * @details Provides user interface functions with ASCII-compatible output
 * 
 * Compile: g++ -std=c++17 -Wall -o ipfs_dht src/main.cpp
 */

#pragma once
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

/**
 * @brief Clear screen (cross-platform)
 */
inline void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Print application header
 */
inline void printHeader() {
    cout << "\n";
    cout << "  +========================================================================+\n";
    cout << "  |                                                                        |\n";
    cout << "  |     IIIII  PPPP   FFFFF  SSSS        DDDD   H   H  TTTTT               |\n";
    cout << "  |       I    P   P  F      S           D   D  H   H    T                 |\n";
    cout << "  |       I    PPPP   FFF    SSSS  ===   D   D  HHHHH    T                 |\n";
    cout << "  |       I    P      F          S       D   D  H   H    T                 |\n";
    cout << "  |     IIIII  P      F      SSSS        DDDD   H   H    T                 |\n";
    cout << "  |                                                                        |\n";
    cout << "  |              INTERPLANETARY FILE SYSTEM - RING DHT SIMULATOR           |\n";
    cout << "  |                                                                        |\n";
    cout << "  +========================================================================+\n";
    cout << "\n";
}

/**
 * @brief Print setup header
 */
inline void printSetupHeader() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                         SYSTEM CONFIGURATION                           |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
}

/**
 * @brief Print main menu
 */
inline void printMainMenu() {
    cout << "\n";
    cout << "  +========================================================================+\n";
    cout << "  |                              MAIN MENU                                 |\n";
    cout << "  +========================================================================+\n";
    cout << "  |                                                                        |\n";
    cout << "  |    1.  Add New Machine                                                 |\n";
    cout << "  |    2.  Remove Machine                                                  |\n";
    cout << "  |                                                                        |\n";
    cout << "  |    3.  Insert File                                                     |\n";
    cout << "  |    4.  Search File                                                     |\n";
    cout << "  |    5.  Delete File                                                     |\n";
    cout << "  |                                                                        |\n";
    cout << "  |    6.  Print Routing Table                                             |\n";
    cout << "  |    7.  Print B-Tree                                                    |\n";
    cout << "  |    8.  Print All Routing Tables                                        |\n";
    cout << "  |    9.  Print All B-Trees                                               |\n";
    cout << "  |                                                                        |\n";
    cout << "  |   10.  View System Status                                              |\n";
    cout << "  |   11.  Restart System                                                  |\n";
    cout << "  |    0.  Exit                                                            |\n";
    cout << "  |                                                                        |\n";
    cout << "  +========================================================================+\n";
    cout << "\n";
}

/**
 * @brief Print a separator line
 */
inline void printSeparator() {
    cout << "\n  ------------------------------------------------------------------------\n\n";
}

/**
 * @brief Print success message
 */
inline void printSuccess(const string& message) {
    cout << "\n  [OK] SUCCESS: " << message << "\n";
}

/**
 * @brief Print error message
 */
inline void printError(const string& message) {
    cout << "\n  [X] ERROR: " << message << "\n";
}

/**
 * @brief Print info message
 */
inline void printInfo(const string& message) {
    cout << "\n  [i] INFO: " << message << "\n";
}

/**
 * @brief Wait for user to press Enter
 */
inline void waitForEnter() {
    cout << "\n  Press Enter to continue...";
    cin.ignore();
    cin.get();
}

/**
 * @brief Get integer input with validation
 */
inline int getIntInput(const string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << "  " << prompt;
        if (cin >> value) {
            if (value >= minVal && value <= maxVal) {
                return value;
            }
            cout << "  Please enter a value between " << minVal << " and " << maxVal << ".\n";
        } else {
            cout << "  Invalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

/**
 * @brief Get yes/no confirmation
 */
inline bool getConfirmation(const string& prompt) {
    char choice;
    cout << "  " << prompt << " (y/n): ";
    cin >> choice;
    return (choice == 'y' || choice == 'Y');
}
