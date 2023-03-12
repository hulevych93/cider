gunit_project(models)

include_directories(.
                    ${Boost_INCLUDE_DIRS}
                    )

gunit_collect_src(LIST_OUT "tests")

add_library(models ${LIST_OUT})
add_dependencies(models ${GUNIT_THIRDPARTY_TARGET_NAME})
