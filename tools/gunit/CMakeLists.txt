gunit_project(gunit-tool)

include_directories(.
    ${CppAst_INCLUDE_DIRS}
    ${CxxOpts_INCLUDE_DIRS}
)

add_executable(gunit main.cpp)

target_link_libraries(gunit PUBLIC
    ${CppAst_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)

