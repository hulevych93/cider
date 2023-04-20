cider_project(models)

cider_collect_src(LIST_OUT)

add_library(models ${LIST_OUT})

add_dependencies(models ${CIDER_THIRDPARTY_TARGET_NAME})
