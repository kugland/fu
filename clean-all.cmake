SET(CMAKE_GENERATED ${CMAKE_BINARY_DIR}/CMakeCache.txt
                    ${CMAKE_BINARY_DIR}/cmake_install.cmake
                    ${CMAKE_BINARY_DIR}/Makefile
                    ${CMAKE_BINARY_DIR}/CMakeFiles
)

FOREACH(FILE ${CMAKE_GENERATED})

  IF (EXISTS ${FILE})
     FILE(REMOVE_RECURSE ${FILE})
  ENDIF()

ENDFOREACH(FILE)
