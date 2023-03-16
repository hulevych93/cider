gunit_project(recorder_test)

gunit_collect_src(LIST_OUT)

add_executable(recorder_test ${LIST_OUT})
add_test(NAME "recorder_test" COMMAND recorder_test)

add_dependencies(recorder_test ${GUNIT_THIRDPARTY_TARGET_NAME})

target_include_directories(recorder_test PRIVATE ${GTest_INCLUDE_DIRS})

target_link_libraries(recorder_test
    recorder
    ${GTest_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)
