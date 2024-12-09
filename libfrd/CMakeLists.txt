message(STATUS "Building libfrd...")

file(GLOB LIBFRD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

message(STATUS "Sources: ${LIBFRD_SOURCES}")

add_library(libfrd STATIC ${LIBFRD_SOURCES})
target_include_directories(libfrd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
