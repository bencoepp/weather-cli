#include "Measurement.h"
#include <iomanip>
#include <iostream>
#include <vector>

/**
 * Parses a CSV line and creates a Measurement object from the extracted data.
 *
 * This method processes a single line of a CSV formatted string, splitting it into tokens
 * while respecting quotes. The parsed tokens are then used to populate the fields of
 * a Measurement object.
 *
 * @param line The input string representing a single CSV line containing measurement data.
 *             Fields are expected to be comma-separated, and quotes are handled to preserve
 *             values containing commas.
 * @return A Measurement object populated with the data extracted from the input CSV line.
 *         If parsing fails or data is missing, the method may produce undefined or default values.
 */
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
