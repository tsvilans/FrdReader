cmake_minimum_required(VERSION 3.10)
project(FrdReader)

# Ensure -fPIC is used
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # for creating shared libraries

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add subdirectories
add_subdirectory(libfrd)
add_subdirectory(pyfrd)

# For Python integration
find_package(Python3 COMPONENTS Development REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})

# Add the executable (if needed, specify your actual main source file)
# add_executable(FrdReader main.cpp)
# target_link_libraries(FrdReader libfrd ${Python3_LIBRARIES})
