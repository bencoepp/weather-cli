#include "WeatherHandler.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <future>
#include "barkeep.h"

/**
 * @brief Constructs a WeatherHandler object and initializes the necessary resources.
 *
 * This constructor initializes the WeatherHandler class by setting the provided
 * `path` and `options` values. It also prepares the SQLiteHandler by cleaning
 * the existing database and reinitializing it. This ensures the database is in
 * a consistent state for further operations.
 *
 * @param path A string representing the path to the data source.
 * @param options A LoadOptions struct that defines parameters such as limit,
 * batch size, and flags for async or batch processing.
 */
WeatherHandler::WeatherHandler(std::string path, LoadOptions options) : db("weather.db") {
    this->path = std::move(path);
    this->options = &options;
    db.cleanDatabase();
    db.init();
}

/**
 * @brief Loads and processes weather data by coordinating file handling and batch loading.
 *
 * This method performs the following operations:
 * - Retrieves a list of files from the specified path using the `loadFiles` method.
 * - Passes the retrieved files along with the given mutex to the `loadBatch` method for processing.
 *
 * @param mutex A reference to a `std::mutex` object used to synchronize access during
 * batch loading to ensure thread-safety.
 */
void WeatherHandler::load(std::mutex& mutex) {
    std::vector<std::filesystem::directory_entry> files = loadFiles();
    loadBatch(mutex, files);
}

/**
 * @brief Processes a batch of files and extracts measurements and station data.
 *
 * This method iterates over the provided files, isolates CSV files, and reads
 * each file line by line to parse measurement and station data. If a station
 * is newly encountered, it is added to the station list. Progress is displayed
 * using progress bars, and measurements and station data are saved to the
 * appropriate storage using thread-safe mechanisms.
 *
 * @param mutex A reference to a std::mutex used for thread synchronization
 * when saving data or updating shared resources.
 * @param files A vector of std::filesystem::directory_entry objects
 * representing files to be processed.
 */
void WeatherHandler::loadBatch(std::mutex &mutex, std::vector<std::filesystem::directory_entry> files) {
    for (const auto& entry: files) {
        if (entry.is_regular_file() && entry.path().extension() != ".csv") {
            continue;
        }

        std::ifstream file(entry.path().string());
        if (!file.is_open()) {
            continue;
        }

        std::vector<Measurement> measurements;
        std::vector<Station> stations;

        std::string line;

        while (std::getline(file, line)) {
            if (line.empty()) {
                continue;
            }

            if (line.find("STATION") != std::string::npos) {
                continue;
            }

            measurements.push_back(Measurement::fromCsv(line));

            Station station = Station::fromCsv(line);
            if (std::find(this->stations.begin(), this->stations.end(), station.id) == this->stations.end()) {
                this->stations.push_back(station.id);
                stations.push_back(station);
            }
        }

        file.close();
        this->workStations = 0;
        this->workMeasurements = 0;
        std::cout << "\x1B[2J\x1B[H";

        auto bars = generateBars(files.size(), measurements.size(), stations.size(), this->batchCount);

        if (!this->options->async) {
            bars->show();
        }

        save(measurements, mutex);
        save(stations, mutex);
        this->workFiles++;

        bars->done();
    }

    std::lock_guard lock(mutex);
    this->workBatches++;
}

/**
 * @brief Processes files in batches and loads them using the provided mutex for synchronization.
 *
 * This method splits the files loaded from the source into smaller batches based on the
 * batch size specified in the `options` parameter. Each batch is then processed sequentially
 * by calling an overloaded `loadBatch` method that handles batch-specific logic. The mutex
 * ensures thread-safe execution while processing batches.
 *
 * @param mutex A reference to a `std::mutex` used for synchronizing access to shared resources
 * during batch processing.
 */
void WeatherHandler::loadBatch(std::mutex &mutex) {
    std::vector<std::filesystem::directory_entry> files = loadFiles();
    this->batchCount = std::ceil(files.size() / this->options->batchSize);
    for (size_t start = 0; start < files.size(); start += this->options->batchSize) {
        size_t end = std::min(start + this->options->batchSize, files.size());
        std::vector batches(files.begin() + start, files.begin() + end);
        loadBatch(mutex, batches);
        this->workFiles = 0;
    }
}

/**
 * @brief Asynchronously loads weather data in parallel batches.
 *
 * The `loadAsync` function divides the available files into batches based on the specified
 * batch size in the `LoadOptions` struct. Each batch is processed in an asynchronous task
 * to improve performance by leveraging concurrency. Progress is visually displayed using
 * a progress bar.
 *
 * After all batch operations complete, the function ensures proper synchronization between
 * threads for data integrity. Finally, the progress bar indicates the completion of the
 * loading process.
 *
 * @param mutex A reference to a mutex used for thread synchronization during batch processing.
 */
void WeatherHandler::loadAsync(std::mutex& mutex) {
    std::vector<std::filesystem::directory_entry> files = loadFiles();
    std::vector<std::future<void>> futures;
    this->batchCount = std::ceil(files.size() / this->options->batchSize);

    auto bars = generateBars(files.size(), 0, 0, this->batchCount);
    bars->show();

    for (size_t start = 0; start < files.size(); start += this->options->batchSize) {
        size_t end = std::min(start + this->options->batchSize, files.size());
        std::vector batches(files.begin() + start, files.begin() + end);

        futures.push_back(std::async(std::launch::async, [this, &mutex, batches]() {
            loadBatch(mutex, batches);
        }));
    }
    for (auto& future : futures) {
        future.get();
    }

    bars->done();
}

/**
 * @brief Destroys the WeatherHandler object and releases any allocated resources.
 *
 * This destructor ensures proper cleanup of the WeatherHandler object by
 * deallocating any internal resources, including the database handler or
 * other dynamically allocated components, if necessary.
 */
WeatherHandler::~WeatherHandler() = default;

/**
 * @brief Loads and filters a list of files from the specified directory path.
 *
 * This function iterates through the directory specified by the `path` member, filtering
 * files based on certain criteria, such as being a regular file with a `.csv` extension.
 * It limits the number of files added to the result based on the `limit` value specified
 * in the `options` member. This ensures only a restricted number of relevant files are loaded.
 *
 * @return A vector of `std::filesystem::directory_entry` objects representing the filtered files.
 */
std::vector<std::filesystem::directory_entry> WeatherHandler::loadFiles() const {
    int count = 0;
    std::vector<std::filesystem::directory_entry> files;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (count >= options->limit) {
            break;
        }
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            files.push_back(entry);
            count++;
        }
    }

    return files;
}

/**
 * @brief Saves a collection of measurements to the database in a thread-safe manner.
 *
 * This function ensures thread-safety by acquiring a lock on the provided mutex before
 * inserting the given vector of `Measurement` objects into the database. It also increments
 * the `workMeasurements` counter to track successful operations.
 *
 * @param measurements A reference to a vector containing `Measurement` objects to be saved.
 * @param mutex A reference to a mutex used to ensure exclusive access to shared resources.
 */
void WeatherHandler::save(std::vector<Measurement> &measurements, std::mutex &mutex) {
    std::lock_guard lock(mutex);
    this->db.insertMeasurements(measurements);
    this->workMeasurements++;

}

/**
 * @brief Saves a collection of station records to the database in a thread-safe manner.
 *
 * This method locks the provided mutex to ensure thread-safe access to shared resources
 * and calls the database handler to insert the provided station data. It also updates
 * the internal counter for the number of stations processed.
 *
 * @param stations A reference to a vector of Station objects to be saved to the database.
 * @param mutex A reference to a std::mutex object used to ensure thread safety during data insertion.
 */
void WeatherHandler::save(std::vector<Station> &stations, std::mutex &mutex) {
    std::lock_guard lock(mutex);
    this->db.insertStations(stations);
    this->workStations += stations.size();
}

/**
 * @brief Generates a composite display with progress bars for different stages of the process.
 *
 * Depending on the configuration, this method returns a set of progress bars
 * for various tasks such as loading files, saving measurements, and saving stations.
 * It can also include additional progress bars for batch processing if certain
 * options are enabled.
 *
 * @param files The total number of files to be processed.
 * @param measurements The total number of measurement entities to be saved.
 * @param stations The total number of station entities to be saved.
 * @param batches The total number of batches to be processed.
 * @return A shared pointer to a `barkeep::CompositeDisplay` containing the relevant progress bars.
 */
std::shared_ptr<barkeep::CompositeDisplay> WeatherHandler::generateBars(int files, int measurements, int stations, int batches) {
    if (this->options->async) {
        return barkeep::Composite(
                {barkeep::ProgressBar(&this->workBatches, {
                .total = batches,
                .message = "Batches",
                .speed = 1,
                .speed_unit = "batch/s",
                .style = barkeep::Rich,
                .show = false,
                }),
              barkeep::ProgressBar(&this->workFiles, {
                    .total = files,
                    .message = "Load files",
                    .speed = 1,
                  .speed_unit = "files/s",
                    .style = barkeep::Rich,
                    .show = false,
                }),},"\n");
    }else if (this->options->batch) {
        return barkeep::Composite(
        {barkeep::ProgressBar(&this->workBatches, {
        .total = batches,
        .message = "Batches",
        .speed = 1,
        .speed_unit = "batch/s",
        .style = barkeep::Rich,
        .show = false,
        }),
      barkeep::ProgressBar(&this->workFiles, {
            .total = files,
            .message = "Load files",
            .speed = 1,
          .speed_unit = "files/s",
            .style = barkeep::Rich,
            .show = false,
        }),
        barkeep::ProgressBar(&this->workMeasurements, {
            .total = measurements,
            .message = "Save measurements",
            .speed = 1,
            .speed_unit = "entities/s",
            .style = barkeep::Rich,
            .show = false,
        }),
        barkeep::ProgressBar(&this->workStations, {
            .total =stations,
            .message = "Save stations",
            .speed = 1,
            .speed_unit = "entities/s",
            .style = barkeep::Rich,
            .show = false,
        })},"\n");
    }else {
        return barkeep::Composite(
      {barkeep::ProgressBar(&this->workFiles, {
            .total = files,
            .message = "Load files",
            .speed = 1,
          .speed_unit = "files/s",
            .style = barkeep::Rich,
            .show = false,
        }),
        barkeep::ProgressBar(&this->workMeasurements, {
            .total = measurements,
            .message = "Save measurements",
            .speed = 1,
            .speed_unit = "entities/s",
            .style = barkeep::Rich,
            .show = false,
        }),
        barkeep::ProgressBar(&this->workStations, {
            .total =stations,
            .message = "Save stations",
            .speed = 1,
            .speed_unit = "entities/s",
            .style = barkeep::Rich,
            .show = false,
        })},"\n");
    }
}
