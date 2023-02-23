gunit_project(gunit-models)

include_directories(.
                    ${Lua_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    )

gunit_collect_src(LIST_OUT "tests")

add_library(gunit-models ${LIST_OUT})
add_dependencies(gunit-models models)
