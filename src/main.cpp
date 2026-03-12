#include "IrisQualityAPI.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

struct Config {
    std::string inputPath;
    std::string outputFormat = "table";
    std::string outFile;
    bool recursive = false;
    bool showHelp  = false;
};

static void printHelp(const char* prog) {
    std::cout
        << "Usage: " << prog << " --path <image_or_dir> [options]\n\n"
        << "Options:\n"
        << "  --path <path>              Image file or directory to analyze\n"
        << "  --output <csv|json|table>  Output format (default: table)\n"
        << "  --out-file <file>          Write output to file\n"
        << "  --recursive                Scan subdirectories recursively\n"
        << "  --help                     Show this help message\n\n"
        << "Examples:\n"
        << "  datasetanalyzer --path iris_0001.png\n"
        << "  datasetanalyzer --path ./dataset --recursive --output csv\n"
        << "  datasetanalyzer --path ./dataset --output json --out-file results.json\n\n"
        << "Supported formats: PNG, JPG/JPEG, BMP, TIFF\n"
        << "Image constraints: 256x256 – 1000x680 px, 8bpp\n";
}

static Config parseArgs(int argc, char** argv) {
    Config cfg;
    if (argc < 2) { cfg.showHelp = true; return cfg; }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if      (arg == "--help" || arg == "-h")         cfg.showHelp = true;
        else if (arg == "--path"      && i+1 < argc)     cfg.inputPath     = argv[++i];
        else if (arg == "--output"    && i+1 < argc)     cfg.outputFormat  = argv[++i];
        else if (arg == "--out-file"  && i+1 < argc)     cfg.outFile       = argv[++i];
        else if (arg == "--recursive")                   cfg.recursive     = true;
        else { std::cerr << "Unknown argument: " << arg << "\n"; cfg.showHelp = true; }
    }
    return cfg;
}

int main(int argc, char** argv) {
    Config cfg = parseArgs(argc, argv);

    if (cfg.showHelp)    { printHelp(argv[0]); return 0; }
    if (cfg.inputPath.empty()) {
        std::cerr << "Error: --path is required.\n";
        printHelp(argv[0]);
        return 1;
    }
    if (cfg.outputFormat != "csv" && cfg.outputFormat != "json" && cfg.outputFormat != "table") {
        std::cerr << "Error: --output must be one of: csv, json, table\n";
        return 1;
    }

    // collect images
    std::vector<std::string> images;
    try {
        images = PathUtils::collectImages(cfg.inputPath, cfg.recursive);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    if (images.empty()) {
        std::cerr << "No supported images found at: " << cfg.inputPath << "\n";
        return 1;
    }

    std::cout << "Found " << images.size() << " image(s) to analyze.\n\n";

    // analyze — one by one so we can show progress
    std::vector<IrisResult> results;
    int failed = 0;

    for (const auto& path : images) {
        std::cout << "Analyzing: " << path << " ... " << std::flush;

        auto t0 = std::chrono::steady_clock::now();
        IrisResult r = QualityAPI::analyze(path);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0).count();

        if (r.hasError()) {
            std::cerr << "FAILED: " << r.error << "\n";
            ++failed;
        } else {
            std::cout << "OK  quality=" << QualityAPI::overallQuality(r)
                      << "  [" << QualityAPI::qualityLabel(r) << "]"
                      << "  " << ms << "ms\n";
        }
        results.push_back(std::move(r));
    }

    // final output
    std::string output;
    if(cfg.outputFormat == "csv")  output = QualityAPI::toCSV(results);
    else if(cfg.outputFormat == "json") output = QualityAPI::toJSON(results);
    else output = QualityAPI::toTable(results);

    if (!cfg.outFile.empty()) {
        std::ofstream ofs(cfg.outFile);
        if (!ofs) { std::cerr << "Error: cannot write to: " << cfg.outFile << "\n"; return 1; }
        ofs << output;
        std::cout << "\nResults written to: " << cfg.outFile << "\n";
    } else {
        std::cout << "\n" << output << "\n";
    }

    std::cout << "Summary: " << (results.size() - failed) << "/" << results.size()
              << " images analyzed successfully.\n";

    return (failed == 0) ? 0 : 2;
}
