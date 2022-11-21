gunit_project(swig)

set(GUNIT_SWIG_VERSION                4.1.0)
set(GUNIT_SWIG_UNIQUE_DIR_NAME        swig-${GUNIT_SWIG_VERSION}-${GUNIT_3RD_PARTY_LIB_DIR_SUFFIX})
set(GUNIT_SWIG_PREFIX                 ${GUNIT_3RD_PARTY_INSTALL_DIR}/${GUNIT_SWIG_UNIQUE_DIR_NAME})
set(GUNIT_SWIG_BUILD_DIR              ${GUNIT_3RD_PARTY_BUILD_DIR}/${GUNIT_SWIG_UNIQUE_DIR_NAME})

gunit_export_var(GUNIT_SWIG_EXECUTABLE ${GUNIT_SWIG_PREFIX}/bin/swig)
set(ENV{SWIG_LIB} ${GUNIT_SWIG_PREFIX}/share/swig/${GUNIT_SWIG_VERSION})

set(TMP_DIR         ${GUNIT_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${GUNIT_SWIG_BUILD_DIR})

gunit_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${GUNIT_SWIG_PREFIX}
)

function(gunit_generate_swig)
    set(OPTIONS EXTERNAL_RUNTIME)
    set(ONE_VALUE_ARGS LANGUAGE FLAGS OUTPUT_DIR)
    set(MULTI_VALUE_ARGS GENERATOR_ARGS DEFINES SWIG_FILES)

    cmake_parse_arguments(SWIG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})
    gunit_add_cmake_deps(${SWIG_SWIG_FILES})
    set(TEMP_DIR "${GUNIT_TMP_DIR}/swig_gen")

    # This ensures that there is no files from previous cmake runs.
    file(REMOVE_RECURSE "${TEMP_DIR}")
    file(MAKE_DIRECTORY "${TEMP_DIR}")

    foreach(FILE ${SWIG_SWIG_FILES})
        list(REMOVE_ITEM REDUNDANT_OUTPUT_FILES "${FILE}")
        get_filename_component(FILE_NAME "${FILE}" NAME_WE)
        get_filename_component(SWIG_DIRECTORY ${GUNIT_SWIG_EXECUTABLE} DIRECTORY)
        execute_process(COMMAND "${GUNIT_SWIG_EXECUTABLE}" -c++ -${SWIG_LANGUAGE} ${SWIG_GENERATOR_ARGS}
                        -cppext cpp "-I${CMAKE_SOURCE_DIR}"
                        -outdir "${TEMP_DIR}"
                        -o "${TEMP_DIR}/${FILE_NAME}_wrap.cpp"
                        -oh "${TEMP_DIR}/${FILE_NAME}_wrap.h"
                        ${SWIG_DEFINES}
                        ${FILE}
                WORKING_DIRECTORY ${SWIG_DIRECTORY}
                COMMAND_ERROR_IS_FATAL ANY)

        if(SWIG_EXTERNAL_RUNTIME)
            execute_process(COMMAND "${GUNIT_SWIG_EXECUTABLE}" -c++ -${SWIG_LANGUAGE} ${SWIG_FLAGS} -cppext cpp
                            -external-runtime "${TEMP_DIR}/swig${SWIG_LANGUAGE}run.h"
                    WORKING_DIRECTORY ${SWIG_DIRECTORY}
                    COMMAND_ERROR_IS_FATAL ANY)
        endif()
    endforeach()

    file(REMOVE_RECURSE "${TEMP_DIR}")
    file(GLOB_RECURSE OUTPUT_FILES "${SWIG_OUTPUT_DIR}/*.cpp")
endfunction()

ExternalProject_Add(
    swig
    PREFIX                  ${GUNIT_SWIG_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${GUNIT_SWIG_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${GUNIT_3RD_PARTY_DOWNLOAD_DIR}
    URL                     https://github.com/swig/swig/archive/refs/tags/v4.1.0.tar.gz
    URL_MD5                 "794433378154eb61270a3ac127d9c5f3"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${Swig_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${GUNIT_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${GUNIT_THIRDPARTY_TARGET_NAME} swig)
