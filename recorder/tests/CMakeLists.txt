gunit_project(recorder_test)

gunit_collect_src(LIST_OUT)

include_directories(.
    ${GTest_INCLUDE_DIRS}
)

add_executable(recorder_test ${LIST_OUT})
add_test(NAME "recorder_test" COMMAND recorder_test)

target_link_libraries(recorder_test
    recorder
    models
    scripting
    ${GTest_LIBRARIES}
    ${Fmt_LIBRARIES}
    ${Lua_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)
