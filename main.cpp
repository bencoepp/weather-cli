#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <future>

#include "Measurement.h"
#include "Station.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <windows.h>
#include "barkeep.h"
#include "SQLiteHandler.h"

struct Command {
    std::string description;
    std::vector<std::string> arguments;
    std::vector<std::string> options;
};

void helpCommand(const std::map<std::string, Command>& commands) {
    std::cout << "Available Commands:" << std::endl << std::endl;

    for (const auto& [command, details] : commands) {
        std::cout << std::setw(15) << std::left << command
                  << "- " << details.description << std::endl;
        if (!details.arguments.empty()) {
            std::cout << "  Arguments:" << std::endl;
            for (const auto& arg : details.arguments) {
                std::cout << "    - " << arg << std::endl;
            }
        }
        if (!details.options.empty()) {
            std::cout << "  Options:" << std::endl;
            for (const auto& opt : details.options) {
                std::cout << "    - " << opt << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

void loadDataAsync(const std::vector<std::filesystem::directory_entry>& batch,
                  std::vector<Measurement>& measurements,
                  std::map<std::string, Station>& stations,
                  std::mutex& mtx,
                  int& work) {
    try {
        for (const auto& entry : batch) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                std::ifstream file(entry.path().string());
                if (!file.is_open()) {
                    std::cerr << "Could not open file: " << entry.path().string() << std::endl;
                    continue;
                }

                std::string line;
                while (std::getline(file, line)) {
                    if (line.empty()) {
                        continue;
                    }

                    if (line.find("STATION") != std::string::npos) {
                        continue;
                    }
                    Measurement measurement = Measurement::fromCsv(line);
                    Station station = Station::fromCsv(line);

                    std::lock_guard<std::mutex> lock(mtx);
                    measurements.push_back(measurement);
                    if (!stations.contains(station.id)) {
                        stations.insert({station.id, station});
                    }
                }

                file.close();

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    work++;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void loadCommand(const std::vector<std::string>& options) {
    bool drop = false;
    bool async = false;
    bool batch = false;
    bool clean = false;
    bool garbage = false;
    int limit = 0;
    int batchSize = 100;
    std::string path;

    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i] == "--drop") {
            drop = true;
        } else if (options[i] == "--async") {
            async = true;
        } else if (options[i] == "--batch") {
            batch = true;
        } else if (options[i] == "--limit") {
            if (i + 1 < options.size()) {
                limit = std::stoi(options[i + 1]);
                ++i;
            } else {
                std::cerr << "Error: --limit option requires a value." << std::endl;
                return;
            }
        } else if (options[i] == "--batch-size") {
            if (i + 1 < options.size()) {
                batchSize = std::stoi(options[i + 1]);
                ++i;
            } else {
                std::cerr << "Error: --batch-size option requires a value." << std::endl;
                return;
            }
        } else if (options[i] == "--clean") {
            clean = true;
        } else if (options[i] == "--garbage") {
            garbage = true;
        } else if (options[i] == "--path") {
            // Ensure a value is provided after "--path"
            if (i + 1 < options.size()) {
                path = options[i + 1];
                ++i;
            } else {
                std::cerr << "Error: --path option requires a value." << std::endl;
                return;
            }
        } else {
            std::cerr << "Warning: Unknown option '" << options[i] << "' ignored." << std::endl;
        }
    }

    if (path.empty()) {
        std::cerr << "Error: --path option is required." << std::endl;
    }

    std::cout << "Loading data from " << path << std::endl;
    std::vector<Measurement> measurements;
    std::map<std::string, Station> stations;
    std::mutex mtx;

    SQLiteHandler db("weather.db");
    db.cleanDatabase();
    db.init();

    int workLoading{0};
    int workStations{0};
    int workMeasurements{0};
    std::vector<std::filesystem::directory_entry> files;
    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (count >= limit) {
            break;
        }
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            files.push_back(entry);
            count++;
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    if (async && batch) {
        std::cerr << "Error: --async and --batch options are mutually exclusive." << std::endl;
    }else {
        if (async) {
            std::vector<std::future<void>> futures;
            auto bar = barkeep::ProgressBar(&workLoading, {
                    .total = static_cast<int>(files.size()),
                    .message = "Loading data async...",
                    .speed = 1.0,
                    .speed_unit = "measurements/s",
                    .style = barkeep::Rich,
                });
            for (size_t start = 0; start < files.size(); start += batchSize) {
                size_t end = std::min(start + batchSize, files.size());
                std::vector<std::filesystem::directory_entry> batches(files.begin() + start, files.begin() + end);

                futures.push_back(std::async(std::launch::async, loadDataAsync, std::move(batches), std::ref(measurements), std::ref(stations), std::ref(mtx), std::ref(workLoading)));
            }

            for (auto& future : futures) {
                future.get();
            }
            bar->done();
        } else if (batch) {
            for (size_t start = 0; start < files.size(); start += batchSize) {
                size_t end = std::min(start + batchSize, files.size());
                std::vector<std::filesystem::directory_entry> batches(files.begin() + start, files.begin() + end);
                std::vector<Measurement> batchMeasurements;
                std::map<std::string, Station> batchStations;
                loadDataAsync(batches, batchMeasurements, batchStations, mtx, workLoading);

                std::cout << "\x1B[2J\x1B[H";

                auto bars = barkeep::Composite(
                {barkeep::ProgressBar(&workLoading, {
                      .total = static_cast<int>(files.size()),
                      .message = "Loading data",
                      .speed = 1,
                      .speed_unit = "files/s",
                      .style = barkeep::Rich,
                      .show = false,
                }),
                barkeep::ProgressBar(&workMeasurements, {
                    .total = static_cast<int>(batchMeasurements.size()),
                    .message = "Saving measurements",
                    .speed = 1,
                    .speed_unit = "entities/s",
                    .style = barkeep::Rich,
                    .show = false,
                }),
                barkeep::ProgressBar(&workStations, {
                    .total = static_cast<int>(batchStations.size()),
                    .message = "Saving stations",
                    .speed = 1,
                    .speed_unit = "entities/s",
                    .style = barkeep::Rich,
                    .show = false,
                }),},
              "\n");
                bars->show();

                for (auto& measurement : batchMeasurements) {
                    db.insertMeasurement(measurement);
                    workMeasurements++;
                }

                for (auto& [id, station] : batchStations) {
                    db.insertStation(station);
                    workStations++;
                }
                bars->done();
                workMeasurements = 0;
                workStations = 0;
            }
        }else {
            auto bar = barkeep::ProgressBar(&workLoading, {
              .total = static_cast<int>(files.size()),
              .message = "Loading data",
              .speed = 1.,
              .speed_unit = "files/s",
              .style = barkeep::ProgressBarStyle::Rich,
            });
            loadDataAsync(files, measurements, stations, mtx, workLoading);
            bar->done();
            auto bars = barkeep::Composite(
              {barkeep::ProgressBar(&workMeasurements, {
                    .total = static_cast<int>(measurements.size()),
                    .message = "Saving measurements",
                    .speed = 1,
                    .speed_unit = "entities/s",
                    .style = barkeep::Rich,
                    .show = false,
                }),
                barkeep::ProgressBar(&workStations, {
                    .total = static_cast<int>(stations.size()),
                    .message = "Saving stations",
                    .speed = 1,
                    .speed_unit = "entities/s",
                    .style = barkeep::Rich,
                    .show = false,
                })},
              "\n");
            bars->show();
            for (auto& measurement : measurements) {
                db.insertMeasurement(measurement);
                workMeasurements++;
            }
            for (auto& [id, station] : stations) {
                db.insertStation(station);
                workStations++;
            }
            bars->done();
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << "Loaded " << measurements.size() << " measurements in " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" << std::endl;

    std::cout << "Saved " << db.countStations() << " stations" << std::endl;
    std::cout << "Saved " << db.countMeasurements() << " measurements" << std::endl;
}

void queryCommand(const std::vector<std::string>& options) {

}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(CP_UTF8);

    std::map<std::string, Command> commands = {
        {"load", {"Load data from directory", {}, {"-d (drop)", "-a (async)", "-c (clean)", "-b (batch)", "-g (garbage)" , "-p (path)", "-bs (batch-size)"}}},
        {"query", {"Allows the user to query the weather data", {}, {}}},
        {"help", {"Displays the help information", {}, {}}}
    };

    if (argc < 2) {
        helpCommand(commands);
        return 0;
    }

    std::string command = argv[1];
    std::vector<std::string> options;

    for (int i = 2; i < argc; ++i) {
        options.emplace_back(argv[i]);
    }

    if (command == "help") {
        helpCommand(commands);
    } else if (command == "load") {
        loadCommand(options);
    } else if (command == "query") {
        queryCommand(options);
    }

    return 0;
}
