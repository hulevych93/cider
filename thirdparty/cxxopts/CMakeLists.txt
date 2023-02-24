gunit_project(cxxopts)

set(GUNIT_CXXOPTS_VERSION                2.2.1)
set(GUNIT_CXXOPTS_UNIQUE_DIR_NAME        cxxopts-${GUNIT_CXXOPTS_VERSION}-${GUNIT_3RD_PARTY_LIB_DIR_SUFFIX})
set(GUNIT_CXXOPTS_PREFIX                 ${GUNIT_3RD_PARTY_INSTALL_DIR}/${GUNIT_CXXOPTS_UNIQUE_DIR_NAME})
set(GUNIT_CXXOPTS_BUILD_DIR              ${GUNIT_3RD_PARTY_BUILD_DIR}/${GUNIT_CXXOPTS_UNIQUE_DIR_NAME})
set(CxxOpts_INCLUDE_DIRS                 ${GUNIT_CXXOPTS_PREFIX}/include)

gunit_export_var(CxxOpts_INCLUDE_DIRS ${CxxOpts_INCLUDE_DIRS})
gunit_export_var(CxxOpts_LIBRARIES ${CxxOpts_LIBRARIES})

set(TMP_DIR         ${GUNIT_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${GUNIT_CXXOPTS_BUILD_DIR})

gunit_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCXXOPTS_BUILD_TESTS=OFF
    -DCMAKE_INSTALL_PREFIX=${GUNIT_CXXOPTS_PREFIX}
)

ExternalProject_Add(
    cxxopts
    PREFIX                  ${GUNIT_CXXOPTS_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${GUNIT_CXXOPTS_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${GUNIT_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                     https://github.com/jarro2783/cxxopts/archive/v2.2.1.zip
    URL_MD5                 "74dfd3b72c55a06b714f26b620d53cbb"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    # Install
    INSTALL_COMMAND         ${GUNIT_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${GUNIT_THIRDPARTY_TARGET_NAME} cxxopts)
