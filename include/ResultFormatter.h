#pragma once

#include "IrisAnalyzer.h"
#include <string>
#include <vector>

class ResultFormatter {
public:
    // format a list of result objects as csv, json, or console
    static std::string format(const std::vector<IrisResult>& results,
                              const std::string& fmt);

private:
    static std::string formatCSV(const std::vector<IrisResult>& results);
    static std::string formatJSON(const std::vector<IrisResult>& results);
    static std::string formatTable(const std::vector<IrisResult>& results);

    static std::string escapeJSON(const std::string& s);
};
