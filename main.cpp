#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include "Measurement.h"
#include "Station.h"
#include <SQLiteCpp/SQLiteCpp.h>

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
    std::vector<Station> stations;
    if (async && batch) {
        std::cerr << "Error: --async and --batch options are mutually exclusive." << std::endl;
    }else {
        if (async) {
            std::cout << "Loading data asynchronously..." << std::endl;
        } else if (batch) {
            std::cout << "Loading data in batches..." << std::endl;
        }else {
            std::cout << "Loading data synchronously..." << std::endl;
            int amount = 0;
            try {
                int files = 0;
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                        files++;
                        if (files >= limit) {
                            break;
                        }
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

                            amount++;
                            Measurement measurement = Measurement::fromCsv(line);
                            measurements.push_back(measurement  );
                            Station station = Station::fromCsv(line);
                        }

                        file.close();
                    }
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }

            std::cout << "Loaded " << amount << " measurements." << std::endl;
        }
    }
}

void queryCommand(const std::vector<std::string>& options) {

}

int main(int argc, char* argv[]) {
    std::map<std::string, Command> commands = {
        {"load", {"Load data from directory", {}, {"-d (drop)", "-a (async)", "-c (clean)", "-b (batch)", "-g (garbage)" , "-p (path)"}}},
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
