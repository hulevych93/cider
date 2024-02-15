# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pugixml)

set(EXPERIMENT_PUGIXML_VERSION                1.1.0)
set(EXPERIMENT_PUGIXML_UNIQUE_DIR_NAME        pugixml-${EXPERIMENT_PUGIXML_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(EXPERIMENT_PUGIXML_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${EXPERIMENT_PUGIXML_UNIQUE_DIR_NAME})
set(EXPERIMENT_PUGIXML_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${EXPERIMENT_PUGIXML_UNIQUE_DIR_NAME})
set(Pugixml_INCLUDE_DIRS                   ${EXPERIMENT_PUGIXML_PREFIX}/include)
set(Pugixml_LIB_DIR                        ${EXPERIMENT_PUGIXML_PREFIX}/lib)
set(Pugixml_COMPONENTS                     hjson${CIDER_LIB_NAME_DEBUG_SUFFIX})
set(Pugixml_LIBRARIES)

foreach(TMP_Pugixml_LIB_BASE IN LISTS Pugixml_COMPONENTS)
    string(TOUPPER "${TMP_Pugixml_LIB_BASE}" TMP_Pugixml_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("Pugixml"
                               ${TMP_Pugixml_LIB_BASE}
                               EXPERIMENT_PUGIXML_${TMP_Pugixml_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND Pugixml_LIBRARIES "${EXPERIMENT_PUGIXML_${TMP_Pugixml_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(Pugixml_INCLUDE_DIRS ${Pugixml_INCLUDE_DIRS})
cider_export_var(Pugixml_LIBRARIES ${Pugixml_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${EXPERIMENT_PUGIXML_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${EXPERIMENT_PUGIXML_PREFIX}
    -DPUGIXML_NO_STL=TRUE
    -DPUGIXML_NO_XPATH=TRUE
    -DPUGIXML_HAS_MOVE=FALSE
)

ExternalProject_Add(
    pugixml
    PREFIX                  ${EXPERIMENT_PUGIXML_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${EXPERIMENT_PUGIXML_PREFIX}/stamp
    # Download
    GIT_REPOSITORY          https://github.com/hulevych93/pugixml.git
    UPDATE_DISCONNECTED     1
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${Pugixml_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} pugixml)

add_subdirectory(tests)
