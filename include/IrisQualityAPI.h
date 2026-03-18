#pragma once

/**
 * @file IrisQualityAPI.h
 * @brief Public API for iris image quality assessment using the embedded BIQTIris engine.
 *
 * @code
 *   #include "IrisQualityAPI.h"
 *
 *   // Analyze a single image
 *   cv::Mat frame = cv::imread("iris.png", cv::IMREAD_GRAYSCALE);
 *   DetectedIris det = ...;
 *
 *   std::map<std::string, float> scores;
 *   int rc = QualityAPI::assess(frame, det, &scores);
 * 
 *   // You can use scores["metric"]  
 *   if (rc == 0)
 *       std::cout << "Quality: " << scores["irisBiqtQuality"] << "\n";
 *
 *   // Or use already implemented methods:
 *   std::cout << "Label: " << QualityAPI::qualityLabel(scores) << "\n";
 *   std::cout << "Sharpness: " << QualityAPI::isoSharpness(scores) << "\n";
 * @endcode
 */

#include "BIQTIris.h"

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

namespace QualityAPI {

// ---------------------------------------------------------------------------
// Analysis
// ---------------------------------------------------------------------------

/**
 * @brief Analyzes a single iris image.
 *
 * @param image        8-bpp grayscale cv::Mat (CV_8UC1, continuous memory).
 *                     Size must be within 256x256 – 1000x680 px.
 * @param detectedIris Externally computed iris/pupil centre and radius.
 * @param scores       Output map; Must not be nullptr.
 * @return  0 on success.
 *          1 if the image is invalid, too small, not CV_8UC1, or not continuous.
 *          2 if the image exceeds the maximum supported size.
 *          3 if an unexpected exception occurred.
 *
 * @code
 *   std::map<std::string, float> scores;
 *   int rc = QualityAPI::assess(frame, det, &scores);
 *   if (rc == 0)
 *       std::cout << scores["irisBiqtQuality"] << "\n";
 * @endcode
 */
inline int32_t assess(const cv::Mat& image,
                  const DetectedIris& detectedIris,
                  std::map<std::string, float>* scores) {
    BIQTIris engine;
    return engine.assessQuality(image, detectedIris, scores);
}

/**
 * @brief Analyzes a batch of frames.
 *
 * @param frames and @param detections must have the same length.
 * Each result map in @param results receives scores for the corresponding frame.
 *
 * @param frames     Vector of 8-bpp grayscale cv::Mat images.
 * @param detections Vector of DetectedIris, one per frame.
 * @param results    Output vector of score maps; will be resized to match
 *                   frames.size(). Must not be nullptr.
 * @return Vector of per-frame return codes (same semantics as assess()).
 * @throws std::invalid_argument if frames.size() != detections.size().
 *
 * @code
 *   std::vector<std::map<std::string, float>> results;
 *   auto codes = QualityAPI::assessAll(frames, detections, &results);
 * @endcode
 */
inline std::vector<int> assessAll(const std::vector<cv::Mat>& frames,
                                   const std::vector<DetectedIris>& detections,
                                   std::vector<std::map<std::string, float>>* results) {
    if (frames.size() != detections.size()) {
        throw std::invalid_argument(
            "QualityAPI::assessAll: frames and detections must have the same size.");
    }

    results->resize(frames.size());
    std::vector<int> codes;
    codes.reserve(frames.size());

    for (std::size_t i = 0; i < frames.size(); ++i) {
        BIQTIris engine;
        codes.push_back(engine.assessQuality(frames[i], detections[i], &(*results)[i]));
    }
    return codes;
}

// ---------------------------------------------------------------------------
// Overall quality
// ---------------------------------------------------------------------------

/**
 * @brief Returns the ISO overall quality score (0–100).
 *
 * The primary composite quality metric. Derived from all ISO sub-scores.
 * Higher is better. Returns -1.0f if unavailable.
 */
inline float overallQuality(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoOverallQuality");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the raw overall quality score (0–100).
 *
 * Non-ISO composite score computed directly by the BIQTIris algorithm,
 * before ISO normalization. Returns -1.0f if unavailable.
 */
inline float rawQuality(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtQuality");
    return (it != scores.end()) ? it->second : -1.0f;
}

// ---------------------------------------------------------------------------
// ISO metrics
// ---------------------------------------------------------------------------

/**
 * @brief Returns the ISO sharpness score (0–100).
 *
 * Measures focus/defocus of the iris texture. Low values indicate
 * motion blur or out-of-focus capture. Returns -1.0f if unavailable.
 */
inline float isoSharpness(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoSharpness");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the ISO usable iris area percentage (0–100).
 *
 * Fraction of the iris region not occluded by eyelids or eyelashes.
 * Low values indicate heavy occlusion. Returns -1.0f if unavailable.
 */
inline float isoUsableIrisArea(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoUsableIrisArea");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the ISO iris-pupil contrast score (0–100).
 *
 * Greyscale contrast between the iris and pupil regions.
 * Low values may indicate poor lighting or pigmentation issues.
 * Returns -1.0f if unavailable.
 */
inline float isoIrisPupilContrast(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoIrisPupilContrast");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the ISO iris-sclera contrast score (0–100).
 *
 * Greyscale contrast between the iris and the sclera (white of the eye).
 * Low values indicate difficulty segmenting the iris boundary.
 * Returns -1.0f if unavailable.
 */
inline float isoIrisScleraContrast(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoIrisScleraContrast");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the ISO iris-pupil ratio score (0–100).
 *
 * Ratio of pupil diameter to iris diameter. Extreme dilation or
 * constriction reduces this score. Returns -1.0f if unavailable.
 */
inline float isoIrisPupilRatio(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoIrisPupilRatio");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the ISO iris-pupil concentricity score (0–100).
 *
 * Measures how well the pupil center aligns with the iris center.
 * Low values indicate significant decentration. Returns -1.0f if unavailable.
 */
inline float isoIrisPupilConcentricity(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoIrisPupilConcentricity");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the ISO margin adequacy score (0–100).
 *
 * Measures whether sufficient iris margin is visible around the pupil.
 * Low values indicate the iris is too close to the image border.
 * Returns -1.0f if unavailable.
 */
inline float isoMarginAdequacy(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoMarginAdequacy");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the ISO greyscale utilization score (0–100).
 *
 * Measures how well the image uses the available greyscale dynamic range.
 * Low values indicate over- or under-exposure. Returns -1.0f if unavailable.
 */
inline float isoGreyscaleUtilization(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoGreyscaleUtilization");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the ISO pupil boundary circularity score (0–100).
 *
 * Measures how closely the pupil boundary approximates a circle.
 * Low values may indicate occlusion or segmentation errors.
 * Returns -1.0f if unavailable.
 */
inline float isoPupilBoundaryCircularity(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIsoPupilBoundaryCircularity");
    return (it != scores.end()) ? it->second : -1.0f;
}

// ---------------------------------------------------------------------------
// Raw metrics
// ---------------------------------------------------------------------------

/**
 * @brief Returns the raw contrast score (integer range).
 *
 * Direct contrast measurement before ISO normalization.
 * Returns -1.0f if unavailable.
 */
inline float contrast(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtContrast");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the raw sharpness / defocus score (integer range).
 *
 * Direct defocus measurement before ISO normalization.
 * Returns -1.0f if unavailable.
 */
inline float sharpness(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtSharpness");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the raw iris-sclera greyscale difference mean (float).
 *
 * Average difference in greyscale intensity between iris and sclera segments.
 * Returns -1.0f if unavailable.
 */
inline float irisScleraGS(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIrisScleraGs");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the raw iris-pupil greyscale difference (float).
 *
 * Greyscale intensity difference between the iris and pupil regions.
 * Returns -1.0f if unavailable.
 */
inline float irisPupilGS(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtIrisPupilGs");
    return (it != scores.end()) ? it->second : -1.0f;
}

/**
 * @brief Returns the average pupil circularity deviation (float).
 *
 * Mean deviation of the pupil boundary from a perfect circle, in pixels.
 * Returns -1.0f if unavailable.
 */
inline float pupilCircularityAvgDeviation(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtPupilCircularityAvgDeviation");
    return (it != scores.end()) ? it->second : -1.0f;
}

// ---------------------------------------------------------------------------
// Normalized scores (0.0–1.0)
// ---------------------------------------------------------------------------

/** @brief Returns the normalized contrast score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedContrast(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedContrast");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized sharpness score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedSharpness(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedSharpness");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized iris diameter score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIrisDiameter(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIrisDiameter");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized iris-sclera greyscale score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIrisScleraGS(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIrisScleraGs");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized iris-pupil greyscale score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIrisPupilGS(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIrisPupilGs");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO usable iris area score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoUsableIrisArea(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoUsableIrisArea");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO sharpness score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoSharpness(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoSharpness");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO iris-pupil ratio score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoIrisPupilRatio(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoIrisPupilRatio");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO iris-pupil contrast score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoIrisPupilContrast(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoIrisPupilContrast");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO iris-sclera contrast score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoIrisScleraContrast(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoIrisScleraContrast");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO margin adequacy score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoMarginAdequacy(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoMarginAdequacy");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO greyscale utilization score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoGreyscaleUtilization(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoGreyscaleUtilization");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO iris-pupil concentricity score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoIrisPupilConcentricity(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoIrisPupilConcentricity");
    return (it != scores.end()) ? it->second : -1.0f;
}

/** @brief Returns the normalized ISO iris diameter score (0.0–1.0). Returns -1.0f if unavailable. */
inline float normalizedIsoIrisDiameter(const std::map<std::string, float>& scores) {
    auto it = scores.find("irisBiqtNormalizedIsoIrisDiameter");
    return (it != scores.end()) ? it->second : -1.0f;
}

// ---------------------------------------------------------------------------
// Quality assessment
// ---------------------------------------------------------------------------

/**
 * @brief Returns true if the scores meet the minimum quality threshold.
 *
 * @param scores    Score map produced by assess().
 * @param threshold Minimum acceptable irisBiqtIsoOverallQuality (default: 40.0).
 * @return          false if the map is empty or quality is below threshold.
 *
 * @code
 *   if (QualityAPI::isAcceptable(scores, 70.0f))
 *       std::cout << "Image is suitable for enrollment.\n";
 * @endcode
 */
inline bool isAcceptable(const std::map<std::string, float>& scores,
                          float threshold = 40.0f) {
    return overallQuality(scores) >= threshold;
}

/**
 * @brief Returns a human-readable quality label for the scores.
 *
 * Thresholds:
 *   - "GOOD"     - irisBiqtIsoOverallQuality >= 70
 *   - "MARGINAL" - irisBiqtIsoOverallQuality >= 40
 *   - "POOR"     - irisBiqtIsoOverallQuality <  40
 *   - "ERROR"    - score unavailable (assess() failed)
 *
 * @code
 *   std::cout << QualityAPI::qualityLabel(scores) << "\n";
 * @endcode
 */
inline std::string qualityLabel(const std::map<std::string, float>& scores) {
    float q = overallQuality(scores);
    if (q < 0.0f)  return "ERROR";
    if (q >= 70.0f) return "GOOD";
    if (q >= 40.0f) return "MARGINAL";
    return "POOR";
}

}
