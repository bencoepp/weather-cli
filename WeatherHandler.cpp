#include "WeatherHandler.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <future>

#include "barkeep.h"

WeatherHandler::WeatherHandler(std::string path, LoadOptions options) : db("weather.db") {
    this->path = std::move(path);
    this->options = &options;
    db.cleanDatabase();
    db.init();
}

void WeatherHandler::load(std::mutex& mutex) {
    std::vector<std::filesystem::directory_entry> files = loadFiles();
    loadBatch(mutex, files);
}

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

WeatherHandler::~WeatherHandler() = default;

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

void WeatherHandler::save(const std::vector<Measurement>& measurements, std::mutex& mutex) {
    for (auto measurement: measurements) {
        std::lock_guard lock(mutex);
        this->db.insertMeasurement(measurement);
        this->workMeasurements++;
    }
}

void WeatherHandler::save(const std::vector<Station>& stations, std::mutex& mutex) {
    for (auto station: stations) {
        std::lock_guard lock(mutex);
        this->db.insertStation(station);
        this->workStations++;
    }
}

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
