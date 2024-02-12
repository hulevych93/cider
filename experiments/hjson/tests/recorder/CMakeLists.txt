# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_recorder)

set(GEN_DIR ${CIDER_GEN_DIR}/hjson)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${GTest_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

cider_collect_src(LIST_OUT)

add_executable(hjson_recorder ${LIST_OUT} ${TESTED_FILES})
add_dependencies(hjson_recorder ${CIDER_THIRDPARTY_TARGET_NAME})

set(SWIG_MODULE_NAME "hjson")

prv_collect_src(${Hjson_INCLUDE_DIRS} HDRS_OUT ".swig" ".cpp")

set(GEN_SRC_FILES)
foreach(HDR ${HDRS_OUT})
    get_filename_component(FILE_NAME "${HDR}" NAME_WE)
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.h")
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.cpp")
endforeach()

cider_tool_generate(${SWIG_MODULE_NAME} "HjsonHooked" "${HDRS_OUT}" ${GEN_DIR})

list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_lua.cpp")

add_library(hjson-models ${GEN_SRC_FILES})
add_dependencies(hjson-models hjson_gen_swig)

cider_generate_swig(LANGUAGE lua
                    EXTERNAL_RUNTIME
                    OUTPUT_DIR "${GEN_DIR}"
                    DEPENDS hjson_gen_swig
                    FILE "${GEN_DIR}/${SWIG_MODULE_NAME}.swig")

add_library(hjson_wrap "${GEN_DIR}/${SWIG_MODULE_NAME}_wrap.cpp" "${GEN_DIR}/swigluarun.h")
add_dependencies(hjson_wrap hjson hjson-models)

add_dependencies(hjson_recorder hjson_wrap)

target_link_libraries(hjson_recorder PUBLIC
  hjson_wrap
  hjson-models
  recorder
  ${Lua_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

add_custom_target(record_hjson_script
                   COMMAND "${CMAKE_CURRENT_BINARY_DIR}/hjson_recorder" ${SCRIPT_PATH}
                   WORKING_DIRECTORY ${CIDER_DIRECTORY}
                   DEPENDS hjson_recorder
                   COMMENT "Recording script files...")

if(HJSON_ENABLE_COVERAGE)
                   setup_target_for_coverage_lcov(
                       NAME hjson_recoder_coverage
                       EXECUTABLE hjson_recorder
                       EXECUTABLE_ARGS "${CMAKE_BINARY_DIR}/experiments/hjson/tests/script.lua"
                       BASE_DIRECTORY "${EXPERIMENT_HJSON_BUILD_DIR}/src"
                       WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
                       EXCLUDE "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*")
endif()

if(1)
  add_custom_target(coverage_driver_run
      COMMAND coverage_driver
      "${CMAKE_SOURCE_DIR}/experiments/hjson/tests"
      "${EXPERIMENT_HJSON_BUILD_DIR}/src"
      "${CMAKE_BINARY_DIR}/experiments/hjson/tests/player"
      "${CMAKE_BINARY_DIR}/experiments/hjson/tests/scripts/script_55.lua"
      "${CMAKE_BINARY_DIR}/bin"

      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/experiments/hjson/tests/player"
      DEPENDS hjson_player
      VERBATIM # Protect arguments to commands
  )
endif()
