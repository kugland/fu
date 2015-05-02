CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(fu)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

ADD_DEFINITIONS(
    -std=c++11
    -pthread
)

find_package(Libunwind REQUIRED)

# Set up include paths
include_directories(
    ${LIBUNWIND_INCLUDE_DIR}
)

SET(TARGET_fu_NAME fu)
SET(TARGET_fu_FILES
    src/audio/base/buffer.cpp
    src/audio/base/buffer.hpp
    src/audio/base/connection.cpp
    src/audio/base/connection.hpp
    src/core/logger.cpp
    src/core/logger.hpp
    src/core/semaphore.cpp
    src/core/semaphore.hpp)

ADD_EXECUTABLE(${TARGET_fu_NAME} ${TARGET_fu_FILES})

TARGET_LINK_LIBRARIES(fu ${LIBUNWIND_LIBRARIES})