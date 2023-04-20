cider_project(swig)

set(CIDER_SWIG_VERSION                4.1.0)
set(CIDER_SWIG_UNIQUE_DIR_NAME        swig-${CIDER_SWIG_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_SWIG_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_SWIG_UNIQUE_DIR_NAME})
set(CIDER_SWIG_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_SWIG_UNIQUE_DIR_NAME})

cider_export_var(CIDER_SWIG_EXECUTABLE ${CIDER_SWIG_PREFIX}/bin/swig)
set(ENV{SWIG_LIB} ${CIDER_SWIG_PREFIX}/share/swig/${CIDER_SWIG_VERSION})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_SWIG_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DWITH_PCRE=OFF
    -DCMAKE_INSTALL_PREFIX=${CIDER_SWIG_PREFIX}
)

find_package(BISON)
if(NOT BISON_FOUND)
    message(FATAL_ERROR "Please, install BISON to be able to build SWIG")
endif()

function(cider_generate_swig)
    set(OPTIONS EXTERNAL_RUNTIME)
    set(ONE_VALUE_ARGS LANGUAGE FLAGS OUTPUT_DIR DEPENDS FILE)
    set(MULTI_VALUE_ARGS GENERATOR_ARGS DEFINES)

    cmake_parse_arguments(SWIG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # This ensures that there is no files from previous cmake runs.
    file(REMOVE_RECURSE "${SWIG_OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${SWIG_OUTPUT_DIR}")

    get_filename_component(FILE_NAME "${SWIG_FILE}" NAME_WE)
    get_filename_component(SWIG_DIRECTORY ${CIDER_SWIG_EXECUTABLE} DIRECTORY)

    add_custom_command(OUTPUT "${SWIG_OUTPUT_DIR}/${FILE_NAME}_wrap.cpp" "${SWIG_OUTPUT_DIR}/${FILE_NAME}_wrap.h"
                       COMMAND "${CIDER_SWIG_EXECUTABLE}" -c++ -${SWIG_LANGUAGE} ${SWIG_GENERATOR_ARGS}
                      -cppext cpp "-I${CMAKE_SOURCE_DIR}"
                      -outdir "${SWIG_OUTPUT_DIR}"
                      -w509 # Warning: Overloaded method effectively ignored
                      -o "${SWIG_OUTPUT_DIR}/${FILE_NAME}_wrap.cpp"
                      -oh "${SWIG_OUTPUT_DIR}/${FILE_NAME}_wrap.h"
                      -xmlout "${SWIG_OUTPUT_DIR}/${FILE_NAME}_wrap.xml"
                      ${SWIG_DEFINES}
                      ${SWIG_FILE}
              WORKING_DIRECTORY ${SWIG_DIRECTORY}
              DEPENDS ${SWIG_DEPENDS}
              COMMENT "Generating swig wrapper files...")

    if(SWIG_EXTERNAL_RUNTIME)
      add_custom_command(OUTPUT "${SWIG_OUTPUT_DIR}/swig${SWIG_LANGUAGE}run.h"
                COMMAND "${CIDER_SWIG_EXECUTABLE}" -c++ -${SWIG_LANGUAGE} ${SWIG_FLAGS} -cppext cpp
                        -external-runtime "${SWIG_OUTPUT_DIR}/swig${SWIG_LANGUAGE}run.h"
                  WORKING_DIRECTORY ${SWIG_DIRECTORY}
                  COMMENT "Generating swig runtime headers...")
      endif()
endfunction()

ExternalProject_Add(
    swig
    PREFIX                  ${CIDER_SWIG_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_SWIG_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${CIDER_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                     https://github.com/swig/swig/archive/refs/tags/v4.1.0.tar.gz
    URL_MD5                 "794433378154eb61270a3ac127d9c5f3"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${Swig_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} swig)
