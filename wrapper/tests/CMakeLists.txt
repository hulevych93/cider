gunit_project(wrapper_test)

gunit_collect_src(LIST_OUT)

include_directories(.
    ${GTest_INCLUDE_DIRS}
)

add_executable(wrapper_test ${LIST_OUT})

target_link_libraries(wrapper_test
    wrapper
    models
    recorder
    ${Fmt_LIBRARIES}
    ${GTest_LIBRARIES}
    ${Lua_LIBRARIES})
