# Find libpipeline.

FIND_PATH(PIPELINE_INCLUDE_DIR pipeline.h)

SET(PIPELINE_NAMES ${PIPELINE_NAMES} pipeline libpipeline)
FIND_LIBRARY(PIPELINE_LIBRARY NAMES ${PIPELINE_NAMES} PATH)

IF(PIPELINE_INCLUDE_DIR AND PIPELINE_LIBRARY)
    SET(PIPELINE_FOUND TRUE)
ENDIF(PIPELINE_INCLUDE_DIR AND PIPELINE_LIBRARY)

IF(PIPELINE_FOUND)
    IF(NOT Pipeline_FIND_QUIETLY)
        MESSAGE(STATUS "Found libpipeline: ${PIPELINE_LIBRARY}")
    ENDIF (NOT Pipeline_FIND_QUIETLY)
ELSE(PIPELINE_FOUND)
    IF(Pipeline_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find pipeline")
    ENDIF(Pipeline_FIND_REQUIRED)
ENDIF (PIPELINE_FOUND)