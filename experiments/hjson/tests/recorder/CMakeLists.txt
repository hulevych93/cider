# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_recorder)

set(GEN_DIR ${CIDER_GEN_DIR}/hjson)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_executable(hjson_recorder hjson_recorder.cpp ${TESTED_FILES})
add_dependencies(hjson_recorder ${CIDER_THIRDPARTY_TARGET_NAME})

target_include_directories(hjson_recorder PRIVATE ${GEN_DIR})
target_link_libraries(hjson_recorder PUBLIC
  hjson_tests
  hjson_wrap
  hjson-models
  recorder
)
