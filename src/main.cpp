#include "IrisAnalyzer.h"
#include "ResultFormatter.h"
#include "PathUtils.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <fstream>
#include <chrono>

struct Config {
    std::string inputPath;
    std::string providerPath;
    std::string outputFormat = "table";
    std::string outFile;
    bool recursive = false;
    bool showHelp = false;
};

static void printHelp(const char* progName) {
    std::cout << "Usage: " << progName << " --path <image_or_dir> [options]\n\n"
              << "Options:\n"
              << "  --path <path>              Image file or directory to analyze\n"
              << "  --provider <path>          Path to BIQTIris shared library\n"
              << "                             (auto-detected if not specified)\n"
              << "  --output <csv|json|table>  Output format (default: table)\n"
              << "  --out-file <file>          Write output to file\n"
              << "  --recursive                Scan subdirectories recursively\n"
              << "  --help                     Show this help message\n\n"
              << "Examples:\n"
              << "  datasetanalyzer --path iris_0001.png\n"
              << "  datasetanalyzer --path ./dataset --recursive --output csv\n"
              << "  datasetanalyzer --path ./dataset --output json --out-file results.json\n\n"
              << "Supported image formats: PNG, JPG/JPEG, BMP, TIFF\n"
              << "Image constraints (BIQTIris): 256x256 to 1000x680 pixels, 8bpp\n";
}

static Config parseArgs(int argc, char** argv) {
    Config cfg;
    if (argc < 2) {
        cfg.showHelp = true;
        return cfg;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            cfg.showHelp = true;
        } else if (arg == "--path" && i + 1 < argc) {
            cfg.inputPath = argv[++i];
        } else if (arg == "--provider" && i + 1 < argc) {
            cfg.providerPath = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            cfg.outputFormat = argv[++i];
        } else if (arg == "--out-file" && i + 1 < argc) {
            cfg.outFile = argv[++i];
        } else if (arg == "--recursive") {
            cfg.recursive = true;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            cfg.showHelp = true;
        }
    }

    return cfg;
}

int main(int argc, char** argv) {
    Config cfg = parseArgs(argc, argv);

    if (cfg.showHelp) {
        printHelp(argv[0]);
        return 0;
    }

    if (cfg.inputPath.empty()) {
        std::cerr << "Error: --path is required.\n";
        printHelp(argv[0]);
        return 1;
    }

    if (cfg.outputFormat != "csv" && cfg.outputFormat != "json" && cfg.outputFormat != "table") {
        std::cerr << "Error: --output must be one of: csv, json, table\n";
        return 1;
    }

    // collect image files
    std::vector<std::string> images;
    try {
        images = PathUtils::collectImages(cfg.inputPath, cfg.recursive);
    } catch (const std::exception& e) {
        std::cerr << "Error collecting images: " << e.what() << "\n";
        return 1;
    }

    if (images.empty()) {
        std::cerr << "No supported images found at: " << cfg.inputPath << "\n";
        return 1;
    }

    std::cout << "Found " << images.size() << " image(s) to analyze.\n\n";

    // load iris provider
    IrisAnalyzer analyzer;
    try {
        analyzer.initialize(cfg.providerPath);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize BIQTIris provider: " << e.what() << "\n";
        std::cerr << "\nMake sure BIQTIris is installed. You can:\n"
                  << "  1. Install BIQT framework from https://github.com/mitre/biqt\n"
                  << "  2. Specify library path with --provider <path/to/BIQTIris.dll or .so>\n";
        return 1;
    }

    // analyze each image
    std::vector<IrisResult> results;
    int failed = 0;
    for (const auto& imgPath : images) {
        std::cout << "Analyzing: " << imgPath << " ... " << std::flush;
        try {
            auto t0 = std::chrono::steady_clock::now();
            IrisResult result = analyzer.analyze(imgPath);
            auto t1 = std::chrono::steady_clock::now();
            long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

            results.push_back(result);
            std::cout << "OK (quality=" << result.getMetric("iso_overall_quality")
                      << ", " << ms << "ms)\n";
        } catch (const std::exception& e) {
            std::cerr << "FAILED: " << e.what() << "\n";
            IrisResult errResult;
            errResult.imagePath = imgPath;
            errResult.error = e.what();
            results.push_back(errResult);
            ++failed;
        }
    }
    
    std::string output = ResultFormatter::format(results, cfg.outputFormat);

    if (!cfg.outFile.empty()) {
        std::ofstream ofs(cfg.outFile);
        if (!ofs) {
            std::cerr << "Error: Cannot write to file: " << cfg.outFile << "\n";
            return 1;
        }
        ofs << output;
        std::cout << "\nResults written to: " << cfg.outFile << "\n";
    } else {
        std::cout << "\n" << output << "\n";
    }

    std::cout << "\nSummary: " << (results.size() - failed) << "/" << results.size()
              << " images analyzed successfully.\n";

    return (failed == 0) ? 0 : 2;
}
