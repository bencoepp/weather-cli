#ifndef STATION_H
#define STATION_H
#include <string>


/**
 * @class Station
 * @brief Represents a geographical station with associated metadata.
 *
 * This class stores information about a station including its unique identification,
 * geographical coordinates (latitude, longitude), elevation, and other attributes like name
 * and call sign. It also provides functionality to create a Station object from a CSV formatted string.
 */
class Station {
public:
    std::string id;
    std::string name;
    double longitude;
    double latitude;
    double elevation;
    std::string callSign;

    static Station fromCsv(std::string line);
};



#endif //STATION_H
