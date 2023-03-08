gunit_project(tests)

gunit_collect_src(LIST_OUT)

include_directories(.
    ${GTest_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GUNIT_GEN_DIR}
)

add_executable(tests ${LIST_OUT})
add_test(NAME "tests" COMMAND tests)

function(gunit_generate FILES OUTPUT_DIR)
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

set(GEN_DIR ${GUNIT_GEN_DIR}/gunit-models)

prv_collect_src(${CMAKE_SOURCE_DIR}/models HDRS_OUT ".swig" ".cpp")

set(GEN_SRC_FILES)
foreach(HDR ${HDRS_OUT})
    get_filename_component(FILE_NAME "${HDR}" NAME_WE)
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.h")
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.cpp")
endforeach()

gunit_generate("${HDRS_OUT}" ${GEN_DIR})

add_library(gunit-models ${GEN_SRC_FILES})
add_dependencies(gunit-models gunit)

if(CLANGFORMAT_EXECUTABLE)
 foreach(OUT_FILE ${GEN_SRC_FILES})
  add_custom_command(TARGET gunit-models
                   PRE_BUILD
                   COMMAND ${CLANGFORMAT_EXECUTABLE} -style=Chromium -i ${OUT_FILE})
 endforeach()
endif()

target_link_libraries(tests
    recorder
    gunit-models
    models_wrap
    models
    scripting
    ${GTest_LIBRARIES}
    ${Fmt_LIBRARIES}
    ${Lua_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)
