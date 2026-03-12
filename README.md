# DatasetAnalyzer

CLI tool for iris image quality analysis. Built on top of [BIQT](https://github.com/mitre/biqt) / [BIQTIris](https://github.com/mitre/biqt-iris) by MITRE.

Computes 30 metrics and 9 geometry features per image according to ISO/IEC 29794-6. Supports single files and batch directory processing.

---

## Requirements

- Linux (Ubuntu/Debian)
- g++ with C++17 support
- cmake
- libjsoncpp-dev
- BIQTIris installed (`libBIQTIris.so`)

---

## Build
```bash
sudo apt install -y libjsoncpp-dev
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
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

# Specify library path manually
./datasetanalyzer --path ./dataset --provider /path/to/libBIQTIris.so
```

### Options

| Flag | Description |
|------|-------------|
| `--path` | Path to image file or directory (required) |
| `--provider` | Path to `libBIQTIris.so` (if not auto-detected) |
| `--output` | Output format: `table` (default), `csv`, `json` |
| `--out-file` | Write output to file |
| `--recursive` | Scan subdirectories |
| `--help` | Show help |

---

## Output formats

**table** — human-readable console output grouped by metric sections with sequential numbering.

**csv** — one row per metric: `Provider,Image,Detection,AttributeType,Key,Value`. Convenient for further processing.

**json** — array of objects, each containing `image`, `metrics`, `features`.

---

## Supported image formats

PNG, JPG/JPEG, BMP, TIFF

### BIQTIris constraints

- Resolution: 256×256 to 1000×680 pixels
- Bit depth: 8 bpp (grayscale)
- Image must contain a detectable iris — otherwise an error is returned
