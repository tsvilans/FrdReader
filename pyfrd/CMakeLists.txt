cmake_minimum_required(VERSION 3.15...3.30)
project(
    ${SKBUILD_PROJECT_NAME} 
    VERSION ${SKBUILD_PROJECT_VERSION} 
    LANGUAGES CXX)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

file(GLOB PYFRD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(LIBFRD_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/../libfrd/frd_reader.cpp"
    )

list (APPEND PYFRD_SOURCES ${LIBFRD_SOURCES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libfrd)

python_add_library(pyfrd MODULE ${PYFRD_SOURCES} WITH_SOABI)
target_link_libraries(pyfrd PRIVATE pybind11::headers)

target_compile_definitions(pyfrd PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(TARGETS pyfrd DESTINATION .)