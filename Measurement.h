//
// Created by benco on 09/12/2024.
//

#ifndef MEASUREMENT_H
#define MEASUREMENT_H
#include <string>
#include <ctime>
#include <vector>

class Measurement {
public:
    std::string id;
    std::string station;
    std::time_t date;
    std::string reportType;
    std::string qualityControlFlag;
    double wind;
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
};



#endif //MEASUREMENT_H
