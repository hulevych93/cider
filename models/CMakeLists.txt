gunit_project(models)

set(GEN_DIR ${GUNIT_GEN_DIR}/wrapper)
gunit_generate_swig(LANGUAGE lua
                    EXTERNAL_RUNTIME
                    OUTPUT_DIR "${GEN_DIR}"
                    SWIG_FILES "${PROJECT_SOURCE_DIR}/models.swig")

include_directories(.
                    ${Lua_INCLUDE_DIRS}
                    ${GEN_DIR}
                    )

gunit_collect_src(LIST_OUT "tests")
list(APPEND LIST_OUT "${GEN_DIR}/models_wrap.cpp" "${GEN_DIR}/swigluarun.h")

add_library(models ${LIST_OUT})
add_dependencies(models swig)

add_subdirectory(tests)
