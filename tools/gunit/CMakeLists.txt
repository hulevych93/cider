gunit_project(gunit-tool)

gunit_collect_src(SRC "tests")

include_directories(.
    ${CppAst_INCLUDE_DIRS}
    ${CxxOpts_INCLUDE_DIRS}
)

add_executable(gunit ${SRC})

target_link_libraries(gunit PUBLIC
    ${CppAst_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)

