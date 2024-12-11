#ifndef SQLITEHANDLER_H
#define SQLITEHANDLER_H
#include <string>

#include "Measurement.h"
#include "Station.h"
#include "SQLiteCpp/Database.h"


class SQLiteHandler {
public:
    explicit SQLiteHandler(std::string database);
    void init();
    Measurement getMeasurement(std::string stationId);
    Station getStation(std::string stationId);
    Measurement& insertMeasurement(Measurement& measurement);
    Station& insertStation(Station& station);
    Measurement& updateMeasurements(Measurement& measurement);
    Station& updateStation(Station& station);
    bool deleteMeasurements(Measurement& measurement);
    bool deleteStation(Station& station);
    bool cleanDatabase();
    std::vector<Measurement> getAllMeasurements();
    std::vector<Station> getAllStations();
    int countMeasurements();
    int countStations();
    ~SQLiteHandler();
private:
    std::string database;
    SQLite::Database db;
};



#endif //SQLITEHANDLER_H
