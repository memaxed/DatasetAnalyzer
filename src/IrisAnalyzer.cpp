#include "IrisAnalyzer.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <dlfcn.h>
#include <json/json.h>

IrisAnalyzer::IrisAnalyzer() = default;

IrisAnalyzer::~IrisAnalyzer() {
    unloadLibrary();
}

void IrisAnalyzer::initialize(const std::string& providerPath) {
    std::string libPath = providerPath;
    if (libPath.empty()) {
        libPath = autoDetectLibrary();
    }

    if (libPath.empty()) {
        throw std::runtime_error(
            "libBIQTIris.so not found. Specify path with --provider, or install BIQT:\n"
            "  https://github.com/mitre/biqt\n"
            "  https://github.com/mitre/biqt-iris\n"
            "Locations checked:\n"
            "  $BIQT_HOME/providers/BIQTIris/libBIQTIris.so\n"
            "  /usr/local/share/biqt/providers/BIQTIris/libBIQTIris.so\n"
            "  ./libBIQTIris.so"
        );
    }

    loadLibrary(libPath);
    initialized_ = true;
}

void IrisAnalyzer::loadLibrary(const std::string& path) {
    unloadLibrary();

    libHandle_ = dlopen(path.c_str(), RTLD_LAZY);
    if (!libHandle_) {
        throw std::runtime_error(
            "Failed to load '" + path + "': " + dlerror());
    }

    void* sym = dlsym(libHandle_, "provider_eval");
    if (!sym) {
        dlclose(libHandle_);
        libHandle_ = nullptr;
        throw std::runtime_error(
            "'" + path + "' does not export 'provider_eval'. "
            "Make sure this is the BIQTIris provider library.");
    }

    evaluateFn_ = reinterpret_cast<ProviderEvalFn>(sym);
}

void IrisAnalyzer::unloadLibrary() {
    evaluateFn_ = nullptr;
    if (libHandle_) {
        dlclose(libHandle_);
        libHandle_ = nullptr;
    }
    initialized_ = false;
}

IrisResult IrisAnalyzer::analyze(const std::string& imagePath) {
    if (!initialized_ || !evaluateFn_) {
        throw std::runtime_error("IrisAnalyzer not initialized. Call initialize() first.");
    }

    // provider_eval returns JSON string allocated with new[]; caller must delete[].
    const char* rawResult = evaluateFn_(imagePath.c_str());
    if (!rawResult) {
        throw std::runtime_error("BIQTIris returned null for: " + imagePath);
    }

    std::string json(rawResult);
    delete[] rawResult;

    if (json.empty()) {
        throw std::runtime_error("BIQTIris returned empty result for: " + imagePath);
    }

    return parseJSON(imagePath, json);
}

/* BIQTIris json format
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

std::string IrisAnalyzer::autoDetectLibrary() {
    std::vector<std::string> candidates;

    const char* biqtHome = std::getenv("BIQT_HOME");
    if (biqtHome) {
        candidates.push_back(std::string(biqtHome) + "/providers/BIQTIris/libBIQTIris.so");
        candidates.push_back(std::string(biqtHome) + "/lib/libBIQTIris.so");
    }

    candidates.push_back("/usr/local/share/biqt/providers/BIQTIris/libBIQTIris.so");
    candidates.push_back("/usr/share/biqt/providers/BIQTIris/libBIQTIris.so");
    candidates.push_back("/opt/biqt/providers/BIQTIris/libBIQTIris.so");
    candidates.push_back("./libBIQTIris.so");

    for (const auto& path : candidates) {
        std::ifstream f(path);
        if (f.good()) {
            std::cout << "Found BIQTIris library at: " << path << "\n";
            return path;
        }
    }

    return "";
}
