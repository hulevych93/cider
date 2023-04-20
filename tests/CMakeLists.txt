cider_project(tests)

cider_collect_src(LIST_OUT "models")

add_subdirectory(models)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${GTest_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${CIDER_GEN_DIR}
)

add_executable(tests ${LIST_OUT})
add_test(NAME "tests" COMMAND tests)
add_dependencies(tests ${CIDER_THIRDPARTY_TARGET_NAME})

set(GEN_DIR ${CIDER_GEN_DIR})
set(SWIG_MODULE_NAME "example")

prv_collect_src(${PROJECT_SOURCE_DIR}/models HDRS_OUT ".swig" ".cpp")

set(GEN_SRC_FILES)
foreach(HDR ${HDRS_OUT})
    get_filename_component(FILE_NAME "${HDR}" NAME_WE)
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.h")
    list(APPEND GEN_SRC_FILES "${GEN_DIR}/${FILE_NAME}.cpp")
endforeach()

cider_tool_generate(${SWIG_MODULE_NAME} "${HDRS_OUT}" ${GEN_DIR})

list(APPEND GEN_SRC_FILES "${GEN_DIR}/${SWIG_MODULE_NAME}_lua.cpp")

add_library(cider-models ${GEN_SRC_FILES})
add_dependencies(cider-models gen_swig)

cider_generate_swig(LANGUAGE lua
                    EXTERNAL_RUNTIME
                    OUTPUT_DIR "${GEN_DIR}"
                    DEPENDS gen_swig
                    FILE "${GEN_DIR}/${SWIG_MODULE_NAME}.swig")

add_library(models_wrap "${GEN_DIR}/${SWIG_MODULE_NAME}_wrap.cpp" "${GEN_DIR}/swigluarun.h")
add_dependencies(models_wrap models)

if(CLANGFORMAT_EXECUTABLE)
 foreach(OUT_FILE ${GEN_SRC_FILES})
  add_custom_command(TARGET cider-models
                   PRE_BUILD
                   COMMAND ${CLANGFORMAT_EXECUTABLE} -style=Chromium -i ${OUT_FILE})
 endforeach()
endif()

target_link_libraries(tests
    recorder
    cider-models
    models_wrap
    models
    scripting
    ${GTest_LIBRARIES}
    ${Lua_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)
