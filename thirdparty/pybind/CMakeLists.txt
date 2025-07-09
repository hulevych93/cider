# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pybind)

set(CIDER_PYBIND_VERSION                2.13.6)
set(CIDER_PYBIND_UNIQUE_DIR_NAME        pybind-${CIDER_PYBIND_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_PYBIND_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_PYBIND_UNIQUE_DIR_NAME})
set(CIDER_PYBIND_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_PYBIND_UNIQUE_DIR_NAME})
set(PyBind_INCLUDE_DIRS                 ${CIDER_PYBIND_PREFIX}/include)

cider_export_var(PyBind_INCLUDE_DIRS ${PyBind_INCLUDE_DIRS})
cider_export_var(PyBind_LIBRARIES ${PyBind_LIBRARIES})

set(TMP_DIR         ${PYBIND_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_PYBIND_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${CIDER_PYBIND_PREFIX}
)

ExternalProject_Add(
    pybind
    PREFIX                  ${CIDER_PYBIND_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_PYBIND_PREFIX}/stamp
    # Download
    GIT_REPOSITORY          https://github.com/pybind/pybind11.git
    GIT_TAG                 a2e59f0

    UPDATE_DISCONNECTED     1
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    # Install
    INSTALL_COMMAND         ${PYBIND_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} pybind)
