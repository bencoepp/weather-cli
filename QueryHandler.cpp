#include "QueryHandler.h"

#include <iostream>
#include <utility>

#include "tabulate.h"

QueryHandler::QueryHandler(std::string query, std::string sortType, std::string sortParameter, const bool statusBar) : db("weather.db"), query(std::move(query)), sortType(std::move(sortType)), sortParameter(std::move(sortParameter)), statusBar(statusBar) {
}

void QueryHandler::execute() {
    this->startTimer = std::chrono::high_resolution_clock::now();
    queryData();
    sortData();
    generateTable();
    this->endTimer = std::chrono::high_resolution_clock::now();
    generateStatusBar();
}

void QueryHandler::generateStatusBar() const {
    if (this->statusBar) {
        std::cout << "\n============================================" << std::endl;
        std::cout << "| Status Bar" << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << "| Total Values : " << values.size() << std::endl;
        std::cout << "| Query        : " << query << std::endl;
        std::cout << "| Min Value    : " << 0 << std::endl;
        std::cout << "| Max Value    : " << 100 << std::endl;
        std::cout << "| Median Value : " << 50 << std::endl;
        std::cout << "| Performance  : " << std::endl;
        std::cout << "|   Time       : " << std::chrono::duration<double, std::milli>(endTimer - startTimer).count() << " ms" << std::endl;
        std::cout << "|   Memory     : " << 100 << " KB" << std::endl;
        std::cout << "|   CPU Usage  : " << 2000 << " %" << std::endl;
        std::cout << "============================================" << std::endl;
    }
}

void QueryHandler::generateTable() {
    tabulate::Table table;
    bool firstRow = true;
    int maxValues = values.size() > 50 ? 50 : values.size();
    std::vector<variant<std::string, const char *, string_view, tabulate::Table>> header;
    for (int i = 0; i < maxValues; i++) {
        const std::map<std::string, std::string> &row = this->values[i];
        std::vector<variant<std::string, const char *, string_view, tabulate::Table>> rowValues;
        for (const auto &column : row) {
            if (firstRow) {
                header.push_back(column.first);
            }
            rowValues.push_back(column.second);
        }
        if (firstRow) {
            table.add_row(header);
        }
        table.add_row(rowValues);
        firstRow = false;
    }
    std::cout << table << std::endl;
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

    this->values = this->db.executeQuery(this->query);
}
