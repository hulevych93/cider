gunit_project(gunit-tool)

gunit_collect_src(SRC "tests")

add_executable(gunit ${SRC})

target_include_directories(gunit PRIVATE ${CppAst_INCLUDE_DIRS} ${CxxOpts_INCLUDE_DIRS})

target_link_libraries(gunit PUBLIC ${CppAst_LIBRARIES} ${PLATFORM_LIBRARIES})

