#include "WeatherHandler.h"

#include <fstream>
#include <iostream>
#include <algorithm>

#include "barkeep.h"

WeatherHandler::WeatherHandler(std::string path, LoadOptions options) : db("weather.db") {
    this->path = std::move(path);
    this->options = &options;
    db.cleanDatabase();
    db.init();
}

void WeatherHandler::load(std::mutex& mutex) {
    std::vector<std::filesystem::directory_entry> files = loadFiles();

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

            Station station = Station::fromCsv(line);

            measurements.push_back(Measurement::fromCsv(line));
            if (std::find(this->stations.begin(), this->stations.end(), station.id) == this->stations.end()) {
                this->stations.push_back(station.id);
                stations.push_back(station);
            }
        }

        file.close();
        this->workStations = 0;
        this->workMeasurements = 0;
        std::cout << "\x1B[2J\x1B[H";

        auto bars = barkeep::Composite(
      {barkeep::ProgressBar(&this->workFiles, {
            .total = static_cast<int>(files.size()),
            .message = "Load files",
            .speed = 1,
            .style = barkeep::Rich,
            .show = false,
        }),
        barkeep::ProgressBar(&this->workMeasurements, {
            .total = static_cast<int>(measurements.size()),
            .message = "Save measurements",
            .speed = 1,
            .style = barkeep::Rich,
            .show = false,
        }),
        barkeep::ProgressBar(&this->workStations, {
            .total = static_cast<int>(stations.size()),
            .message = "Save stations",
            .speed = 1,
            .style = barkeep::Rich,
            .show = false,
        })},
      "\n");
        bars->show();

        save(measurements, mutex);
        save(stations, mutex);
        this->workFiles++;

        bars->done();
    }
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
