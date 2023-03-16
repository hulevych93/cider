gunit_project(models)

gunit_collect_src(LIST_OUT)

add_library(models ${LIST_OUT})

add_dependencies(models ${GUNIT_THIRDPARTY_TARGET_NAME})
