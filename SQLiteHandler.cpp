#include "SQLiteHandler.h"
#include <iostream>
#include <random>
#include <windows.h>

/**
 * @class SQLiteHandler
 * @brief Handles the initialization and management of an SQLite database connection.
 *
 * The SQLiteHandler class is responsible for opening a connection to an SQLite database
 * with the specified file name and flags. By default, it attempts to open the database
 * in read/write mode and create the database file if it does not exist. It wraps around
 * the SQLite library's functionality to simplify database management.
 *
 * Resource management is handled through RAII, ensuring the database connection is
 * properly closed when the object is destroyed.
 *
 * Thread-safety: This class is not thread-safe by itself. External synchronization is
 * required if accessing the same SQLiteHandler instance from multiple threads.
 *
 * Exception safety: Throws exceptions related to database opening or initialization errors,
 * which must be handled where the SQLiteHandler is instantiated.
 *
 * @param[in] database The name of the database file to open or create.
 *                     If the file does not exist, it will be created.
 */
SQLiteHandler::SQLiteHandler(const std::string& database): db(database, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE) {
    this->database = database;
}

/**
 * @brief Initializes the SQLite database and ensures required tables are created.
 *
 * The init method sets up the necessary database schema for the application. It creates
 * tables if they do not already exist, ensuring the "stations" and "measurements" tables
 * are present. This method uses SQL statements resistant to duplication to avoid conflicts
 * during initialization.
 *
 * The "stations" table stores metadata about various monitoring stations, including attributes
 * like id, name, coordinates, elevation, and call sign.
 *
 * The "measurements" table is a comprehensive schema for recording meteorological and related
 * data, including temperature, wind, precipitation metrics, weather conditions, and other observations.
 * It supports a variety of data types and structured fields such as comma-separated values for
 * additional details.
 *
 * Error handling: Any exception thrown by SQLite operations is caught and logged, ensuring that
 * application initialization does not abruptly terminate. Errors are output to the standard error stream.
 *
 * Note: This method assumes the database connection has already been established before execution.
 */
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

/**
 * @brief Retrieves a measurement record from the database based on the provided measurement ID.
 *
 * This method queries the connected SQLite database for a specific measurement record with the
 * given ID. It retrieves various parameters such as station, date, weather report type, and
 * meteorological measurements including wind, cloud ceiling, visibility, temperature, dew points,
 * and sea-level pressure.
 *
 * The result is populated into a `Measurement` object and returned. The method assumes that the
 * table structure and schema match the query, and the measurement ID exists in the database.
 * The method binds the provided measurement ID to the query before execution.
 *
 * Exception safety: May throw exceptions related to SQLite operation errors, such as query syntax
 * issues or database connection problems. The user must handle these appropriately where this method
 * is called.
 *
 * Thread-safety: This method is not thread-safe if the `SQLiteHandler` instance is accessed
 * concurrently without external synchronization.
 *
 * @param[in] measurementId A string representing the unique identifier of the measurement to retrieve.
 * @return A `Measurement` object containing the data for the specified measurement ID. If no record
 *         is found, the returned object may contain default or empty fields.
 */
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

/**
 * @brief Retrieves a station record from the database based on its unique identifier.
 *
 * Executes a SQLite query to fetch station details such as id, name, longitude, latitude,
 * elevation, and call sign from the `stations` table where the station ID matches the provided value.
 * The details are then mapped to a Station object and returned.
 *
 * Thread-safety: This function is not thread-safe by itself and requires external
 * synchronization if called from multiple threads accessing the same database connection.
 *
 * Exception safety: Throws exceptions for errors related to query preparation or
 * database execution. These exceptions must be handled where this function is called.
 *
 * @param[in] stationId The unique identifier of the station to retrieve.
 * @return Station An object representing the station with attributes populated from the database.
 */
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

/**
 * @brief Inserts a new measurement into the database.
 *
 * This method takes a reference to a Measurement object, assigns it a unique identifier,
 * and inserts its data into the "measurements" table within the database. The measurement
 * object is updated with the newly generated ID before being returned.
 *
 * The following fields of the Measurement object are stored in the database:
 * - id
 * - station
 * - date
 * - reportType
 * - qualityControlFlag
 * - wind
 * - cloudCeiling
 * - visibilityDistance
 * - temperature
 * - dewPoints
 * - seaLevelPressure
 *
 * The database operation is performed through a parameterized query to ensure data
 * integrity and prevent SQL injection.
 *
 * @param[in,out] measurement A reference to a Measurement object containing the data
 *                             to be inserted into the database.
 *                             This object will be updated with the newly assigned unique ID.
 * @return A reference to the updated Measurement object.
 */
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

/**
 * @brief Inserts a list of measurements into the SQLite database.
 *
 * This method iterates through the provided list of measurements, generates a unique ID
 * for each measurement entry, and inserts the corresponding data into the `measurements`
 * table in the SQLite database. The function utilizes prepared statements for batching
 * the insert operations efficiently.
 *
 * Thread-safety: This method is not thread-safe. Proper synchronization is required when
 * accessed from multiple threads.
 *
 * Exception safety: Throws exceptions if there are issues with database connectivity, invalid SQL,
 * or binding errors. Exception handling must be implemented by the caller.
 *
 * @param[in,out] measurements A vector of Measurement objects. Each measurement in the vector
 *                              will have its unique ID assigned during insertion.
 */
void SQLiteHandler::insertMeasurements(std::vector<Measurement> &measurements) const {
    SQLite::Statement query(db, R"(INSERT INTO measurements (id, station, date, reportType, qualityControlFlag, wind, cloudCeiling, visibilityDistance, temperature, dewPoints, seaLevelPressure)
                                   VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?))");
    for (Measurement &measurement : measurements) {
        measurement.id = generateUniqueId("measurements");

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
        query.clearBindings();
        query.reset();
    }
}

/**
 * @brief Inserts a station record into the SQLite database.
 *
 * This method takes a Station object as input and inserts its attributes into
 * the `stations` table of the SQLite database. It uses parameterized queries
 * to bind the attributes of the Station object to the corresponding columns
 * in the database table. The operation is executed using the SQLite library.
 *
 * The `stations` table is expected to have the following columns:
 * - id
 * - name
 * - longitude
 * - latitude
 * - elevation
 * - callSign
 *
 * The method assumes that all necessary fields in the Station object are initialized
 * and that the database schema is correctly set up to accept the input.
 *
 * Exception safety: If there is an error during query preparation, binding, or execution,
 * the corresponding SQLite exception will be thrown. Callers must handle these exceptions.
 *
 * @param[out] station The Station object whose data will be inserted into the database.
 * @return A reference to the input Station object.
 */
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

/**
 * @brief Inserts a list of station records into the database.
 *
 * This method takes a vector of Station objects and inserts them into the `stations`
 * table in the SQLite database. Each Station's properties are bound to the
 * corresponding columns in the table, and the records are added sequentially.
 *
 * Exception safety: The method will throw exceptions if there are issues with the
 * database connection, query execution, or data binding.
 *
 * @param[in,out] stations A vector of Station objects containing the station data to be added.
 *                         The objects in the vector can be modified during the process, if needed.
 */
void SQLiteHandler::insertStations(std::vector<Station> &stations) const {
    SQLite::Statement query(db, R"(INSERT INTO stations (id, name, longitude, latitude, elevation, callSign)
                                   VALUES (?, ?, ?, ?, ?, ?))");
    for (Station &station : stations) {
        query.bind(1, station.id);
        query.bind(2, station.name);
        query.bind(3, station.longitude);
        query.bind(4, station.latitude);
        query.bind(5, station.elevation);
        query.bind(6, station.callSign);
        query.exec();
        query.clearBindings();
        query.reset();
    }
}

/**
 * @brief Updates an existing measurement record in the SQLite database.
 *
 * This method modifies an existing measurement record in the database
 * based on the details provided in the input `Measurement` object.
 * The `Measurement` object should include an identifier that matches
 * a record in the database. The database record gets updated to match
 * the values provided in the input object. If the measurement does not exist,
 * this method may throw an error or behave as configured in the SQLite logic.
 *
 * @param[in, out] measurement A reference to the `Measurement` object containing
 *                             the updated values for the database record. This object
 *                             must have a valid identifier to locate the database record.
 * @return A reference to the updated `Measurement` object. The returned object
 *         may reflect changes that occurred during the update operation.
 */
Measurement & SQLiteHandler::updateMeasurements(Measurement &measurement) {
}

/**
 * @brief Updates the specified station record in the SQLite database.
 *
 * Provides functionality to modify and persist the properties of an existing station
 * record in the database. The station object that is passed as input should contain
 * the updated values, including its unique identifier which is used to locate the
 * corresponding record in the database.
 *
 * The station object provided as input is validated and then saved into the database.
 * Any pre-existing record with the same unique identifier will be overwritten with
 * the new values. This operation ensures data consistency by updating all relevant
 * fields.
 *
 * Resource management: This function depends on an active connection to the SQLite database.
 * It is the caller's responsibility to ensure the SQLiteHandler instance is properly
 * initialized and connected.
 *
 * Exception safety: Throws exceptions in case of SQL execution errors or invalid
 * input data. Users of this function must handle these exceptions.
 *
 * Thread-safety: Not guaranteed to be thread-safe. External synchronization may
 * be necessary if accessed concurrently.
 *
 * @param[in,out] station Reference to a Station object containing the updated
 *                         data to be saved. Upon successful execution, it reflects
 *                         the data stored in the database.
 *
 * @return A reference to the updated Station object for potential further use.
 */
Station & SQLiteHandler::updateStation(Station &station) {
}

/**
 * @brief Deletes a specific measurement record from the database.
 *
 * This method attempts to delete the provided measurement record from the database.
 * If the record exists, it will be removed. Otherwise, the method will return false,
 * indicating that no matching record was found or removed.
 *
 * @param[in] measurement The measurement object representing the record to be deleted.
 * @return True if the measurement record was successfully deleted, otherwise false.
 */
bool SQLiteHandler::deleteMeasurements(Measurement &measurement) {
}

/**
 * @brief Deletes a specified station entry from the SQLite database.
 *
 * This method attempts to delete the provided station entry from the database, based on the
 * station's unique identifier. If the station is successfully located and removed, the method
 * returns true. If the station does not exist in the database or cannot be deleted, it returns false.
 *
 * This method does not handle cascading deletions or associations with other records.
 *
 * @param[in] station A reference to the Station object that needs to be deleted.
 *                    The station must contain a valid ID for the deletion to be processed.
 * @return True if the station was deleted successfully, otherwise false.
 */
bool SQLiteHandler::deleteStation(Station &station) {
}

/**
 * @brief Cleans the SQLite database by dropping specific tables.
 *
 * The cleanDatabase method removes tables 'measurements' and 'stations'
 * from the connected SQLite database. This operation is irreversible
 * and should be used with caution, as it deletes all data within
 * these tables.
 *
 * Thread-safety: This method is not thread-safe by itself. External
 * synchronization is required if accessing the same SQLiteHandler
 * instance from multiple threads.
 *
 * Exception safety: If an SQLite exception occurs during execution,
 * an error message will be logged, and the method will return false.
 * The exception is not propagated to the caller.
 *
 * @return true if all tables were successfully dropped, false if
 * an error occurred during the operation.
 */
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

/**
 * @brief Retrieves all measurements from the "measurements" table in the SQLite database.
 *
 * This method queries the "measurements" table, reads all rows, and constructs a vector
 * of Measurement objects representing the data in the table. Each Measurement object
 * corresponds to a single row, with its fields populated from the respective columns in
 * the table.
 *
 * @return A vector of Measurement objects containing all rows retrieved from the database.
 *         If the table is empty, returns an empty vector.
 */
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

/**
 * @brief Retrieves all station records from the database.
 *
 * This method executes a SQL query to fetch all records from the "stations" table
 * and creates a list of Station objects based on the retrieved data. Each Station
 * object contains the station's id, name, geographical coordinates (longitude and latitude),
 * elevation, and call sign.
 *
 * @return A vector of Station objects representing all stations stored in the database.
 *         If there are no records in the "stations" table, an empty vector is returned.
 */
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

/**
 * @brief Counts the number of measurements stored in the "measurements" table of the database.
 *
 * This method executes a SQL query to count all rows in the "measurements" table.
 * It ensures accurate retrieval of the total measurement count from the database.
 * A valid database connection must be established before invoking this method.
 *
 * Thread-safety: This method is not inherently thread-safe. External synchronization
 * is required if accessing the same SQLiteHandler instance from multiple threads.
 *
 * Exception safety: May throw exceptions related to SQL query errors or database access issues.
 *
 * @return The total number of measurements in the "measurements" table.
 */
int SQLiteHandler::countMeasurements() const {
    SQLite::Statement query(db, "SELECT COUNT(*) FROM measurements;");
    query.executeStep();
    return query.getColumn(0).getInt();
}

/**
 * @brief Counts the number of stations in the database.
 *
 * This method executes an SQL query to count the total number of rows in
 * the "stations" table of the database. It uses an SQLite prepared statement
 * for executing the query and retrieving the result.
 *
 * Thread-safety: This method assumes exclusive access to the database
 * connection. If the SQLiteHandler is accessed from multiple threads, ensure
 * proper synchronization.
 *
 * Exception safety: Throws exceptions if the SQL statement fails or the
 * database connection is invalid. These exceptions must be appropriately
 * handled by the caller.
 *
 * @return The total number of stations in the database as an integer.
 */
int SQLiteHandler::countStations() const {
    SQLite::Statement query(db, "SELECT COUNT(*) FROM stations;");
    query.executeStep();
    return query.getColumn(0).getInt();
}

/**
 * @brief Destructor for the SQLiteHandler class.
 *
 * Responsible for gracefully releasing resources used by the SQLiteHandler instance.
 * The destructor explicitly nullifies the internal database pointer to ensure that
 * any dynamic associations or dependencies are cleared during object destruction.
 *
 * This destructor complements the RAII idiom by safely managing cleanup without
 * explicit calls by the user. After the destructor is called, the internal state
 * is reset to prevent illegal access or double deletion.
 *
 * Thread-safety: External synchronization is required if accessing shared instances.
 *
 * Exception safety: This method does not throw exceptions.
 */
SQLiteHandler::~SQLiteHandler() {
    db = nullptr;
}

std::vector<std::map<std::string, std::string>> SQLiteHandler::executeQuery(const std::string &query) {
    SQLite::Statement statement(db, query);
    std::vector<std::map<std::string, std::string>> values;

    while (statement.executeStep()) {
        std::map<std::string, std::string> row;
        for (int i = 0; i < statement.getColumnCount(); ++i) {
            row[statement.getColumnName(i)] = statement.getColumn(i).getText();
        }
        values.push_back(row);
    }
    return values;
}

/**
 * @brief Generates a unique identifier for a specified database table by ensuring no conflicts.
 *
 * This method repeatedly generates random IDs and checks if the ID already exists in the
 * specified database table. It continues until a unique ID is found and then returns it.
 * The length of the ID is fixed at 32 characters, as determined by the `generateId` method.
 *
 * The uniqueness of the ID is checked using a SQL query executed against the provided table name.
 * This method is designed for internal database operations where unique IDs are required for new entries.
 *
 * @param[in] table The name of the database table in which the generated ID must be unique.
 *
 * @return A unique 32-character identifier that does not exist in the specified table.
 */
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

/**
 * @brief Generates a random alphanumeric string of the specified length.
 *
 * This function creates a random string composed of lowercase letters,
 * uppercase letters, and digits. The generated string can be used, for example,
 * as a unique identifier or token.
 *
 * Thread-safety: Since this function utilizes a local random number generator,
 * it is thread-safe when accessed concurrently by multiple threads.
 *
 * Exception safety: If memory allocation for the result string fails, an exception
 * may be thrown. This should be handled by the caller.
 *
 * @param[in] length The length of the random string to generate.
 *                   It must be greater than 0 for meaningful output.
 * @return A random alphanumeric string of the specified length.
 */
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
