#include "SQLiteHandler.h"

#include <iostream>

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

Measurement SQLiteHandler::getMeasurement(std::string measurementId) {

}

Station SQLiteHandler::getStation(std::string stationId) {
    SQLite::Statement query(db,"SELECT id, name, longitude, latitude, elevation, callSign FROM stations WHERE id = ?;");
    query.bind(1, stationId);

    while (query.executeStep())
    {
        Station station;
        station.id = query.getColumn(0).getText();
        station.name = query.getColumn(1).getText();
        station.longitude = query.getColumn(2).getDouble();
        station.latitude = query.getColumn(3).getDouble();
        station.elevation = query.getColumn(4).getDouble();
        station.callSign = query.getColumn(5).getString();

        return station;
    }
}

Measurement & SQLiteHandler::insertMeasurement(Measurement &measurement) {
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

std::vector<Measurement> SQLiteHandler::getAllMeasurements() {
}

std::vector<Station> SQLiteHandler::getAllStations() {
}

int SQLiteHandler::countMeasurements() {
}

int SQLiteHandler::countStations() {
}

SQLiteHandler::~SQLiteHandler() {
    db = nullptr;
}
