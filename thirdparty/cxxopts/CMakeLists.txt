cider_project(cxxopts)

set(CIDER_CXXOPTS_VERSION                2.2.1)
set(CIDER_CXXOPTS_UNIQUE_DIR_NAME        cxxopts-${CIDER_CXXOPTS_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_CXXOPTS_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_CXXOPTS_UNIQUE_DIR_NAME})
set(CIDER_CXXOPTS_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_CXXOPTS_UNIQUE_DIR_NAME})
set(CxxOpts_INCLUDE_DIRS                 ${CIDER_CXXOPTS_PREFIX}/include)

cider_export_var(CxxOpts_INCLUDE_DIRS ${CxxOpts_INCLUDE_DIRS})
cider_export_var(CxxOpts_LIBRARIES ${CxxOpts_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_CXXOPTS_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCXXOPTS_BUILD_TESTS=OFF
    -DCMAKE_INSTALL_PREFIX=${CIDER_CXXOPTS_PREFIX}
)

ExternalProject_Add(
    cxxopts
    PREFIX                  ${CIDER_CXXOPTS_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_CXXOPTS_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${CIDER_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                     https://github.com/jarro2783/cxxopts/archive/v2.2.1.zip
    URL_MD5                 "74dfd3b72c55a06b714f26b620d53cbb"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    # Install
    INSTALL_COMMAND         ${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} cxxopts)
