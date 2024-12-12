#ifndef WEATHERHANDLER_H
#define WEATHERHANDLER_H
#include "SQLiteHandler.h"

struct LoadOptions {
    int limit;
    int batchSize;
};

class WeatherHandler {
public:
    WeatherHandler(std::string path, LoadOptions options);
    void load(std::mutex& mutex);
    void loadAsync();
    ~WeatherHandler();
private:
    LoadOptions* options;
    SQLiteHandler db;
    std::string path;
    std::vector<std::string> stations;
    int workFiles = 0;
    int workMeasurements = 0;
    int workStations = 0;
    std::vector<std::filesystem::directory_entry> loadFiles() const;
    void save(const std::vector<Measurement>& measurements, std::mutex& mutex);
    void save(const std::vector<Station>& stations, std::mutex& mutex);
};



#endif //WEATHER HANDLER_H
