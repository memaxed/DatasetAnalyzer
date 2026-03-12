#pragma once

/**
 * @file IrisQualityAPI.h
 * @brief Public API for iris image quality assessment using the embedded BIQTIris engine.
 *
 * This is the only header you need to include. All functionality is exposed
 * through the QualityAPI namespace.
 *
 * Basic usage:
 * @code
 *   #include "IrisQualityAPI.h"
 *
 *   // Analyze a single image
 *   IrisResult result = QualityAPI::analyze("iris.png");
 *   std::cout << QualityAPI::qualityLabel(result);       // "GOOD"
 *   std::cout << QualityAPI::overallQuality(result);     // 83.0
 *
 *   // Analyze a dataset and export
 *   auto results = QualityAPI::analyzeDirectory("./dataset", true);
 *   auto good    = QualityAPI::filterByQuality(results, 70.0);
 *   std::string csv = QualityAPI::toCSV(good);
 * @endcode
 */

#include "IrisAnalyzer.h"
#include "PathUtils.h"
#include "ResultFormatter.h"

#include <string>
#include <vector>
#include <stdexcept>

namespace QualityAPI {

// ---------------------------------------------------------------------------
// Analysis
// ---------------------------------------------------------------------------

/**
 * @brief Analyzes a single iris image.
 *
 * @param imagePath Path to a supported image file (PNG, JPG, BMP, TIFF).
 *                  Image must be 256x256–1000x680 px, 8bpp grayscale.
 * @return IrisResult containing quality metrics and geometry features.
 *         Check IrisResult::hasError() before accessing metrics.
 *
 * @code
 *   IrisResult r = QualityAPI::analyze("subject_01_left.png");
 *   if (!r.hasError())
 *       std::cout << QualityAPI::overallQuality(r) << "\n";
 * @endcode
 */
inline IrisResult analyze(const std::string& imagePath) {
    IrisAnalyzer analyzer;
    analyzer.initialize();
    return analyzer.analyze(imagePath);
}

/**
 * @brief Analyzes all supported images in a directory.
 *
 * @param dirPath   Path to a directory containing iris images.
 * @param recursive If true, subdirectories are scanned recursively.
 * @return Vector of IrisResult, one per image, sorted alphabetically by path.
 *
 * @code
 *   auto results = QualityAPI::analyzeDirectory("./dataset", true);
 *   for (const auto& r : results)
 *       std::cout << r.imagePath << ": " << QualityAPI::qualityLabel(r) << "\n";
 * @endcode
 */
inline std::vector<IrisResult> analyzeDirectory(const std::string& dirPath,
                                                 bool recursive = false) {
    IrisAnalyzer analyzer;
    analyzer.initialize();

    std::vector<IrisResult> results;
    for (const auto& path : PathUtils::collectImages(dirPath, recursive)) {
        results.push_back(analyzer.analyze(path));
    }
    return results;
}

/**
 * @brief Analyzes an explicit list of image paths.
 *
 * Useful when you already have a pre-filtered or manually assembled file list.
 *
 * @param paths Vector of absolute or relative image file paths.
 * @return Vector of IrisResult in the same order as the input paths.
 *
 * @code
 *   auto results = QualityAPI::analyzeAll({ "left.png", "right.png" });
 * @endcode
 */
inline std::vector<IrisResult> analyzeAll(const std::vector<std::string>& paths) {
    IrisAnalyzer analyzer;
    analyzer.initialize();

    std::vector<IrisResult> results;
    results.reserve(paths.size());
    for (const auto& path : paths) {
        results.push_back(analyzer.analyze(path));
    }
    return results;
}

// ---------------------------------------------------------------------------
// Overall quality
// ---------------------------------------------------------------------------

/**
 * @brief Returns the ISO overall quality score (0–100).
 *
 * The primary composite quality metric. Derived from all ISO sub-scores.
 * Higher is better. Returns -1.0 if unavailable.
 */
inline double overallQuality(const IrisResult& r) {
    return r.getMetric("iso_overall_quality");
}

/**
 * @brief Returns the raw overall quality score (0–100).
 *
 * Non-ISO composite score computed directly by the BIQTIris algorithm,
 * before ISO normalization. Returns -1.0 if unavailable.
 */
inline double rawQuality(const IrisResult& r) {
    return r.getMetric("quality");
}

// ---------------------------------------------------------------------------
// ISO metrics
// ---------------------------------------------------------------------------

/**
 * @brief Returns the ISO sharpness score (0–100).
 *
 * Measures focus/defocus of the iris texture. Low values indicate
 * motion blur or out-of-focus capture. Returns -1.0 if unavailable.
 */
inline double isoSharpness(const IrisResult& r) {
    return r.getMetric("iso_sharpness");
}

/**
 * @brief Returns the ISO usable iris area percentage (0–100).
 *
 * Fraction of the iris region not occluded by eyelids or eyelashes.
 * Low values indicate heavy occlusion. Returns -1.0 if unavailable.
 */
inline double isoUsableIrisArea(const IrisResult& r) {
    return r.getMetric("iso_usable_iris_area");
}

/**
 * @brief Returns the ISO iris-pupil contrast score (0–100).
 *
 * Greyscale contrast between the iris and pupil regions.
 * Low values may indicate poor lighting or pigmentation issues.
 * Returns -1.0 if unavailable.
 */
inline double isoIrisPupilContrast(const IrisResult& r) {
    return r.getMetric("iso_iris_pupil_contrast");
}

/**
 * @brief Returns the ISO iris-sclera contrast score (0–100).
 *
 * Greyscale contrast between the iris and the sclera (white of the eye).
 * Low values indicate difficulty segmenting the iris boundary.
 * Returns -1.0 if unavailable.
 */
inline double isoIrisScleraContrast(const IrisResult& r) {
    return r.getMetric("iso_iris_sclera_contrast");
}

/**
 * @brief Returns the ISO iris-pupil ratio score (0–100).
 *
 * Ratio of pupil diameter to iris diameter. Extreme dilation or
 * constriction reduces this score. Returns -1.0 if unavailable.
 */
inline double isoIrisPupilRatio(const IrisResult& r) {
    return r.getMetric("iso_iris_pupil_ratio");
}

/**
 * @brief Returns the ISO iris-pupil concentricity score (0–100).
 *
 * Measures how well the pupil center aligns with the iris center.
 * Low values indicate significant decentration. Returns -1.0 if unavailable.
 */
inline double isoIrisPupilConcentricity(const IrisResult& r) {
    return r.getMetric("iso_iris_pupil_concentricity");
}

/**
 * @brief Returns the ISO margin adequacy score (0–100).
 *
 * Measures whether sufficient iris margin is visible around the pupil.
 * Low values indicate the iris is too close to the image border.
 * Returns -1.0 if unavailable.
 */
inline double isoMarginAdequacy(const IrisResult& r) {
    return r.getMetric("iso_margin_adequacy");
}

/**
 * @brief Returns the ISO greyscale utilization score (0–100).
 *
 * Measures how well the image uses the available greyscale dynamic range.
 * Low values indicate over- or under-exposure. Returns -1.0 if unavailable.
 */
inline double isoGreyscaleUtilization(const IrisResult& r) {
    return r.getMetric("iso_greyscale_utilization");
}

/**
 * @brief Returns the ISO pupil boundary circularity score (0–100).
 *
 * Measures how closely the pupil boundary approximates a circle.
 * Low values may indicate occlusion or segmentation errors.
 * Returns -1.0 if unavailable.
 */
inline double isoPupilBoundaryCircularity(const IrisResult& r) {
    return r.getMetric("iso_pupil_boundary_circularity");
}

// ---------------------------------------------------------------------------
// Raw metrics
// ---------------------------------------------------------------------------

/**
 * @brief Returns the raw contrast score (integer range).
 *
 * Direct contrast measurement before ISO normalization.
 * Returns -1.0 if unavailable.
 */
inline double contrast(const IrisResult& r) {
    return r.getMetric("contrast");
}

/**
 * @brief Returns the raw sharpness / defocus score (integer range).
 *
 * Direct defocus measurement before ISO normalization.
 * Returns -1.0 if unavailable.
 */
inline double sharpness(const IrisResult& r) {
    return r.getMetric("sharpness");
}

/**
 * @brief Returns the raw iris-sclera greyscale difference mean (float).
 *
 * Average difference in greyscale intensity between iris and sclera segments.
 * Returns -1.0 if unavailable.
 */
inline double irisScleraGS(const IrisResult& r) {
    return r.getMetric("iris_sclera_gs");
}

/**
 * @brief Returns the raw iris-pupil greyscale difference (float).
 *
 * Greyscale intensity difference between the iris and pupil regions.
 * Returns -1.0 if unavailable.
 */
inline double irisPupilGS(const IrisResult& r) {
    return r.getMetric("iris_pupil_gs");
}

/**
 * @brief Returns the average pupil circularity deviation (float).
 *
 * Mean deviation of the pupil boundary from a perfect circle, in pixels.
 * Returns -1.0 if unavailable.
 */
inline double pupilCircularityAvgDeviation(const IrisResult& r) {
    return r.getMetric("pupil_circularity_avg_deviation");
}

// ---------------------------------------------------------------------------
// Normalized scores
// ---------------------------------------------------------------------------

/**
 * @brief Returns the normalized contrast score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedContrast(const IrisResult& r) {
    return r.getMetric("normalized_contrast");
}

/**
 * @brief Returns the normalized sharpness score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedSharpness(const IrisResult& r) {
    return r.getMetric("normalized_sharpness");
}

/**
 * @brief Returns the normalized iris diameter score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIrisDiameter(const IrisResult& r) {
    return r.getMetric("normalized_iris_diameter");
}

/**
 * @brief Returns the normalized iris-sclera greyscale score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIrisScleraGS(const IrisResult& r) {
    return r.getMetric("normalized_iris_sclera_gs");
}

/**
 * @brief Returns the normalized iris-pupil greyscale score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIrisPupilGS(const IrisResult& r) {
    return r.getMetric("normalized_iris_pupil_gs");
}

/**
 * @brief Returns the normalized ISO usable iris area score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoUsableIrisArea(const IrisResult& r) {
    return r.getMetric("normalized_iso_usable_iris_area");
}

/**
 * @brief Returns the normalized ISO sharpness score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoSharpness(const IrisResult& r) {
    return r.getMetric("normalized_iso_sharpness");
}

/**
 * @brief Returns the normalized ISO iris-pupil ratio score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoIrisPupilRatio(const IrisResult& r) {
    return r.getMetric("normalized_iso_iris_pupil_ratio");
}

/**
 * @brief Returns the normalized ISO iris-pupil contrast score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoIrisPupilContrast(const IrisResult& r) {
    return r.getMetric("normalized_iso_iris_pupil_contrast");
}

/**
 * @brief Returns the normalized ISO iris-sclera contrast score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoIrisScleraContrast(const IrisResult& r) {
    return r.getMetric("normalized_iso_iris_sclera_contrast");
}

/**
 * @brief Returns the normalized ISO margin adequacy score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoMarginAdequacy(const IrisResult& r) {
    return r.getMetric("normalized_iso_margin_adequacy");
}

/**
 * @brief Returns the normalized ISO greyscale utilization score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoGreyscaleUtilization(const IrisResult& r) {
    return r.getMetric("normalized_iso_greyscale_utilization");
}

/**
 * @brief Returns the normalized ISO iris-pupil concentricity score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoIrisPupilConcentricity(const IrisResult& r) {
    return r.getMetric("normalized_iso_iris_pupil_concentricity");
}

/**
 * @brief Returns the normalized ISO iris diameter score (0.0–1.0).
 * Returns -1.0 if unavailable.
 */
inline double normalizedIsoIrisDiameter(const IrisResult& r) {
    return r.getMetric("normalized_iso_iris_diameter");
}

// ---------------------------------------------------------------------------
// Geometry features
// ---------------------------------------------------------------------------

/**
 * @brief Returns the image width in pixels.
 * Returns -1.0 if unavailable.
 */
inline double imageWidth(const IrisResult& r) {
    return r.getFeature("image_width");
}

/**
 * @brief Returns the image height in pixels.
 * Returns -1.0 if unavailable.
 */
inline double imageHeight(const IrisResult& r) {
    return r.getFeature("image_height");
}

/**
 * @brief Returns the X coordinate of the iris center in pixels.
 * Returns -1.0 if unavailable.
 */
inline double irisCenterX(const IrisResult& r) {
    return r.getFeature("iris_center_x");
}

/**
 * @brief Returns the Y coordinate of the iris center in pixels.
 * Returns -1.0 if unavailable.
 */
inline double irisCenterY(const IrisResult& r) {
    return r.getFeature("iris_center_y");
}

/**
 * @brief Returns the iris diameter in pixels.
 * Returns -1.0 if unavailable.
 */
inline double irisDiameter(const IrisResult& r) {
    return r.getFeature("iris_diameter");
}

/**
 * @brief Returns the X coordinate of the pupil center in pixels.
 * Returns -1.0 if unavailable.
 */
inline double pupilCenterX(const IrisResult& r) {
    return r.getFeature("pupil_center_x");
}

/**
 * @brief Returns the Y coordinate of the pupil center in pixels.
 * Returns -1.0 if unavailable.
 */
inline double pupilCenterY(const IrisResult& r) {
    return r.getFeature("pupil_center_y");
}

/**
 * @brief Returns the pupil diameter in pixels.
 * Returns -1.0 if unavailable.
 */
inline double pupilDiameter(const IrisResult& r) {
    return r.getFeature("pupil_diameter");
}

/**
 * @brief Returns the pupil radius in pixels.
 * Returns -1.0 if unavailable.
 */
inline double pupilRadius(const IrisResult& r) {
    return r.getFeature("pupil_radius");
}

// ---------------------------------------------------------------------------
// Quality assessment
// ---------------------------------------------------------------------------

/**
 * @brief Returns true if the image meets the minimum quality threshold.
 *
 * @param r         The result to evaluate.
 * @param threshold Minimum acceptable iso_overall_quality (default: 40.0).
 * @return false if the result contains an error or quality is below threshold.
 *
 * @code
 *   if (QualityAPI::isAcceptable(result, 70.0))
 *       std::cout << "Image is suitable for enrollment.\n";
 * @endcode
 */
inline bool isAcceptable(const IrisResult& r, double threshold = 40.0) {
    if (r.hasError()) return false;
    return overallQuality(r) >= threshold;
}

/**
 * @brief Returns a human-readable quality label for the result.
 *
 * Thresholds:
 *   - "GOOD"     - iso_overall_quality >= 70
 *   - "MARGINAL" - iso_overall_quality >= 40
 *   - "POOR"     - iso_overall_quality <  40
 *   - "ERROR"    - processing failed
 *
 * @code
 *   std::cout << r.imagePath << ": " << QualityAPI::qualityLabel(r) << "\n";
 * @endcode
 */
inline std::string qualityLabel(const IrisResult& r) {
    if (r.hasError()) return "ERROR";
    double q = overallQuality(r);
    if (q >= 70) return "GOOD";
    if (q >= 40) return "MARGINAL";
    return "POOR";
}

// ---------------------------------------------------------------------------
// Filtering
// ---------------------------------------------------------------------------

/**
 * @brief Filters out results that contain errors.
 *
 * @param results Input result vector.
 * @return New vector containing only successfully processed results.
 *
 * @code
 *   auto valid = QualityAPI::filterValid(results);
 *   std::cout << valid.size() << " images processed successfully.\n";
 * @endcode
 */
inline std::vector<IrisResult> filterValid(const std::vector<IrisResult>& results) {
    std::vector<IrisResult> out;
    for (const auto& r : results) {
        if (!r.hasError()) out.push_back(r);
    }
    return out;
}

/**
 * @brief Filters results by minimum overall quality threshold.
 *
 * @param results   Input result vector.
 * @param threshold Minimum iso_overall_quality to include (default: 40.0).
 * @return New vector containing only results that meet the threshold.
 *
 * @code
 *   auto enrollable = QualityAPI::filterByQuality(results, 70.0);
 * @endcode
 */
inline std::vector<IrisResult> filterByQuality(const std::vector<IrisResult>& results,
                                                double threshold = 40.0) {
    std::vector<IrisResult> out;
    for (const auto& r : results) {
        if (isAcceptable(r, threshold)) out.push_back(r);
    }
    return out;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

/**
 * @brief Formats results as a human-readable console table.
 *
 * Includes all ISO metrics, raw scores, normalized scores, and geometry.
 * Best suited for interactive use or debugging.
 */
inline std::string toTable(const std::vector<IrisResult>& results) {
    return ResultFormatter::format(results, "table");
}

/**
 * @brief Formats results as CSV.
 *
 * Columns: Provider, Image, Detection, AttributeType, Key, Value.
 * Suitable for importing into spreadsheets or analysis tools.
 */
inline std::string toCSV(const std::vector<IrisResult>& results) {
    return ResultFormatter::format(results, "csv");
}

/**
 * @brief Formats results as a JSON array.
 *
 * Each element contains "image", "metrics", and "features" fields.
 * Suitable for REST APIs or downstream processing.
 */
inline std::string toJSON(const std::vector<IrisResult>& results) {
    return ResultFormatter::format(results, "json");
}

}