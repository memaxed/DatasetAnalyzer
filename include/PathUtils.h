#pragma once

#include <string>
#include <vector>

class PathUtils {
public:
    // returns list of image file paths found at root
    // if root is a file, returns {root} if it's a supported image
    // if root is a directory, scans it for images
    // if recursive==true, also scans subdirectories
    static std::vector<std::string> collectImages(const std::string& root,
                                                   bool recursive = false);

    // returns true if the file has a supported image extension
    static bool isSupportedImage(const std::string& path);

    // returns true if path is an existing directory
    static bool isDirectory(const std::string& path);

    // returns true if path is an existing file
    static bool isFile(const std::string& path);
};
