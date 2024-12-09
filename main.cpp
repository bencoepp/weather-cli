#include <iostream>
#include <filesystem>
#include <vector>


void helpCommand(const std::vector<std::string>& options){}

void loadCommand(const std::vector<std::string>& options) {
    bool drop = false;
    bool async = false;
    bool batch = false;
    bool clean = false;
    bool garbage = false;
    std::string path;

    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i] == "--drop") {
            drop = true;
        } else if (options[i] == "--async") {
            async = true;
        } else if (options[i] == "--batch") {
            batch = true;
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
}

void queryCommand(const std::vector<std::string>& options) {

}

int main(int argc, char* argv[]) {
    std::string command = argv[1];
    std::vector<std::string> options;

    for (int i = 2; i < argc; ++i) {
        options.emplace_back(argv[i]);
    }

    if (command == "help") {
        helpCommand(options);
    } else if (command == "load") {
        loadCommand(options);
    } else if (command == "query") {
        queryCommand(options);
    }else {
        helpCommand(options);
    }

    return 0;
}
