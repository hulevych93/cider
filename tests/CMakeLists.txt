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

set(GEN_DIR ${GUNIT_GEN_DIR}/gunit-models)

prv_collect_src(${CMAKE_SOURCE_DIR}/models HDRS_OUT ".swig" ".cpp")

set(GEN_SRC_FILES)
foreach(HDR ${HDRS_OUT})
    get_filename_component(FILE_NAME "${HDR}" NAME_WE)
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.h")
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.cpp")
endforeach()

gunit_generate_sources("${HDRS_OUT}" ${GEN_DIR})

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
