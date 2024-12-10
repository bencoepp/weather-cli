#include "Station.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

Station Station::fromCsv(std::string line) {
    Station station = {};

    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, ',')) {
        token.erase(std::ranges::remove(token, '"').begin(), token.end());
        tokens.push_back(token);
    }

    try {
        station.id = tokens[0];
        station.name = tokens[6];
        station.latitude = std::stod(tokens[3]);
        station.longitude = std::stod(tokens[4]);
        station.elevation = std::stod(tokens[5]);
        station.callSign = tokens[8];
    }catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return station;

}
