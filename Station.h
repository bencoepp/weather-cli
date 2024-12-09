//
// Created by benco on 09/12/2024.
//

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
};



#endif //STATION_H
