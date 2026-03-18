# datasetanalyzer

C++ library for iris image quality analysis.
Built on top of [BIQTIris](https://github.com/mitre/biqt-iris) by mitre.

Computes 29 metrics per image according to ISO/IEC 29794-6.

---

## Requirements

### Linux (Ubuntu/Debian)
- g++ with C++17 support
- cmake
- libopencv-dev

### Windows
- Visual Studio 2019+ with C++ Desktop workload
- [vcpkg](https://github.com/microsoft/vcpkg) with `opencv`
---

## Build

### Linux
```bash
sudo apt install -y build-essential cmake libopencv-dev
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

### Windows (MSVC + vcpkg)
```cmd
vcpkg install opencv:x64-windows
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake -A x64
cmake --build . --config Release
```

---

## Using as a library

Include a single header and use everything through the `QualityAPI` namespace. You also need OpenCV.

### Minimal example

**`demo.cpp`:**
```cpp
#include <iostream>
#include <map>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "IrisQualityAPI.h"

int main(int argc, char** argv) {
    const std::string imagePath = (argc > 1) ? argv[1] : "S5078L00.jpg";

    // Load image: must be grayscale
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Failed to load image: " << imagePath << "\n";
        return 1;
    }

    std::cout << "Image: " << imagePath << "  "
              << image.cols << "x" << image.rows << "\n\n";

    // From outer detector
    DetectedIris det{312, 227, 93, 314, 229, 42};

    // Run quality assessment
    std::map<std::string, float> scores;
    int32_t rc = QualityAPI::assess(image, det, &scores);

    if (rc != 0) {
        std::cerr << "assess() failed with code " << rc << "\n";
        return rc;
    }

    // Overall
    std::cout << "--- Overall ---\n";
    std::cout << "Label: " << QualityAPI::qualityLabel(scores) << "\n";
    std::cout << "ISO overall: " << QualityAPI::overallQuality(scores) << "\n";
    std::cout << "Raw quality: " << QualityAPI::rawQuality(scores) << "\n\n";

    // ISO metrics
    std::cout << "--- ISO Metrics ---\n";
    std::cout << "Sharpness: " << QualityAPI::isoSharpness(scores) << "\n";
    std::cout << "Usable iris area: " << QualityAPI::isoUsableIrisArea(scores) << "\n";
    std::cout << "Iris-pupil contr: " << QualityAPI::isoIrisPupilContrast(scores) << "\n";
    std::cout << "Iris-sclera contr: " << QualityAPI::isoIrisScleraContrast(scores) << "\n";
    std::cout << "Iris-pupil ratio: " << QualityAPI::isoIrisPupilRatio(scores) << "\n";
    std::cout << "Concentricity: " << QualityAPI::isoIrisPupilConcentricity(scores) << "\n";
    std::cout << "Margin adequacy: " << QualityAPI::isoMarginAdequacy(scores) << "\n";
    std::cout << "Greyscale util: " << QualityAPI::isoGreyscaleUtilization(scores) << "\n";
    std::cout << "Pupil circ: " << QualityAPI::isoPupilBoundaryCircularity(scores) << "\n\n";

    return 0;
}
```

**Project layout:**
```
demo/
├── CMakeLists.txt
├── demo.cpp
└── lib/
    └── datasetanalyzer/   ← this repo
```

**`demo CMakeLists.txt`:**
```cmake
cmake_minimum_required(VERSION 3.14)
project(irisquality_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_subdirectory(lib/DatasetAnalyzer)

add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE irisqualityapi ${OpenCV_LIBS})
```

### API 

Full API reference with all 29 metrics, and normalized
scores is documented in [`include/IrisQualityAPI.h`](include/IrisQualityAPI.h).

---

## BIQTIris native constraints

The original BIQTIris constraints are preserved. Removing them from [`lib/biqt-iris/src/BIQTIris.cpp`](lib/biqt-iris/src/BIQTIris.cpp) may result in undefined behavior or crashes.
Constraints:
- Resolution: 256×256 to 1000×680 pixels
- Bit depth: 8 bpp grayscale