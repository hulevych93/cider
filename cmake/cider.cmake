#cider.cmake

function(cider_tool_generate MODULE_NAME FILES OUTPUT_DIR)
    cider_add_cmake_deps(${FILES})

    # This ensures that there is no files from previous cmake runs.
    file(REMOVE_RECURSE "${OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    set(CIDER_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/cider")

    set(OUTPUT_FILES)
    foreach(FILE ${FILES})
        get_filename_component(FILE_NAME "${FILE}" NAME_WE)
        list(APPEND OUTPUT_FILES "${OUTPUT_DIR}/${FILE_NAME}.h")
        list(APPEND OUTPUT_FILES "${OUTPUT_DIR}/${FILE_NAME}.cpp")

        set_source_files_properties("${OUTPUT_DIR}/${FILE_NAME}.h" PROPERTIES GENERATED TRUE)
        set_source_files_properties("${OUTPUT_DIR}/${FILE_NAME}.cpp" PROPERTIES GENERATED TRUE)
    endforeach()

    set_source_files_properties(${GEN_DIR}/${MODULE_NAME}.swig PROPERTIES GENERATED TRUE)
    set_source_files_properties(${GEN_DIR}/${MODULE_NAME}_lua.cpp PROPERTIES GENERATED TRUE)

    set(OUT_PARAM)
    cider_join_list(FILES ":" OUT_PARAM)

    get_filename_component(CIDER_DIRECTORY ${CIDER_EXECUTABLE} DIRECTORY)
    add_custom_target(${MODULE_NAME}_gen_swig
                       COMMAND "${CIDER_EXECUTABLE}" --out_dir=${OUTPUT_DIR}
                       --database_dir=${CMAKE_BINARY_DIR}
                       --files="${OUT_PARAM}"
                       --swig=${MODULE_NAME} --std=c++14 --lua --namespace=hook
                       WORKING_DIRECTORY ${CIDER_DIRECTORY}
                       DEPENDS cider
                       COMMENT "Generating cider files...")
endfunction()
