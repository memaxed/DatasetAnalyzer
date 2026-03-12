// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#ifndef PROVIDERINTERFACE_H
#define PROVIDERINTERFACE_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/writer.h>
#include <map>
#include <vector>

class Provider {

  public:
    struct QualityResult {
        std::map<std::string, double> metrics;
        std::map<std::string, double> features;
    };

    struct EvaluationResult {
        int errorCode;
        std::string provider;
        std::string message;
        std::vector<QualityResult> qualityResult;
    };

    virtual ~Provider() = default;

    virtual std::string name()        { return DescriptorObject["name"].asString(); }
    virtual std::string version()     { return DescriptorObject["version"].asString(); }
    virtual std::string description() { return DescriptorObject["description"].asString(); }
    virtual std::string modality()    { return DescriptorObject["modality"].asString(); }

    virtual std::vector<std::string> attributes() {
        Json::Value attrs = DescriptorObject["attributes"];
        std::vector<std::string> attr_list;
        for (const auto& attr : attrs)
            attr_list.push_back(attr["name"].asString());
        return attr_list;
    }

    virtual std::string describeAttribute(const std::string& attribute) {
        Json::Value attrs = DescriptorObject["attributes"];
        for (const auto& attr : attrs)
            if (attr["name"].asString() == attribute)
                return attr["description"].asString();
        return "This module does not support the specified attribute.";
    }

    virtual EvaluationResult evaluate(const std::string& file) = 0;

    static EvaluationResult deserializeResult(const char* result_str) {
        EvaluationResult result;
        Json::Value result_json;
        Json::Reader reader;
        if (!reader.parse(result_str, result_json))
            std::cout << "Failed to parse" << reader.getFormattedErrorMessages();

        result.errorCode = result_json["errorCode"].asInt();
        result.provider  = result_json["provider"].asString();
        result.message   = result_json["message"].asString();

        for (const auto& qualityResult : result_json["qualityResult"]) {
            Provider::QualityResult quality_result;
            for (const auto& metric : qualityResult["metrics"].getMemberNames())
                quality_result.metrics[metric] = qualityResult["metrics"][metric].asFloat();
            for (const auto& feature : qualityResult["features"].getMemberNames())
                quality_result.features[feature] = qualityResult["features"][feature].asFloat();
            result.qualityResult.push_back(std::move(quality_result));
        }
        return result;
    }

    static char* serializeResult(const Provider::EvaluationResult& result) {
        Json::Value result_json;
        result_json["errorCode"] = result.errorCode;
        result_json["provider"]  = result.provider;
        result_json["message"]   = result.message;

        Json::Value vec(Json::arrayValue);
        for (const auto& qualityResult : result.qualityResult) {
            Json::Value quality_result;
            Json::Value metrics_map;
            Json::Value features_map;
            for (const auto& metric  : qualityResult.metrics)  metrics_map[metric.first]  = metric.second;
            for (const auto& feature : qualityResult.features) features_map[feature.first] = feature.second;
            quality_result["metrics"]  = std::move(metrics_map);
            quality_result["features"] = std::move(features_map);
            vec.append(std::move(quality_result));
        }
        result_json["qualityResult"] = std::move(vec);

        std::string result_str = result_json.toStyledString();
        char* result_cstr = new char[result_str.length() + 1];
        strncpy(result_cstr, result_str.c_str(), result_str.length() + 1);
        return result_cstr;
    }

  protected:
    Json::Value DescriptorObject;
};

#endif