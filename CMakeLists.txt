cmake_minimum_required(VERSION 3.14)
project(datasetanalyzer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/main.cpp
    src/IrisAnalyzer.cpp
    src/ResultFormatter.cpp
    src/PathUtils.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

add_executable(datasetanalyzer ${SOURCES})

target_include_directories(datasetanalyzer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIRS}
)
target_link_libraries(datasetanalyzer PRIVATE dl ${JSONCPP_LIBRARIES})
target_compile_options(datasetanalyzer PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS datasetanalyzer
    RUNTIME DESTINATION bin
)

message(STATUS "")
message(STATUS "--- datasetanalyzer build configuration ---")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "  After building, run:")
message(STATUS "    datasetanalyzer --path <image_or_dir>")
message(STATUS "")
message(STATUS "  Make sure BIQTIris is installed and builded from:")
message(STATUS "    https://github.com/mitre/biqt-iris")
message(STATUS "")
