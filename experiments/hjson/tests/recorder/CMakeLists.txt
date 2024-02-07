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

add_executable(hjson_recorder ${LIST_OUT})

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
  ${Hjson_LIBRARIES}
)
