#ifndef SQLITEHANDLER_H
#define SQLITEHANDLER_H
#include <string>

#include "Measurement.h"
#include "Station.h"
#include "SQLiteCpp/Database.h"


class SQLiteHandler {
public:
    explicit SQLiteHandler(const std::string& database);
    void init();
    Measurement getMeasurement(const std::string &measurementId) const;
    Station getStation(const std::string &stationId) const;
    Measurement& insertMeasurement(Measurement& measurement) const;
    void insertMeasurements(std::vector<Measurement>& measurements) const;
    Station& insertStation(Station& station) const;
    void insertStations(std::vector<Station>& stations) const;
    Measurement& updateMeasurements(Measurement& measurement);
    Station& updateStation(Station& station);
    bool deleteMeasurements(Measurement& measurement);
    bool deleteStation(Station& station);
    bool cleanDatabase();
    std::vector<Measurement> getAllMeasurements() const;
    std::vector<Station> getAllStations() const;
    int countMeasurements() const;
    int countStations() const;
    ~SQLiteHandler();
    std::vector<std::map<std::string, std::string>> executeQuery(const std::string &query);

    std::string generateUniqueId(const std::string &table) const;

private:
    std::string database;
    SQLite::Database db;

    static std::string generateId(size_t length);
};



#endif //SQLITEHANDLER_H
