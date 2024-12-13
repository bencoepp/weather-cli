#include "Station.h"
#include <iostream>
#include <sstream>
#include <vector>

/**
 * Parses a CSV line and constructs a Station object using the extracted data.
 * The method assumes the CSV line is structured with specific fields and uses
 * their positions to populate the attributes of a Station object.
 *
 * @param line A string representing a single row of CSV data. Each field is separated
 *             by a comma, and fields potentially containing commas are enclosed in quotes.
 * @return A Station object constructed using the data extracted from the CSV line.
 *         In case of parsing issues, an empty Station object may be returned.
 */
Station Station::fromCsv(std::string line) {
    Station station = {};

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
