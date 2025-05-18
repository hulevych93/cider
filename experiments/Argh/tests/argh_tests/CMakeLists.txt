# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(argh_tests)

set(GEN_DIR ${CIDER_GEN_DIR}/argh)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${GTest_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_library(argh_tests OBJECT argh_tests.cpp)

add_dependencies(argh_tests ${CIDER_THIRDPARTY_TARGET_NAME})

set(SWIG_MODULE_NAME "argh")

prv_collect_src(${Argh_INCLUDE_DIRS} HDRS_OUT ".swig" ".cpp")

set(GEN_SRC_FILES)
foreach(HDR ${HDRS_OUT})
    get_filename_component(FILE_NAME "${HDR}" NAME_WE)
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.h")
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.cpp")
endforeach()

cider_tool_generate(${SWIG_MODULE_NAME} "ArghHooked" "${HDRS_OUT}" ${GEN_DIR})

list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_lua.cpp")
list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_mutator.cpp")
list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_serialization.cpp")
list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_deserialization.cpp")
list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_comparison.cpp")
list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_hash.cpp")

add_library(argh-models ${GEN_SRC_FILES})
add_dependencies(argh_tests argh-models)

cider_generate_swig(LANGUAGE lua
                    EXTERNAL_RUNTIME
                    OUTPUT_DIR "${GEN_DIR}"
                    DEPENDS argh-models
                    FILE "${GEN_DIR}/${SWIG_MODULE_NAME}.swig")

add_library(argh_wrap "${GEN_DIR}/${SWIG_MODULE_NAME}_wrap.cpp" "${GEN_DIR}/swigluarun.h")
add_dependencies(argh_wrap argh-models)
add_dependencies(argh_tests argh_wrap)

target_link_libraries(argh_tests PUBLIC
  argh_wrap
  argh-models
  recorder
  serialization
  ${Argh_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)
