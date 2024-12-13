#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H
#include <string>

#include "SQLiteHandler.h"


/**
 * @class QueryHandler
 * @brief Handles the execution of queries, sorting, and displaying tabulated results.
 *
 * This class provides functionality for processing database queries, sorting the resulting data,
 * generating tables for display, and optionally displaying a status bar with performance metrics.
 */
class QueryHandler {
public:
    QueryHandler(std::string query, std::string sortType, std::string sortParameter, bool statusBar);
    void execute();
private:
    SQLiteHandler db;
    std::string query;
    std::string sortType;
    std::string sortParameter;
    std::vector<std::map<std::string, std::string>> values;
    bool statusBar;
    void generateStatusBar() const;
    void generateTable();
    void sortData();
    void queryData();

    std::chrono::system_clock::time_point startTimer;
    std::chrono::system_clock::time_point endTimer;
};



#endif //QUERYHANDLER_H
