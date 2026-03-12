#include "ResultFormatter.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>

static const std::vector<std::string> TABLE_METRICS = {
    "iso_overall_quality",
    "quality",
    "iso_sharpness",
    "iso_usable_iris_area",
    "iso_iris_pupil_contrast",
    "iso_iris_sclera_contrast",
    "iso_iris_pupil_ratio",
    "iso_margin_adequacy",
    "iso_greyscale_utilization",
    "iso_iris_pupil_concentricity",
    "iso_pupil_boundary_circularity",
    "contrast",
    "sharpness",
};

static const std::vector<std::string> TABLE_FEATURES = {
    "image_width",
    "image_height",
    "iris_diameter",
    "pupil_diameter",
    "iris_center_x",
    "iris_center_y",
};

std::string ResultFormatter::format(const std::vector<IrisResult>& results,
                                    const std::string& fmt) {
    if (fmt == "csv")  return formatCSV(results);
    if (fmt == "json") return formatJSON(results);
    return formatTable(results);
}

std::string ResultFormatter::formatCSV(const std::vector<IrisResult>& results) {
    std::ostringstream ss;
    ss << "Provider,Image,Detection,AttributeType,Key,Value\n";

    for (const auto& r : results) {
        if (r.hasError()) {
            ss << "BIQTIris," << r.imagePath << ",0,Error,message," << r.error << "\n";
            continue;
        }
        for (const auto& kv : r.metrics) {
            ss << "BIQTIris," << r.imagePath << ",1,Metric,"
               << kv.first << "," << kv.second << "\n";
        }
        for (const auto& kv : r.features) {
            ss << "BIQTIris," << r.imagePath << ",1,Feature,"
               << kv.first << "," << kv.second << "\n";
        }
    }
    return ss.str();
}

std::string ResultFormatter::formatJSON(const std::vector<IrisResult>& results) {
    std::ostringstream ss;
    ss << "[\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        ss << "  {\n";
        ss << "    \"image\": \"" << escapeJSON(r.imagePath) << "\",\n";

        if (r.hasError()) {
            ss << "    \"error\": \"" << escapeJSON(r.error) << "\"\n";
        } else {
            ss << "    \"metrics\": {\n";
            bool first = true;
            for (const auto& kv : r.metrics) {
                if (!first) ss << ",\n";
                ss << "      \"" << kv.first << "\": " << std::fixed
                   << std::setprecision(6) << kv.second;
                first = false;
            }
            ss << "\n    },\n";

            ss << "    \"features\": {\n";
            first = true;
            for (const auto& kv : r.features) {
                if (!first) ss << ",\n";
                ss << "      \"" << kv.first << "\": " << std::fixed
                   << std::setprecision(0) << kv.second;
                first = false;
            }
            ss << "\n    }\n";
        }

        ss << "  }";
        if (i + 1 < results.size()) ss << ",";
        ss << "\n";
    }

    ss << "]\n";
    return ss.str();
}

std::string ResultFormatter::formatTable(const std::vector<IrisResult>& results) {
    std::ostringstream ss;

    // each image's info print
    for (const auto& r : results) {
        ss << "Image: " << r.imagePath << "\n";

        if (r.hasError()) {
            ss << "  ERR: " << r.error << "\n\n";
            continue;
        }

        int n = 1;
        auto pm = [&](const std::string& key) {
            double v = r.getMetric(key);
            if (v >= 0)
                ss << "  " << n++ << " " << key << " - " << std::fixed << std::setprecision(2) << v << "\n";
        };
        auto pn = [&](const std::string& key) {
            double v = r.getMetric(key);
            if (v >= 0)
                ss << "  " << n++ << " " << key << " - " << std::fixed << std::setprecision(6) << v << "\n";
        };
        auto pf = [&](const std::string& key) {
            double v = r.getFeature(key);
            if (v >= 0)
                ss << "  " << n++ << " " << key << " - " << std::fixed << std::setprecision(0) << v << " px\n";
        };

        ss << "\n  Overall Quality\n";
        pm("iso_overall_quality");
        pm("quality");

        ss << "\n  ISO Metrics\n";
        pm("iso_sharpness");
        pm("iso_usable_iris_area");
        pm("iso_iris_pupil_contrast");
        pm("iso_iris_sclera_contrast");
        pm("iso_iris_pupil_ratio");
        pm("iso_iris_pupil_concentricity");
        pm("iso_margin_adequacy");
        pm("iso_greyscale_utilization");
        pm("iso_pupil_boundary_circularity");

        ss << "\n  Raw Metrics\n";
        pm("contrast");
        pm("sharpness");
        pm("iris_pupil_gs");
        pm("iris_sclera_gs");
        pm("pupil_circularity_avg_deviation");

        ss << "\n  Normalized Scores\n";
        pn("normalized_contrast");
        pn("normalized_sharpness");
        pn("normalized_iris_pupil_gs");
        pn("normalized_iris_sclera_gs");
        pn("normalized_iso_greyscale_utilization");
        pn("normalized_iso_iris_diameter");
        pn("normalized_iso_iris_pupil_concentricity");
        pn("normalized_iso_iris_pupil_contrast");
        pn("normalized_iso_iris_pupil_ratio");
        pn("normalized_iso_iris_sclera_contrast");
        pn("normalized_iso_margin_adequacy");
        pn("normalized_iso_sharpness");
        pn("normalized_iso_usable_iris_area");
        pn("normalized_iris_diameter");

        ss << "\n  Geometry\n";
        pf("image_width");
        pf("image_height");
        pf("iris_center_x");
        pf("iris_center_y");
        pf("iris_diameter");
        pf("pupil_center_x");
        pf("pupil_center_y");
        pf("pupil_diameter");
        pf("pupil_radius");

        ss << "\n";
    }

    // summary table for multiple images
    if (results.size() > 1) {
        ss << "Summary\n";
        ss << std::string(40, '-') << "\n";

        for (const auto& r : results) {
            if (r.hasError()) {
                ss << r.imagePath << "\n";
                ss << "  iso_overall_quality - N/A  [ERROR]\n";
            } else {
                double isoQ = r.getMetric("iso_overall_quality");
                std::string status = isoQ >= 70 ? "GOOD" : (isoQ >= 40 ? "MARGINAL" : "POOR");
                ss << r.imagePath << "\n";
                ss << "  iso_overall_quality - "
                << (isoQ >= 0 ? std::to_string((int)isoQ) : "N/A")
                << "  [" << status << "]\n";
            }
        }
    }

    return ss.str();
}

std::string ResultFormatter::escapeJSON(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    return out;
}
