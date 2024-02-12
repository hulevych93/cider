# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(tpl)

set(CIDER_TPL_VERSION                2.0.0)
set(CIDER_TPL_UNIQUE_DIR_NAME        tpl-${CIDER_TPL_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_TPL_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_TPL_UNIQUE_DIR_NAME})
set(CIDER_TPL_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_TPL_UNIQUE_DIR_NAME})
set(Tpl_INCLUDE_DIRS                 ${CIDER_TPL_PREFIX}/include)
set(Tpl_LIB_DIR                      ${CIDER_TPL_PREFIX}/lib)
set(Tpl_COMPONENTS                   tiny-process-library)
set(Tpl_LIBRARIES)

foreach(TMP_Tpl_LIB_BASE IN LISTS Tpl_COMPONENTS)
    string(TOUPPER "${TMP_Tpl_LIB_BASE}" TMP_Tpl_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("Tpl"
                               ${TMP_Tpl_LIB_BASE}
                               CIDER_FMT_${TMP_Tpl_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND Tpl_LIBRARIES "${CIDER_FMT_${TMP_Tpl_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(Tpl_INCLUDE_DIRS ${Tpl_INCLUDE_DIRS})
cider_export_var(Tpl_LIBRARIES ${Tpl_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_TPL_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${CIDER_TPL_PREFIX}
)

ExternalProject_Add(
    tpl
    PREFIX                  ${CIDER_TPL_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_TPL_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${CIDER_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                     https://github.com/eidheim/tiny-process-library/archive/refs/tags/v2.0.0.tar.gz
    URL_MD5                 "4b137b119172ef171fbfec6e7613029d"
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

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} tpl)
