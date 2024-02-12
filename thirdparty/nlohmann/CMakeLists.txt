# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(nlohmann)

set(CIDER_NLOHMANN_VERSION                3.11.3)
set(CIDER_NLOHMANN_UNIQUE_DIR_NAME        nlohmann-${CIDER_NLOHMANN_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_NLOHMANN_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_NLOHMANN_UNIQUE_DIR_NAME})
set(CIDER_NLOHMANN_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_NLOHMANN_UNIQUE_DIR_NAME})
set(NLohMann_INCLUDE_DIRS                 ${CIDER_NLOHMANN_PREFIX}/include)

cider_export_var(NLohMann_INCLUDE_DIRS ${NLohMann_INCLUDE_DIRS})
cider_export_var(NLohMann_LIBRARIES ${NLohMann_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_NLOHMANN_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${CIDER_NLOHMANN_PREFIX}
    -DJSON_BuildTests=OFF
)

ExternalProject_Add(
    hlohmann
    PREFIX                  ${CIDER_NLOHMANN_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_NLOHMANN_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${CIDER_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                     https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz
    URL_MD5                 "d603041cbc6051edbaa02ebb82cf0aa9"
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

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} hlohmann)
