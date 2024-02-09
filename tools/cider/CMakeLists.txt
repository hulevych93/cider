# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(cider-tool)

cider_collect_src(SRC "tests")

add_executable(cider ${SRC})

add_dependencies(cider ${CIDER_THIRDPARTY_TARGET_NAME})

target_include_directories(cider PRIVATE ${CppAst_INCLUDE_DIRS} ${CxxOpts_INCLUDE_DIRS})

target_link_directories(cider PRIVATE ${CppAst_LIB_DIR})

target_link_libraries(cider PUBLIC ${CppAst_LIBRARIES} ${PLATFORM_LIBRARIES})

set_target_properties(cider PROPERTIES BUILD_RPATH "${CppAst_LIB_DIR}")
