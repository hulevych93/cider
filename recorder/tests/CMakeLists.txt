gunit_project(recorder_test)

gunit_collect_src(LIST_OUT)

include_directories(.
    ${GTest_INCLUDE_DIRS}
)

add_executable(recorder_test ${LIST_OUT})

target_link_libraries(recorder_test
    recorder
    models
    ${GTest_LIBRARIES}
    ${Fmt_LIBRARIES})
