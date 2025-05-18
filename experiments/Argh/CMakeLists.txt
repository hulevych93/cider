# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(Argh)

set(EXPERIMENT_ARGH_VERSION                1.0)
set(EXPERIMENT_ARGH_UNIQUE_DIR_NAME        Argh-${EXPERIMENT_ARGH_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(EXPERIMENT_ARGH_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${EXPERIMENT_ARGH_UNIQUE_DIR_NAME})
set(EXPERIMENT_ARGH_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${EXPERIMENT_ARGH_UNIQUE_DIR_NAME})
set(Argh_INCLUDE_DIRS                   ${EXPERIMENT_ARGH_PREFIX}/include)
set(Argh_LIB_DIR                        ${EXPERIMENT_ARGH_PREFIX}/lib)
set(Argh_COMPONENTS                     argh)
set(Argh_LIBRARIES)

foreach(TMP_Argh_LIB_BASE IN LISTS Argh_COMPONENTS)
    string(TOUPPER "${TMP_Argh_LIB_BASE}" TMP_Argh_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("Argh"
                               ${TMP_Argh_LIB_BASE}
                               EXPERIMENT_ARGH_${TMP_Argh_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND Argh_LIBRARIES "${EXPERIMENT_ARGH_${TMP_Argh_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(Argh_INCLUDE_DIRS ${Argh_INCLUDE_DIRS})
cider_export_var(Argh_LIBRARIES ${Argh_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${EXPERIMENT_ARGH_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${EXPERIMENT_ARGH_PREFIX}
)

ExternalProject_Add(
    Argh
    PREFIX                  ${EXPERIMENT_ARGH_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${EXPERIMENT_ARGH_PREFIX}/stamp
    # Download
    GIT_REPOSITORY          https://github.com/hulevych93/argh.git
    GIT_TAG                 e1078ea
    UPDATE_DISCONNECTED     1
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${Argh_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} Argh)

add_subdirectory(tests)
