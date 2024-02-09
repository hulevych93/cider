cider_project(pugixml_tests)

set(GEN_DIR ${CIDER_GEN_DIR}/pugixml)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${GTest_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

set(SWIG_MODULE_NAME "pugixml")

prv_collect_src(${Pugixml_INCLUDE_DIRS} HDRS_OUT ".swig" ".cpp")

set(GEN_SRC_FILES)
foreach(HDR ${HDRS_OUT})
    get_filename_component(FILE_NAME "${HDR}" NAME_WE)
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.h")
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.cpp")
endforeach()

cider_tool_generate(${SWIG_MODULE_NAME} "PugixmlHooked" "${HDRS_OUT}" ${GEN_DIR})

list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_lua.cpp")

add_library(pugixml-models ${GEN_SRC_FILES})
add_dependencies(pugixml-models pugixml_gen_swig)
add_dependencies(pugixml_gen_swig ${CIDER_THIRDPARTY_TARGET_NAME})

cider_generate_swig(LANGUAGE lua
                    EXTERNAL_RUNTIME
                    OUTPUT_DIR "${GEN_DIR}"
                    DEPENDS pugixml_gen_swig
                    FILE "${GEN_DIR}/${SWIG_MODULE_NAME}.swig")

add_library(pugixml_wrap "${GEN_DIR}/${SWIG_MODULE_NAME}_wrap.cpp" "${GEN_DIR}/swigluarun.h")
add_dependencies(pugixml_wrap pugixml pugixml-models)
