cmake_minimum_required (VERSION 3.10)
project (RSCPP)

## Include src
aux_source_directory("${CMAKE_CURRENT_LIST_DIR}/src" SRC_FILES)
file(GLOB HEADER_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.h")
include_directories(RSCPP "${CMAKE_CURRENT_LIST_DIR}/src")

## Include OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

## Add Executable
add_executable (RSCPP ${HEADER_FILES} ${SRC_FILES})
set_property(TARGET RSCPP PROPERTY CXX_STANDARD 11)

## Link OpenCV
link_directories(${OpenCV_LIBRARY_DIRS})
target_link_libraries(RSCPP ${OpenCV_LIBS})
# message(STATUS ${OpenCV_LIBS})

## realsense2 sdk
set(DEPENDENCIES realsense2)
target_link_libraries(RSCPP -lrealsense2)
