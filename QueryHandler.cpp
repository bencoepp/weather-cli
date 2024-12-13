#include "QueryHandler.h"

#include <iostream>
#include <utility>

QueryHandler::QueryHandler(std::string query, std::string sortType, const bool statusBar) : db("weather.db"), query(std::move(query)), sortType(std::move(sortType)), statusBar(statusBar) {
}

void QueryHandler::execute() {
    this->startTimer = std::chrono::high_resolution_clock::now();
    queryData();
    sortData();
    generateTable();
    this->endTimer = std::chrono::high_resolution_clock::now();
    generateStatusBar();
}

void QueryHandler::generateStatusBar() {
    if (this->statusBar) {
        std::cout << "\n============================================" << std::endl;
        std::cout << "| Status Bar" << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << "| Total Values : " << values << std::endl;
        std::cout << "| Query        : " << query << std::endl;
        std::cout << "| Min Value    : " << 0 << std::endl;
        std::cout << "| Max Value    : " << 100 << std::endl;
        std::cout << "| Median Value : " << 50 << std::endl;
        std::cout << "| Performance  : " << std::endl;
        std::cout << "|   Time       : " << std::chrono::duration<double, std::milli>(startTimer - endTimer).count() << " ms" << std::endl;
        std::cout << "|   Memory     : " << 100 << " KB" << std::endl;
        std::cout << "|   CPU Usage  : " << 2000 << " %" << std::endl;
        std::cout << "============================================" << std::endl;
    }
}

void QueryHandler::generateTable() {
}

void QueryHandler::sortData() {
    if (this->sortType == "bubble") {

    }else if (this->sortType == "quick") {

    }
}

void QueryHandler::queryData() {
    if (this->query.empty()) {
        std::cerr << "Query is empty" << std::endl;
    }

    this->values = db.executeQuery(this->query);
}
