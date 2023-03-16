gunit_project(recorder_test)

gunit_collect_src(LIST_OUT)

add_executable(recorder_test ${LIST_OUT})
add_test(NAME "recorder_test" COMMAND recorder_test)

target_include_directories(recorder_test PRIVATE ${GTest_INCLUDE_DIRS})

target_link_libraries(recorder_test
    recorder
    ${GTest_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)
