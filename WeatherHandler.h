//
// Created by benco on 12/12/2024.
//

#ifndef WEATHERHANDLER_H
#define WEATHERHANDLER_H
#include "SQLiteHandler.h"


class WeatherHandler {
public:
    WeatherHandler();
    void load();
    void loadAsync();
private:
    SQLiteHandler db;
    std::vector<std::string> stations;
    int workFiles = 0;
    int workMeasurements = 0;
    int workStations = 0;
    std::vector<std::filesystem::directory_entry> loadFiles();
    void save(std::vector<Measurement> measurements);
    void save(std::vector<Station> stations);
};



#endif //WEATHERHANDLER_H
