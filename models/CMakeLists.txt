gunit_project(models)

prv_collect_src(${CMAKE_SOURCE_DIR}/models HDRS_OUT ".swig" ".cpp")

set(GEN_DIR ${GUNIT_GEN_DIR}/wrapper)
set(SWIG_MODULE_NAME "example")

gunit_generate_swig_config(${SWIG_MODULE_NAME} "${HDRS_OUT}" ${GEN_DIR})
set_source_files_properties(${GEN_DIR}/${SWIG_MODULE_NAME}.swig PROPERTIES GENERATED TRUE)

gunit_generate_swig(LANGUAGE lua
                    EXTERNAL_RUNTIME
                    OUTPUT_DIR "${GEN_DIR}"
                    DEPENDS gen_swig
                    FILE "${GEN_DIR}/${SWIG_MODULE_NAME}.swig")

include_directories(.
                    ${Lua_INCLUDE_DIRS}
                    ${GEN_DIR}
                    ${Boost_INCLUDE_DIRS}
                    )

gunit_collect_src(LIST_OUT "tests")

add_library(models ${LIST_OUT})
add_dependencies(models ${GUNIT_THIRDPARTY_TARGET_NAME})

add_library(models_wrap "${GEN_DIR}/${SWIG_MODULE_NAME}_wrap.cpp" "${GEN_DIR}/swigluarun.h")
add_dependencies(models_wrap models)

add_subdirectory(tests)
