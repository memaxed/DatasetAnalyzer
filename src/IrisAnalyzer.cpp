#include "IrisAnalyzer.h"
#include "BIQTIris.h"

#include <iostream>
#include <stdexcept>
#include <json/json.h>

IrisAnalyzer::IrisAnalyzer() = default;
IrisAnalyzer::~IrisAnalyzer() = default;

void IrisAnalyzer::initialize() {
    initialized_ = true;
}

IrisResult IrisAnalyzer::analyze(const std::string& imagePath) {
    if (!initialized_) {
        throw std::runtime_error("IrisAnalyzer not initialized. Call initialize() first.");
    }

    BIQTIris engine;
    Provider::EvaluationResult evalResult = engine.evaluate(imagePath);

    const char* rawJson = Provider::serializeResult(evalResult);
    std::string json(rawJson);
    delete[] rawJson;

    return parseJSON(imagePath, json);
}

/* BIQTIris JSON format:
 * {
 *   "errorCode": 0,
 *   "provider": "BIQTIris",
 *   "message": "",
 *   "qualityResult": [
 *     {
 *       "metrics":  { "iso_overall_quality": 63.0, ... },
 *       "features": { "image_width": 320, ... }
 *     }
 *   ]
 * }
 */
IrisResult IrisAnalyzer::parseJSON(const std::string& imagePath, const std::string& json) {
    IrisResult result;
    result.imagePath = imagePath;

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json, root)) {
        result.error = "Failed to parse JSON: " + reader.getFormattedErrorMessages();
        return result;
    }

    int errorCode = root["errorCode"].asInt();
    if (errorCode != 0) {
        result.error = "BIQTIris error " + std::to_string(errorCode) +
                       ": " + root["message"].asString();
        return result;
    }

    const Json::Value& qualityResults = root["qualityResult"];
    if (qualityResults.empty()) {
        result.error = "No iris detected in image";
        return result;
    }

    const Json::Value& qr = qualityResults[0];
    for (const auto& key : qr["metrics"].getMemberNames()) {
        result.metrics[key] = qr["metrics"][key].asDouble();
    }
    for (const auto& key : qr["features"].getMemberNames()) {
        result.features[key] = qr["features"][key].asDouble();
    }

    return result;
}
