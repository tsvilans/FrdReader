find_package(pybind11 REQUIRED)
# Adjust the path to match your actual source files location
file(GLOB PYFRD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
pybind11_add_module(pyfrd ${PYFRD_SOURCES})
target_link_libraries(pyfrd PRIVATE libfrd ${Python3_LIBRARIES})
