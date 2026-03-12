#include "PathUtils.h"

#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <sys/stat.h>
#include <dirent.h>

static const std::vector<std::string> SUPPORTED_EXTS = {
    ".png", ".jpg", ".jpeg", ".bmp", ".tiff", ".tif"
};

bool PathUtils::isFile(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return (st.st_mode & S_IFREG) != 0;
}

bool PathUtils::isDirectory(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return (st.st_mode & S_IFDIR) != 0;
}

bool PathUtils::isSupportedImage(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return false;
    std::string ext = path.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    for (const auto& e : SUPPORTED_EXTS) {
        if (ext == e) return true;
    }
    return false;
}

static void scanDir(const std::string& dir, bool recursive,
                    std::vector<std::string>& out) {
    DIR* d = opendir(dir.c_str());
    if (!d) return;

    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        std::string name(entry->d_name);
        if (name == "." || name == "..") continue;

        std::string fullPath = dir + "/" + name;

        if (PathUtils::isDirectory(fullPath)) {
            if (recursive) scanDir(fullPath, true, out);
        } else if (PathUtils::isFile(fullPath)) {
            if (PathUtils::isSupportedImage(fullPath)) {
                out.push_back(fullPath);
            }
        }
    }
    closedir(d);
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
