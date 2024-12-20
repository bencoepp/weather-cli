﻿#ifndef MEASUREMENT_H
#define MEASUREMENT_H
#include <string>
#include <ctime>
#include <vector>

/**
 * @class Measurement
 * @brief Represents a meteorological measurement containing various observations and metrics.
 *
 * The Measurement class encapsulates a wide range of meteorological data, from basic atmospheric
 * conditions to more detailed environmental information. It is designed to represent a single
 * measurement or observation and provides storage for data such as wind, temperature, precipitation,
 * and more.
 *
 * The class includes static functionality to parse data from a CSV-formatted string into a
 * Measurement object.
 */
class Measurement {
public:
    std::string id;
    std::string station;
    std::string date;
    std::string reportType;
    std::string qualityControlFlag;
    std::string wind;
    double cloudCeiling;
    double visibilityDistance;
    double temperature;
    double dewPoints;
    double seaLevelPressure;
    std::vector<double> hourlyPrecipitation;
    double snowfall;
    double durationOfPrecipitation;
    std::string weatherPhenomena;
    std::vector<std::string> skyCover;
    std::vector<std::string> atmosphericParameters;
    std::string freezingRainObservations;
    std::string lightningActivity;
    std::string atmosphericPressureTendency;
    std::vector<double> temperatureObservations;
    std::string solarRadiation;
    std::vector<std::string> soilConditionsOrGroundTemperature;
    std::string frostObservations;
    std::string precipitationTypeIdentifier;
    std::string groundConditionsOrSnowDepth;
    std::string precipitationMetrics;
    std::string iceObservations;
    std::string groundFrostObservations;
    std::string SolarOrAtmosphericRadiationDetails;
    std::vector<std::string> temperatureExtremes;
    std::vector<std::string> extremeWindConditions;
    std::string condensationMeasurements;
    std::string soilMoisture;
    std::string soilTemperature;
    std::string groundObservations;
    std::string surfaceMetrics;
    std::string snowWaterEquivalent;
    std::vector<std::string> multiLayerWindObservations;
    std::string observationConditionFlags;
    std::vector<std::string> errorsOrMissingDataIndicators;
    std::string remarksOrAdditionalNotes;
    std::string equipmentDiagnosticsMetadata;

    static Measurement fromCsv(std::string line);
};



#endif //MEASUREMENT_H
