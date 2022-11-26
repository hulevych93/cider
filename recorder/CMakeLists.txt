gunit_project(recorder)

gunit_collect_src(RECORDER_SRC "tests")

include_directories(.
    ${Fmt_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

add_library(recorder ${RECORDER_SRC})

add_dependencies(recorder ${GUNIT_THIRDPARTY_TARGET_NAME})

add_subdirectory(tests)
