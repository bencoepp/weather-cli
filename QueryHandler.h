#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H
#include <string>

#include "SQLiteHandler.h"


class QueryHandler {
public:
    QueryHandler(std::string query, std::string sortType, std::string sortParameter, bool statusBar);
    void execute();
private:
    SQLiteHandler db;
    std::string query;
    std::string sortType;
    std::string sortParameter;
    std::vector<std::map<std::string, auto>> values;
    bool statusBar;
    void generateStatusBar();
    void generateTable();
    void sortData();
    void queryData();

    std::chrono::system_clock::time_point startTimer;
    std::chrono::system_clock::time_point endTimer;
};



#endif //QUERYHANDLER_H
