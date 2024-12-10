#include "Measurement.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

Measurement Measurement::fromCsv(std::string line) {
    Measurement measurement = {};

    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '"') {
            // Toggle the inQuotes flag
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            // Split at comma if not inside quotes
            tokens.push_back(token);
            token.clear();
        } else {
            // Append character to the current token
            token += c;
        }
    }

    // Add the last token
    tokens.push_back(token);

    try {
        //measurement.id = ;
        measurement.station = tokens[0];
        measurement.date = tokens[1];
        measurement.reportType = tokens[7];
        measurement.qualityControlFlag = tokens[9];
        measurement.wind = tokens[10];
        measurement.cloudCeiling = std::stod(tokens[11]);
        measurement.visibilityDistance = std::stod(tokens[12]);
        measurement.temperature = std::stod(tokens[13]);
        measurement.dewPoints = std::stod(tokens[14]);
        measurement.seaLevelPressure = std::stod(tokens[15]);
    }catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return measurement;
}
