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
                ++i; // Skip the path value as it's already processed
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

    std::atomic<size_t> work{0};
    std::pmr::vector<std::filesystem::directory_entry> files;
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

        } else if (batch) {
            auto bar = barkeep::ProgressBar(&work, {
              .total = files.size(),
              .message = "Loading data in batches...",
              .speed = 1.,
              .speed_unit = "measurements/s",
              .style = barkeep::Rich,
            });
            try {
                for (size_t start = 0; start < count; start += batchSize) {
                    size_t end = std::min(start + batchSize, files.size());
                    for (size_t i = start; i < end; ++i) {
                        const auto& entry = files[i];
                        std::ifstream file(entry.path().string());
                        if (!file.is_open()) {
                            std::cerr << "Could not open file: " << entry.path().string() << std::endl;
                            continue;
                        }

                        std::string line;
                        while (std::getline(file, line)) {
                            if (line.empty() || line.find("STATION") != std::string::npos) {
                                continue;
                            }

                            measurements.push_back(Measurement::fromCsv(line));
                            Station station = Station::fromCsv(line);
                            if (!stations.contains(station.id)) {
                                stations[station.id] = station;
                            }
                        }

                        file.close();
                        work++;
                    }
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
            bar->done();
        }else {
            auto bar = barkeep::ProgressBar(&work, {
              .total = files.size(),
              .message = "Loading data synchronously...",
              .speed = 1.,
              .speed_unit = "measurements/s",
              .style = barkeep::Rich,
            });
            try {
                for (const auto& entry : files) {
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

                            measurements.push_back(Measurement::fromCsv(line));
                            Station station = Station::fromCsv(line);
                            if (!stations.contains(station.id)) {
                                stations[station.id] = station;
                            }
                        }

                        file.close();
                        work++;
                    }
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
            bar->done();


        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << "Loaded " << measurements.size() << " measurements in " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" << std::endl;
}

void queryCommand(const std::vector<std::string>& options) {

}

int
main(int argc, char* argv[]) {
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
