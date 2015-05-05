CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

PROJECT(fu)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

ADD_DEFINITIONS(
    -std=c++11
    -pthread
)

FIND_PACKAGE(Sndfile REQUIRED)
FIND_PACKAGE(SampleRate REQUIRED)
FIND_PACKAGE(Pipeline REQUIRED)
FIND_PACKAGE(Libunwind REQUIRED)

# Set up include paths
INCLUDE_DIRECTORIES(
    ${SNDFILE_INCLUDE_DIR}
    ${SAMPLERATE_INCLUDE_DIR}
    ${PIPELINE_INCLUDE_DIR}
    ${LIBUNWIND_INCLUDE_DIR}
)

SET(TARGET_fu_NAME fu)
SET(TARGET_fu_FILES
    src/audio/buffer.cpp
    src/audio/buffer.hpp
    src/audio/connection.cpp
    src/audio/connection.hpp
    src/logger.cpp
    src/logger.hpp
    src/semaphore.cpp
    src/semaphore.hpp
)

ADD_EXECUTABLE(${TARGET_fu_NAME} ${TARGET_fu_FILES})

TARGET_LINK_LIBRARIES(
    ${TARGET_fu_NAME}
    ${SNDFILE_LIBRARY}
    ${SAMPLERATE_LIBRARY}
    ${PIPELINE_LIBRARY}
    ${LIBUNWIND_LIBRARIES}
)

ADD_CUSTOM_TARGET(clean-cmake-files COMMAND ${CMAKE_COMMAND} -P clean-all.cmake)
ADD_CUSTOM_TARGET(tarball COMMAND sh ${CMAKE_BINARY_DIR}/make-source-tarball.sh)
