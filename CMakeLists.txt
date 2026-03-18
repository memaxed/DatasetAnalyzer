cmake_minimum_required(VERSION 3.14)
project(irisqualityapi VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W3 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(OpenCV REQUIRED core)

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
        PROPERTIES COMPILE_FLAGS "-fPIC -Wno-reorder -Wno-unused-variable -Wno-unused-parameter"
    )
endif()

add_library(irisqualityapi
    ${BIQTIRIS_SOURCES}
)

add_library(irisqualityapi::irisqualityapi ALIAS irisqualityapi)

target_include_directories(irisqualityapi
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${BIQTIRIS_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(irisqualityapi
    PUBLIC
        ${OpenCV_LIBS}
)

target_compile_features(irisqualityapi PUBLIC cxx_std_17)

if(NOT WIN32)
    target_link_libraries(irisqualityapi PUBLIC ${CMAKE_DL_LIBS})
endif()

install(TARGETS irisqualityapi
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    include/IrisQualityAPI.h
    include/DetectedIris.h
    DESTINATION include/irisqualityapi
)

message(STATUS "")
message(STATUS "--- irisquality build configuration ---")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "")
