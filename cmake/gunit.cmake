#gunit.cmake

function(gunit_generate_sources FILES OUTPUT_DIR)
    gunit_add_cmake_deps(${FILES})

    # This ensures that there is no files from previous cmake runs.
    file(REMOVE_RECURSE "${OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    set(GUNIT_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/gunit")

    set(OUTPUT_FILES)
    foreach(FILE ${FILES})
        get_filename_component(FILE_NAME "${FILE}" NAME_WE)
        list(APPEND OUTPUT_FILES "${OUTPUT_DIR}/${FILE_NAME}.h")
        list(APPEND OUTPUT_FILES "${OUTPUT_DIR}/${FILE_NAME}.cpp")
    endforeach()

    set(OUT_PARAM)
    gunit_join_list(FILES ":" OUT_PARAM)

    get_filename_component(FILE_NAME "${FILE}" NAME_WE)
    get_filename_component(GUNIT_DIRECTORY ${GUNIT_EXECUTABLE} DIRECTORY)
    add_custom_command(OUTPUT ${OUTPUT_FILES}
                       COMMAND "${GUNIT_EXECUTABLE}" --out_dir=${OUTPUT_DIR} --files=${OUT_PARAM} --namespace=hook 2>/dev/null
                       WORKING_DIRECTORY ${GUNIT_DIRECTORY}
                       COMMENT "Generating gunit wrapper files...")
endfunction()

function(gunit_generate_swig_config MODULE_NAME FILES OUTPUT_DIR)
    gunit_add_cmake_deps(${FILES})

    # This ensures that there is no files from previous cmake runs.
    file(REMOVE_RECURSE "${OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    set(GUNIT_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/gunit")

    set(OUT_PARAM)
    gunit_join_list(FILES ":" OUT_PARAM)

    get_filename_component(FILE_NAME "${FILE}" NAME_WE)
    get_filename_component(GUNIT_DIRECTORY ${GUNIT_EXECUTABLE} DIRECTORY)
    add_custom_target(gen_swig
                       COMMAND "${GUNIT_EXECUTABLE}" --out_dir=${OUTPUT_DIR} --files=${OUT_PARAM} --swig=${MODULE_NAME} 2>/dev/null
                       WORKING_DIRECTORY ${GUNIT_DIRECTORY}
                       DEPENDS gunit
                       COMMENT "Generating gunit swig files...")
endfunction()
