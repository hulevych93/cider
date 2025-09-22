# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(tinylog)

set(CIDER_TINYLOG_VERSION                1.9)
set(CIDER_TINYLOG_UNIQUE_DIR_NAME        tinylog-${CIDER_TINYLOG_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_TINYLOG_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_TINYLOG_UNIQUE_DIR_NAME})
set(CIDER_TINYLOG_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_TINYLOG_UNIQUE_DIR_NAME})
set(TinyLog_INCLUDE_DIRS                 ${CIDER_TINYLOG_PREFIX}/include)
set(TinyLog_LIB_DIR                      ${CIDER_TINYLOG_PREFIX}/lib)
set(TinyLog_COMPONENTS                   tlog)
set(TinyLog_LIBRARIES)

foreach(TMP_TinyLog_LIB_BASE IN LISTS TinyLog_COMPONENTS)
    string(TOUPPER "${TMP_TinyLog_LIB_BASE}" TMP_TinyLog_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("TinyLog"
                               ${TMP_TinyLog_LIB_BASE}
                               CIDER_FMT_${TMP_TinyLog_LIB_BASE_UPPER}_LIB
                               TRUE)

    list(APPEND TinyLog_LIBRARIES "${CIDER_FMT_${TMP_TinyLog_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(TinyLog_INCLUDE_DIRS ${TinyLog_INCLUDE_DIRS})
cider_export_var(TinyLog_LIBRARIES ${TinyLog_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_TINYLOG_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${CIDER_TINYLOG_PREFIX}
)

ExternalProject_Add(
    tinylog
    PREFIX                  ${CIDER_TINYLOG_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_TINYLOG_PREFIX}/stamp
    # Download
    GIT_REPOSITORY          https://github.com/hulevych93/tinylog.git
    GIT_TAG                 fd9f88e
    UPDATE_DISCONNECTED     1
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

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} tinylog)
