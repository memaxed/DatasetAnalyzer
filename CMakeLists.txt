cmake_minimum_required(VERSION 3.14)
project(datasetanalyzer VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W3 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(OpenCV REQUIRED core highgui)

find_package(jsoncpp QUIET)
if(NOT jsoncpp_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(JSONCPP jsoncpp)
    endif()
endif()

set(BIQTIRIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/biqt-iris")

set(BIQTIRIS_SOURCES
    ${BIQTIRIS_DIR}/src/BIQTIris.cpp
    ${BIQTIRIS_DIR}/src/ImageOps.cpp
)

if(MSVC)
    set_source_files_properties(${BIQTIRIS_SOURCES}
        PROPERTIES COMPILE_FLAGS "/wd4244"
    )
else()
    set_source_files_properties(${BIQTIRIS_SOURCES}
        PROPERTIES COMPILE_FLAGS "-Wno-reorder -Wno-unused-variable -Wno-unused-parameter"
    )
endif()

if(NOT WIN32)
    set_source_files_properties(${BIQTIRIS_SOURCES}
        PROPERTIES COMPILE_FLAGS "-fPIC"
    )
endif()

add_library(irisquality
    src/IrisAnalyzer.cpp
    src/ResultFormatter.cpp
    src/PathUtils.cpp
    ${BIQTIRIS_SOURCES}
)

add_library(irisquality::irisquality ALIAS irisquality)

target_include_directories(irisquality
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${BIQTIRIS_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(irisquality
    PUBLIC
        ${OpenCV_LIBS}
)

if(jsoncpp_FOUND)
    target_link_libraries(irisquality PUBLIC jsoncpp_lib)
elseif(JSONCPP_FOUND)
    target_include_directories(irisquality PUBLIC ${JSONCPP_INCLUDE_DIRS})
    target_link_libraries(irisquality PUBLIC ${JSONCPP_LIBRARIES})
endif()

target_compile_features(irisquality PUBLIC cxx_std_17)

add_executable(datasetanalyzer
    src/main.cpp
)

target_link_libraries(datasetanalyzer PRIVATE irisquality)

if(NOT WIN32)
    target_link_libraries(irisquality PUBLIC ${CMAKE_DL_LIBS})
endif()

find_library(LIBPQ_LIB NAMES pq)
if(LIBPQ_LIB)
    target_link_libraries(irisquality PUBLIC ${LIBPQ_LIB})
else()
    message(STATUS "libpq not found — suppressing possible GDAL/PQ linker errors")
    if(NOT MSVC)
        target_link_options(irisquality PRIVATE -Wl,--allow-shlib-undefined)
    endif()
endif()

install(TARGETS irisquality datasetanalyzer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

message(STATUS "")
message(STATUS "--- datasetanalyzer build configuration ---")
message(STATUS "  Platform      : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler      : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  OpenCV version: ${OpenCV_VERSION}")
message(STATUS "")
message(STATUS "  After building, run:")
message(STATUS "    datasetanalyzer --path <image_or_dir>")
message(STATUS "")

if(WIN32)
    message(STATUS "  Windows build tips:")
    message(STATUS "    - Recommended compiler: MSVC (Visual Studio 2019+) or MinGW-w64")
    message(STATUS "    - Use vcpkg to install jsoncpp and OpenCV:")
    message(STATUS "        vcpkg install jsoncpp opencv")
    message(STATUS "    - Then pass:")
    message(STATUS "        -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake")
endif()

message(STATUS "")