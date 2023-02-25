gunit_project(tests)

gunit_collect_src(LIST_OUT)

include_directories(.
    ${GTest_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

add_executable(tests ${LIST_OUT})
add_test(NAME "tests" COMMAND recorder_test)

if(TEST_GENERATOR)
    prv_collect_sources(${CMAKE_BINARY_DIR}/gen SRC_OUT)
    add_library(gunit-gen-models ${SRC_OUT})
    add_dependencies(gunit-gen-models gunit)
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
