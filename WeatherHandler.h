#ifndef WEATHERHANDLER_H
#define WEATHERHANDLER_H
#include "barkeep.h"
#include "SQLiteHandler.h"

struct LoadOptions {
    int limit;
    int batchSize;
    bool async;
    bool batch;
};

class WeatherHandler {
public:
    WeatherHandler(std::string path, LoadOptions options);
    void load(std::mutex& mutex);
    void loadBatch(std::mutex& mutex, std::vector<std::filesystem::directory_entry> files);
    void loadBatch(std::mutex& mutex);
    void loadAsync(std::mutex& mutex);
    ~WeatherHandler();
private:
    LoadOptions* options;
    SQLiteHandler db;
    std::string path;
    std::vector<std::string> stations;
    int batchCount = 0;
    int workFiles = 0;
    int workMeasurements = 0;
    int workStations = 0;
    int workBatches = 0;
    std::vector<std::filesystem::directory_entry> loadFiles() const;
    void save(std::vector<Measurement> &measurements, std::mutex &mutex);
    void save(std::vector<Station> &stations, std::mutex &mutex);
    std::shared_ptr<barkeep::CompositeDisplay> generateBars(int files, int measurements, int stations, int batches);
};



#endif //WEATHER HANDLER_H
