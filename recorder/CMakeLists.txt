gunit_project(recorder)

gunit_collect_src(RECORDER_SRC "tests")

add_library(recorder ${RECORDER_SRC})

target_include_directories(recorder PRIVATE ${Fmt_INCLUDE_DIRS} PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(recorder PRIVATE ${Fmt_LIBRARIES})

add_dependencies(recorder ${GUNIT_THIRDPARTY_TARGET_NAME})

add_subdirectory(tests)
