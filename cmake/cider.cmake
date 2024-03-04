# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

function(cider_tool_generate MODULE_NAME NAMESPACE FILES OUTPUT_DIR)
    cider_add_cmake_deps(${FILES})

    # This ensures that there is no files from previous cmake runs.
    set(CIDER_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/cider")

    set(OUTPUT_FILES)
    foreach(FILE ${FILES})
        get_filename_component(FILE_NAME "${FILE}" NAME_WE)
        list(APPEND OUTPUT_FILES "${OUTPUT_DIR}/${FILE_NAME}.h")
        list(APPEND OUTPUT_FILES "${OUTPUT_DIR}/${FILE_NAME}.cpp")
    endforeach()

    set(OUT_PARAM)
    cider_join_list(FILES ":" OUT_PARAM)

    get_filename_component(CIDER_DIRECTORY ${CIDER_EXECUTABLE} DIRECTORY)
    add_custom_command(OUTPUT ${OUTPUT_FILES}
                              ${GEN_DIR}/${MODULE_NAME}.swig
                              ${GEN_DIR}/${MODULE_NAME}_lua.cpp
                              ${GEN_DIR}/${MODULE_NAME}_mutator.cpp
                       COMMAND "${CIDER_EXECUTABLE}" --out_dir=${OUTPUT_DIR}
                       --integration_file="${CMAKE_SOURCE_DIR}/tools/ast.cpp"
                       --swig_directory="${CMAKE_SOURCE_DIR}/swig"
                       --database_dir=${CMAKE_BINARY_DIR}
                       --files="${OUT_PARAM}"
                       --swig=${MODULE_NAME} --std=c++14 --lua --namespace=${NAMESPACE} 2>/dev/null
                       WORKING_DIRECTORY ${CIDER_DIRECTORY}
                       DEPENDS cider
                       COMMENT "Generating cider files...")
endfunction()
