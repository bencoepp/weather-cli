#ifndef STATION_H
#define STATION_H
#include <string>


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
