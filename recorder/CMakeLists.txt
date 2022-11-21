gunit_project(recorder)

gunit_collect_src(RECORDER_SRC "tests")

include_directories(.
    ${Fmt_INCLUDE_DIRS}
)

add_library(recorder ${RECORDER_SRC})

add_subdirectory(tests)
