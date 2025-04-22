# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++_recorder)

set(GEN_DIR ${CIDER_GEN_DIR}/bitmap++)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_executable(bitmap++_recorder bitmap++_recorder.cpp ${TESTED_FILES})
add_dependencies(bitmap++_recorder ${CIDER_THIRDPARTY_TARGET_NAME})

target_include_directories(bitmap++_recorder PRIVATE ${GEN_DIR})
target_link_libraries(bitmap++_recorder PUBLIC
  bitmap++_wrap
  bitmap++-models
  bitmap++_tests
  recorder
)
