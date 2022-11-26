gunit_project(scripting)

include_directories(.
    ${Lua_INCLUDE_DIRS}
)

gunit_collect_src(LIST_OUT "tests")

add_library(scripting ${LIST_OUT})
add_dependencies(scripting ${GUNIT_THIRDPARTY_TARGET_NAME})
