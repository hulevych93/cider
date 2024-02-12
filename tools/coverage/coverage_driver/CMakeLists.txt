# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(coverage_driver)

add_executable(coverage_driver coverage_driver.cpp)

target_include_directories(coverage_driver PRIVATE ${Tpl_INCLUDE_DIRS})
target_link_libraries(coverage_driver PUBLIC
  coverage
  ${Tpl_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)
