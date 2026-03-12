#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

struct IrisResult {
    std::string imagePath;
    std::string error;
    std::map<std::string, double> metrics;
    std::map<std::string, double> features;

    bool hasError() const { return !error.empty(); }

    double getMetric(const std::string& key, double fallback = -1.0) const {
        auto it = metrics.find(key);
        return (it != metrics.end()) ? it->second : fallback;
    }

    double getFeature(const std::string& key, double fallback = -1.0) const {
        auto it = features.find(key);
        return (it != features.end()) ? it->second : fallback;
    }
};

class IrisAnalyzer {
public:
    IrisAnalyzer();
    ~IrisAnalyzer();

    // loads BIQTIris shared library
    void initialize();

    // analyzes a single image file, returns parsed results.
    IrisResult analyze(const std::string& imagePath);

    bool isInitialized() const { return initialized_; }

private:
    bool initialized_ = false;

    // parse JSON output from BIQTIris into IrisResult.
    static IrisResult parseJSON(const std::string& imagePath, const std::string& json);
};
