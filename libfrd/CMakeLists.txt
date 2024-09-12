file(GLOB LIBFRD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(libfrd STATIC ${LIBFRD_SOURCES})
target_include_directories(libfrd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
