#ifndef WEATHERHANDLER_H
#define WEATHERHANDLER_H
#include "barkeep.h"
#include "SQLiteHandler.h"

/**
 * @struct LoadOptions
 * @brief Specifies configuration options for loading data processes.
 *
 * LoadOptions struct defines various parameters to control the behavior of data loading,
 * including limits on the number of files, batch sizes, and whether asynchronous or batch
 * operations should be performed.
 */
struct LoadOptions {
    int limit;
    int batchSize;
    bool async;
    bool batch;
};

/**
 * @class WeatherHandler
 * @brief Handles weather data processing, batch loading, and database operations.
 *
 * The WeatherHandler class is responsible for managing weather data, including loading files,
 * batch processing, and saving data into a SQLite database. Additionally, it provides asynchronous
 * and batch-loading functionality along with progress tracking.
 */
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
