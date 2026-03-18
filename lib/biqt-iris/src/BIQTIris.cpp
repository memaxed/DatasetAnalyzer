// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2022 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#include "BIQTIris.h"

BIQTIris::BIQTIris() {
    mfo.Initialize();
}
/**
 * @brief Computes iris quality metrics from a grayscale image using
 *        externally detected iris and pupil geometry.
 *
 * Iris and pupil detection is not performed internally. The coordinates
 * supplied in @p detectedIris are injected directly into all metric
 * computations, allowing the caller to use their own detector.
 *
 * Image requirements:
 *   - Type:   CV_8UC1 (8-bit single-channel grayscale). The underlying
 *             algorithm reads pixel data as a flat uint8_t array — passing
 *             any other type will produce incorrect results.
 *   - Size:   width 256–1000 px, height 256–680 px.
 *   - Layout: continuous memory (isContinuous() == true).
 *
 * All computed scores are inserted into @p qualityScores with the prefix
 * "irisBiqt". The primary composite score is stored under "irisBiqtQuality".
 * Uses insert() — existing keys are not overwritten, so pass a fresh map
 * for each frame.
 *
 * @param image          8-bpp grayscale image as a cv::Mat.
 * @param detectedIris   Externally computed iris/pupil centre and radius.
 * @param qualityScores  Output map; 32 scores are inserted on success.
 *                       Must not be nullptr.
 * @return  0 on success.
 *          1 if the image is empty, not CV_8UC1, not continuous,
 *            or smaller than 256x256 px.
 *          2 if the image exceeds 1000x680 px.
 *          3 if an unexpected exception occurred.
 */
int32_t BIQTIris::assessQuality(const cv::Mat &image,
                                const DetectedIris &detectedIris,
                                std::map<std::string, float> *qualityScores) {
    if (image.empty() || image.data == nullptr || !image.isContinuous()) {
        std::cerr << "BIQTIris::assessQuality: image is empty or not continuous" << std::endl;
        return 1;
    }
    if (image.type() != CV_8UC1) {
        std::cerr << "BIQTIris::assessQuality: image must be CV_8UC1 (8-bit grayscale)" << std::endl;
        return 1;
    }
    if (image.cols < mfo.min_width_ || image.rows < mfo.min_height_) {
        std::cerr << "BIQTIris::assessQuality: image is too small ("
                  << image.cols << "x" << image.rows << ")" << std::endl;
        return 1;
    }
    if (image.cols > mfo.max_width_ || image.rows > mfo.max_height_) {
        std::cerr << "BIQTIris::assessQuality: image is too large ("
                  << image.cols << "x" << image.rows << ")" << std::endl;
        return 2;
    }

    try {
        const uint8_t *frameBytes = image.data;
        const int width  = image.cols;
        const int height = image.rows;

        double biqtQuality = mfo.GetQualityFromImageFrame(
            frameBytes, width, height,
            detectedIris.irisX, detectedIris.irisY, detectedIris.irisRadius,
            detectedIris.pupilX, detectedIris.pupilY, detectedIris.pupilRadius);

        qualityScores->insert({"irisBiqtQuality", static_cast<float>(biqtQuality)});

        qualityScores->insert({"irisBiqtIsoOverallQuality", static_cast<float>(mfo.GetIsoOverallQuality())});

        qualityScores->insert({"irisBiqtContrast", static_cast<float>(mfo.GetContrastScore())});
        qualityScores->insert({"irisBiqtSharpness", static_cast<float>(mfo.GetDefocusScore())});
        qualityScores->insert({"irisBiqtIrisScleraGs", static_cast<float>(mfo.GetISGSDiffMeanAvg())});
        qualityScores->insert({"irisBiqtIrisPupilGs", static_cast<float>(mfo.GetIrisPupilGSDiff())});
        qualityScores->insert({"irisBiqtPupilCircularityAvgDeviation", static_cast<float>(mfo.GetPupilCircularityDeviationAvg())});

        qualityScores->insert({"irisBiqtNormalizedContrast", static_cast<float>(mfo.GetNContrast())});
        qualityScores->insert({"irisBiqtNormalizedSharpness", static_cast<float>(mfo.GetNDefocus())});
        qualityScores->insert({"irisBiqtNormalizedIrisDiameter", static_cast<float>(mfo.GetNIrisID())});
        qualityScores->insert({"irisBiqtNormalizedIrisScleraGs", static_cast<float>(mfo.GetNISGSMean())});
        qualityScores->insert({"irisBiqtNormalizedIrisPupilGs", static_cast<float>(mfo.GetNIPGSDiff())});
        qualityScores->insert({"irisBiqtNormalizedIsoUsableIrisArea", static_cast<float>(mfo.GetNIrisVis())});

        qualityScores->insert({"irisBiqtIsoUsableIrisArea", static_cast<float>(mfo.GetUsableIrisAreaPercent())});
        qualityScores->insert({"irisBiqtIsoIrisScleraContrast", static_cast<float>(mfo.GetISOIrisScleraContrast())});
        qualityScores->insert({"irisBiqtIsoIrisPupilContrast", static_cast<float>(mfo.GetISOIrisPupilContrast())});
        qualityScores->insert({"irisBiqtIsoPupilBoundaryCircularity", static_cast<float>(mfo.GetISOPupilBoundaryCircularity())});
        qualityScores->insert({"irisBiqtIsoGreyscaleUtilization", static_cast<float>(mfo.GetISOGreyscaleUtilization())});
        qualityScores->insert({"irisBiqtIsoIrisPupilRatio", static_cast<float>(mfo.GetISOPIRatio())});
        qualityScores->insert({"irisBiqtIsoIrisPupilConcentricity", static_cast<float>(mfo.GetISOIPConcentricity())});
        qualityScores->insert({"irisBiqtIsoMarginAdequacy", static_cast<float>(mfo.GetISOMarginAdequacy())});
        qualityScores->insert({"irisBiqtIsoSharpness", static_cast<float>(mfo.GetISOSharpness())});

        qualityScores->insert({"irisBiqtNormalizedIsoSharpness", static_cast<float>(mfo.GetNormalizedISOSharpness())});
        qualityScores->insert({"irisBiqtNormalizedIsoIrisPupilRatio", static_cast<float>(mfo.GetNormalizedISOPIRatio())});
        qualityScores->insert({"irisBiqtNormalizedIsoIrisPupilContrast", static_cast<float>(mfo.GetNormalizedISOIrisPupilContrast())});
        qualityScores->insert({"irisBiqtNormalizedIsoIrisScleraContrast", static_cast<float>(mfo.GetNormalizedISOIrisScleraContrast())});
        qualityScores->insert({"irisBiqtNormalizedIsoMarginAdequacy", static_cast<float>(mfo.GetNormalizedISOMarginAdequacy())});
        qualityScores->insert({"irisBiqtNormalizedIsoGreyscaleUtilization", static_cast<float>(mfo.GetNormalizedISOGreyscaleUtilization())});
        qualityScores->insert({"irisBiqtNormalizedIsoIrisPupilConcentricity", static_cast<float>(mfo.GetNormalizedISOIPConcentricity())});
        qualityScores->insert({"irisBiqtNormalizedIsoIrisDiameter", static_cast<float>(mfo.GetNormalizedISOIrisDiameter())});

        return 0;
    }
    catch (std::exception &ex) {
        std::cerr << "BIQTIris::assessQuality exception: " << ex.what() << std::endl;
        return 3;
    }
}
