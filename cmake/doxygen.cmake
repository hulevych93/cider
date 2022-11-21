# doxygen.cmake

unset(DOXYGEN_EXECUTABLE CACHE)

find_program(DOXYGEN_EXECUTABLE "doxygen")
if(NOT DOXYGEN_EXECUTABLE)
  message("DOXYGEN_EX: doxygen executable not found...")
endif()

if(DOXYGEN_EXECUTABLE)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc)
    add_custom_target(gengine-doc
        COMMENT "Make documenatation dir..."
        COMMAND "${CMAKE_COMMAND}" -E env "INPUT_DIR=${CMAKE_SOURCE_DIR}" "${DOXYGEN_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/doxygen_config.txt"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
    )
endif()
