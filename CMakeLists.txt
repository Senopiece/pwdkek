cmake_minimum_required(VERSION 3.10)
project(PasswordComplexityEstimator)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set path to gzstream
set(GZSTREAM_INCLUDE_DIR "/usr/local/include")
set(GZSTREAM_LIBRARY "/usr/local/lib/libgzstream.a")

# Find zlib library (gzstream depends on zlib)
find_package(ZLIB REQUIRED)

# Add include directories
include_directories(${GZSTREAM_INCLUDE_DIR})

# Add the main project executable
add_executable(PasswordComplexityEstimator estimator.cpp)

# Link libraries
target_link_libraries(PasswordComplexityEstimator ${GZSTREAM_LIBRARY} ZLIB::ZLIB)
