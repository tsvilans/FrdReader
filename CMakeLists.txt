cmake_minimum_required(VERSION 3.15...3.30)
project(FrdReader LANGUAGES CXX)

# Ensure -fPIC is used
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # for creating shared libraries

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add subdirectories

include_directories(libfrd)
add_subdirectory(libfrd)

# add_subdirectory(pyfrd)

# add_subdirectory(FrdReaderCommon)