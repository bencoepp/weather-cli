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

    const int maxSize = values.size() > 50 ? 50 : values.size();

    for (int i = 0; i < maxSize ; ++i) {
         std::vector<std::string> row;
        table.add_row(tabulate::RowStream{} << row);
    }


    table.add_row(tabulate::Table::Row_t{"S/N", "Movie Name", "Director", "Estimated Budget", "Release Date"});
    table.add_row(tabulate::Table::Row_t{"tt1979376", "Toy Story 4", "Josh Cooley", "$200,000,000", "21 June 2019"});
    table.add_row(tabulate::Table::Row_t{"tt3263904", "Sully", "Clint Eastwood", "$60,000,000", "9 September 2016"});
    table.add_row(
        {"tt1535109", "Captain Phillips", "Paul Greengrass", "$55,000,000", " 11 October 2013"});

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
