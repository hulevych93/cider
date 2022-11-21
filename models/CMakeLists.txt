gunit_project(models)

gunit_collect_src(LIST_OUT "tests")

add_library(models ${LIST_OUT})
