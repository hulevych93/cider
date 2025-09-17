# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(mathplotlib)

set(CIDER_MATHPLOT_VERSION                1.0.0)
set(CIDER_MATHPLOT_UNIQUE_DIR_NAME        mathplot-${CIDER_MATHPLOT_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_MATHPLOT_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_MATHPLOT_UNIQUE_DIR_NAME})
set(CIDER_MATHPLOT_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_MATHPLOT_UNIQUE_DIR_NAME})
set(MathPlot_INCLUDE_DIRS                 ${CIDER_MATHPLOT_PREFIX}/include)

cider_export_var(MathPlot_INCLUDE_DIRS ${MathPlot_INCLUDE_DIRS})
cider_export_var(MathPlot_LIBRARIES ${MathPlot_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_MATHPLOT_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${CIDER_MATHPLOT_PREFIX}
)

ExternalProject_Add(
    mathplotlib
    PREFIX                  ${CIDER_MATHPLOT_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_MATHPLOT_PREFIX}/stamp
    # Download
    GIT_REPOSITORY          https://github.com/hulevych93/matplotlib-cpp.git
    GIT_TAG                 9b20147
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

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} mathplotlib)
