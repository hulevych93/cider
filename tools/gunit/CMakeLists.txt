gunit_project(gunit-tool)

gunit_collect_src(SRC "tests")

add_executable(gunit ${SRC})

add_dependencies(gunit ${GUNIT_THIRDPARTY_TARGET_NAME})

target_include_directories(gunit PRIVATE ${CppAst_INCLUDE_DIRS} ${CxxOpts_INCLUDE_DIRS})

target_link_directories(gunit PRIVATE ${CppAst_LIB_DIR})

target_link_libraries(gunit PUBLIC ${CppAst_LIBRARIES} ${PLATFORM_LIBRARIES})

