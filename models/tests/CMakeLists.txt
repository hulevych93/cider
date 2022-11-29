gunit_project(models_test)

gunit_collect_src(LIST_OUT)

include_directories(.
    ${GTest_INCLUDE_DIRS}
)

add_executable(models_test ${LIST_OUT})
add_test(NAME "models_test" COMMAND models_test)

target_link_libraries(models_test
    models
    recorder
    scripting
    ${Fmt_LIBRARIES}
    ${GTest_LIBRARIES}
    ${Lua_LIBRARIES})
