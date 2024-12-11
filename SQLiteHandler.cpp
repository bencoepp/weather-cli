#include "SQLiteHandler.h"
#include <iostream>
#include <random>
#include <windows.h>

SQLiteHandler::SQLiteHandler(const std::string& database): db(database, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE) {
    this->database = database;
}

void SQLiteHandler::init() {
    try {
        db.exec(R"(
            CREATE TABLE IF NOT EXISTS stations (
                id TEXT PRIMARY KEY,
                name TEXT,
                longitude REAL,
                latitude REAL,
                elevation REAL,
                callSign TEXT
            );
        )");

    }catch (const SQLite::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    try {
         db.exec(R"(
                CREATE TABLE IF NOT EXISTS measurements (
                    id TEXT PRIMARY KEY,
                    station TEXT,
                    date TEXT,
                    reportType TEXT,
                    qualityControlFlag TEXT,
                    wind TEXT,
                    cloudCeiling REAL,
                    visibilityDistance REAL,
                    temperature REAL,
                    dewPoints REAL,
                    seaLevelPressure REAL,
                    hourlyPrecipitation TEXT,  -- Comma-separated list of doubles
                    snowfall REAL,
                    durationOfPrecipitation REAL,
                    weatherPhenomena TEXT,
                    skyCover TEXT,  -- Comma-separated list of strings
                    atmosphericParameters TEXT,  -- Comma-separated list of strings
                    freezingRainObservations TEXT,
                    lightningActivity TEXT,
                    atmosphericPressureTendency TEXT,
                    temperatureObservations TEXT,  -- Comma-separated list of doubles
                    solarRadiation TEXT,
                    soilConditionsOrGroundTemperature TEXT,  -- Comma-separated list of strings
                    frostObservations TEXT,
                    precipitationTypeIdentifier TEXT,
                    groundConditionsOrSnowDepth TEXT,
                    precipitationMetrics TEXT,
                    iceObservations TEXT,
                    groundFrostObservations TEXT,
                    SolarOrAtmosphericRadiationDetails TEXT,
                    temperatureExtremes TEXT,  -- Comma-separated list of strings
                    extremeWindConditions TEXT,  -- Comma-separated list of strings
                    condensationMeasurements TEXT,
                    soilMoisture TEXT,
                    soilTemperature TEXT,
                    groundObservations TEXT,
                    surfaceMetrics TEXT,
                    snowWaterEquivalent TEXT,
                    multiLayerWindObservations TEXT,  -- Comma-separated list of strings
                    observationConditionFlags TEXT,
                    errorsOrMissingDataIndicators TEXT,  -- Comma-separated list of strings
                    remarksOrAdditionalNotes TEXT,
                    equipmentDiagnosticsMetadata TEXT
                );
        )");
    }catch (const SQLite::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

Measurement SQLiteHandler::getMeasurement(const std::string &measurementId) const {
    SQLite::Statement query(db,"SELECT id, station, date, reportType, qualityControlFlag, wind, cloudCeiling, visibilityDistance, temperature, dewPoints, seaLevelPressure FROM measurements WHERE id = ?;");
    query.bind(1, measurementId);
    auto measurement = Measurement();

    while (query.executeStep()) {
        measurement.id = query.getColumn(0).getText();
        measurement.station = query.getColumn(1).getText();
        measurement.date = query.getColumn(2).getText();
        measurement.reportType = query.getColumn(3).getText();
        measurement.qualityControlFlag = query.getColumn(4).getText();
        measurement.wind = query.getColumn(5).getText();
        measurement.cloudCeiling = query.getColumn(6).getDouble();
        measurement.visibilityDistance = query.getColumn(7).getDouble();
        measurement.temperature = query.getColumn(8).getDouble();
        measurement.dewPoints = query.getColumn(9).getDouble();
        measurement.seaLevelPressure = query.getColumn(10).getDouble();
    }

    return measurement;
}

Station SQLiteHandler::getStation(const std::string &stationId) const {
    SQLite::Statement query(db,"SELECT id, name, longitude, latitude, elevation, callSign FROM stations WHERE id = ?;");
    query.bind(1, stationId);
    auto station = Station();

    while (query.executeStep())
    {
        station.id = query.getColumn(0).getText();
        station.name = query.getColumn(1).getText();
        station.longitude = query.getColumn(2).getDouble();
        station.latitude = query.getColumn(3).getDouble();
        station.elevation = query.getColumn(4).getDouble();
        station.callSign = query.getColumn(5).getString();

    }
    return station;

}

Measurement & SQLiteHandler::insertMeasurement(Measurement &measurement) const {
    measurement.id = generateUniqueId("measurements");

    SQLite::Statement query(db, R"(INSERT INTO measurements (id, station, date, reportType, qualityControlFlag, wind, cloudCeiling, visibilityDistance, temperature, dewPoints, seaLevelPressure)
                                   VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?))");

    query.bind(1, measurement.id);
    query.bind(2, measurement.station);
    query.bind(3, measurement.date);
    query.bind(4, measurement.reportType);
    query.bind(5, measurement.qualityControlFlag);
    query.bind(6, measurement.wind);
    query.bind(7, measurement.cloudCeiling);
    query.bind(8, measurement.visibilityDistance);
    query.bind(9, measurement.temperature);
    query.bind(10, measurement.dewPoints);
    query.bind(11, measurement.seaLevelPressure);

    query.exec();
    return measurement;
}

Station & SQLiteHandler::insertStation(Station &station) const {
    SQLite::Statement query(db, R"(INSERT INTO stations (id, name, longitude, latitude, elevation, callSign)
                                   VALUES (?, ?, ?, ?, ?, ?))");

    query.bind(1, station.id);
    query.bind(2, station.name);
    query.bind(3, station.longitude);
    query.bind(4, station.latitude);
    query.bind(5, station.elevation);
    query.bind(6, station.callSign);

    query.exec();

    return station;
}

Measurement & SQLiteHandler::updateMeasurements(Measurement &measurement) {
}

Station & SQLiteHandler::updateStation(Station &station) {
}

bool SQLiteHandler::deleteMeasurements(Measurement &measurement) {
}

bool SQLiteHandler::deleteStation(Station &station) {
}

bool SQLiteHandler::cleanDatabase() {
    try {
        db.exec("DROP TABLE measurements;");
        db.exec("DROP TABLE stations;");
        return true;
    }catch (SQLite::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Measurement> SQLiteHandler::getAllMeasurements() const {
    SQLite::Statement query(db, "SELECT * FROM measurements;");
    std::vector<Measurement> measurements;

    while (query.executeStep()) {
        Measurement measurement;
        measurement.id = query.getColumn(0).getText();
        measurement.station = query.getColumn(1).getText();
        measurement.date = query.getColumn(2).getText();
        measurement.reportType = query.getColumn(3).getText();
        measurement.qualityControlFlag = query.getColumn(4).getText();
        measurement.wind = query.getColumn(5).getText();
        measurement.cloudCeiling = query.getColumn(6).getDouble();
        measurement.visibilityDistance = query.getColumn(7).getDouble();
        measurement.temperature = query.getColumn(8).getDouble();
        measurement.dewPoints = query.getColumn(9).getDouble();
        measurement.seaLevelPressure = query.getColumn(10).getDouble();
        measurements.push_back(measurement);
    }

    return measurements;

}

std::vector<Station> SQLiteHandler::getAllStations() const {
    SQLite::Statement query(db, "SELECT * FROM stations;");
    std::vector<Station> stations;

    while (query.executeStep()) {
        Station station;
        station.id = query.getColumn(0).getText();
        station.name = query.getColumn(1).getText();
        station.longitude = query.getColumn(2).getDouble();
        station.latitude = query.getColumn(3).getDouble();
        station.elevation = query.getColumn(4).getDouble();
        station.callSign = query.getColumn(5).getString();
        stations.push_back(station);
    }

    return stations;
}

int SQLiteHandler::countMeasurements() const {
    SQLite::Statement query(db, "SELECT COUNT(*) FROM measurements;");
    query.executeStep();
    return query.getColumn(0).getInt();
}

int SQLiteHandler::countStations() const {
    SQLite::Statement query(db, "SELECT COUNT(*) FROM stations;");
    query.executeStep();
    return query.getColumn(0).getInt();
}

SQLiteHandler::~SQLiteHandler() {
    db = nullptr;
}

std::string SQLiteHandler::generateUniqueId(const std::string &table) const {
    while (true) {
        std::string randomId = generateId(32);
        SQLite::Statement checkQuery(db, "SELECT COUNT(*) FROM " + table + " WHERE id = ?");
        checkQuery.bind(1, randomId);
        if (checkQuery.executeStep() && checkQuery.getColumn(0).getInt() == 0) {
            return randomId;
        }
    }
}

std::string SQLiteHandler::generateId(const size_t length) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, chars.size() - 1);

    std::string randomString;
    for (size_t i = 0; i < length; ++i) {
        randomString += chars[distribution(generator)];
    }
    return randomString;
}
