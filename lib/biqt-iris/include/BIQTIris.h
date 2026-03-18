// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2022 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#ifndef BIQTIRIS_H
#define BIQTIRIS_H

#include "ImageOps.h"
#include "DetectedIris.h"
#include "opencv2/core.hpp"
#include <cstdint>
#include <map>
#include <string>

class BIQTIris {

private:
  MFilter mfo;

public:
  BIQTIris();

  int32_t assessQuality(const cv::Mat &image,
                        const DetectedIris &detectedIris,
                        std::map<std::string, float> *qualityScores);
};

#endif
