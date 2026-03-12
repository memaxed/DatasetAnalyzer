# datasetanalyzer

CLI tool and C++ wrapper library for iris image quality analysis.
Built on top of [BIQTIris](https://github.com/mitre/biqt-iris) by MITRE.

Computes 29 metrics and 9 geometry features per image according to ISO/IEC 29794-6.
Supports single files and batch directory processing.

BIQTIris is compiled directly into the binary — no external framework installation required.

---

## Requirements

### Linux (Ubuntu/Debian)
- g++ with C++17 support
- cmake
- libjsoncpp-dev
- libopencv-dev

### Windows
- Visual Studio 2019+ with C++ Desktop workload
- [vcpkg](https://github.com/microsoft/vcpkg) with `jsoncpp` and `opencv`

---

## Build

### Linux
```bash
sudo apt install -y build-essential cmake libjsoncpp-dev libopencv-dev
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

### Windows (MSVC + vcpkg)
```cmd
vcpkg install jsoncpp:x64-windows opencv:x64-windows
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake -A x64
cmake --build . --config Release
```

---

## Usage
```bash
# Single image
./datasetanalyzer --path iris_0001.png

# Directory
./datasetanalyzer --path ./dataset

# Directory with all subdirectories
./datasetanalyzer --path ./dataset --recursive

# Save results to file
./datasetanalyzer --path ./dataset --output csv --out-file results.csv
```

### Options

| Flag          | Description                                      |
|---------------|--------------------------------------------------|
| `--path`      | Path to image file or directory (required)       |
| `--output`    | Output format: `table` (default), `csv`, `json`  |
| `--out-file`  | Write output to file                             |
| `--recursive` | Scan subdirectories                              |
| `--help`      | Show help                                        |

---

## Output formats

**table** — human-readable console output grouped by metric sections.

**csv** — one row per metric: `Provider,Image,Detection,AttributeType,Key,Value`.

**json** — array of objects, each containing `image`, `metrics`, `features`.

---

## Supported image formats

PNG, JPG/JPEG, BMP, TIFF

### BIQTIris constraints
- Resolution: 256×256 to 1000×680 pixels
- Bit depth: 8 bpp grayscale
- Image must contain a detectable iris — otherwise an error is returned

---

## Using as a library

`datasetanalyzer` is also a reusable C++ library. Include a single header and
use everything through the `QualityAPI` namespace.

### Minimal example

**`demo.cpp`:**
```cpp
#include "IrisQualityAPI.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) { std::cout << "Usage: demo <image.png>\n"; return 1; }

    IrisResult r = QualityAPI::analyze(argv[1]);
    if (r.hasError()) { std::cerr << r.error << "\n"; return 1; }

    std::cout << "Quality: " << QualityAPI::overallQuality(r) << "\n";
    std::cout << "Label: " << QualityAPI::qualityLabel(r)   << "\n";
    std::cout << "Sharpness: " << QualityAPI::isoSharpness(r)  << "\n";
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
project(demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib/datasetanalyzer)

add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE irisquality)
```

### API 

Full API reference with all 29 metrics, 9 geometry features, and normalized
scores is documented in [`include/IrisQualityAPI.h`](include/IrisQualityAPI.h).