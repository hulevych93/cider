gunit_project(scripting)

gunit_collect_src(LIST_OUT "tests")

add_library(scripting ${LIST_OUT})

target_include_directories(scripting PRIVATE ${Lua_INCLUDE_DIRS})

add_dependencies(scripting ${GUNIT_THIRDPARTY_TARGET_NAME})
