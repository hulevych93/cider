# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++_tests)

set(GEN_DIR ${CIDER_GEN_DIR}/bitmap++)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${GTest_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_library(bitmap++_tests OBJECT
    read_bitmap.cpp
    rotation.cpp
    variant_shapes.cpp
    polymorphic_shapes.cpp
    write_bitmap.cpp
    chess_board.cpp
    draw_primitives.cpp)

add_dependencies(bitmap++_tests ${CIDER_THIRDPARTY_TARGET_NAME})

target_compile_definitions(bitmap++_tests PRIVATE "ROOT_DIR=\"${PROJECT_SOURCE_DIR}\"")
target_compile_definitions(bitmap++_tests PRIVATE "BIN_DIR=\"${PROJECT_BINARY_DIR}\"")

set(SWIG_MODULE_NAME "bitmap_cplus")

prv_collect_src(${BitmapPlusPlus_INCLUDE_DIRS} HDRS_OUT ".swig" ".cpp")

set(GEN_SRC_FILES)
foreach(HDR ${HDRS_OUT})
    get_filename_component(FILE_NAME "${HDR}" NAME_WE)
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.h")
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.cpp")
endforeach()

cider_tool_generate(${SWIG_MODULE_NAME} "BitmapPlusPlusHooked" "${HDRS_OUT}" ${GEN_DIR})

list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_lua.cpp")
list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_mutator.cpp")
list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_serialization.cpp")
list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_deserialization.cpp")

add_library(bitmap++-models ${GEN_SRC_FILES})
add_dependencies(bitmap++_tests bitmap++-models)

cider_generate_swig(LANGUAGE lua
                    EXTERNAL_RUNTIME
                    OUTPUT_DIR "${GEN_DIR}"
                    DEPENDS bitmap++-models
                    FILE "${GEN_DIR}/${SWIG_MODULE_NAME}.swig")

add_library(bitmap++_wrap "${GEN_DIR}/${SWIG_MODULE_NAME}_wrap.cpp" "${GEN_DIR}/swigluarun.h")
add_dependencies(bitmap++_wrap bitmap++ bitmap++-models)
add_dependencies(bitmap++_tests bitmap++_wrap)

target_link_libraries(bitmap++_tests PUBLIC
  bitmap++_wrap
  bitmap++-models
  recorder
  serialization
  ${BitmapPlusPlus_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)
