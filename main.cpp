#include <iostream>
#include <filesystem>

bool checkFilesAreAvailable(std::string path) {

    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            return true;
        }
    }

    return false;
}

int main() {
    //sind daten da
    if (checkFilesAreAvailable(R"(F:\project\weather-cli\data)")) {

    }
        //for schleife
        //jede neue zeile einlesen
        //zwei listen (stations, measurements)
            //stimmen die daten
    return 0;
}
