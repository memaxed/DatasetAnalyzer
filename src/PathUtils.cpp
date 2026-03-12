#include "PathUtils.h"

#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

static const std::vector<std::string> SUPPORTED_EXTS = {
    ".png", ".jpg", ".jpeg", ".bmp", ".tiff", ".tif"
};

bool PathUtils::isFile(const std::string& path) {
    return fs::is_regular_file(path);
}

bool PathUtils::isDirectory(const std::string& path) {
    return fs::is_directory(path);
}

bool PathUtils::isSupportedImage(const std::string& path) {
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    for (const auto& e : SUPPORTED_EXTS) {
        if (ext == e) return true;
    }
    return false;
}

static void scanDir(const fs::path& dir, bool recursive,
                    std::vector<std::string>& out) {
    auto scan = [&](auto& iter) {
        for (const auto& entry : iter) {
            if (entry.is_regular_file() && PathUtils::isSupportedImage(entry.path().string())) {
                out.push_back(entry.path().string());
            }
        }
    };

    if (recursive) {
        auto iter = fs::recursive_directory_iterator(dir);
        scan(iter);
    } else {
        auto iter = fs::directory_iterator(dir);
        scan(iter);
    }
}

std::vector<std::string> PathUtils::collectImages(const std::string& root,
                                                   bool recursive) {
    std::vector<std::string> result;

    if (isFile(root)) {
        if (isSupportedImage(root)) {
            result.push_back(root);
        } else {
            throw std::runtime_error("File is not a supported image format: " + root);
        }
    } else if (isDirectory(root)) {
        scanDir(root, recursive, result);
        std::sort(result.begin(), result.end());
    } else {
        throw std::runtime_error("Path does not exist: " + root);
    }

    return result;
}
