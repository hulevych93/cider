# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++)

set(EXPERIMENT_BITMAPPLUSPLUS_VERSION                1.1.1)
set(EXPERIMENT_BITMAPPLUSPLUS_UNIQUE_DIR_NAME        bitmap++-${EXPERIMENT_BITMAPPLUSPLUS_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(EXPERIMENT_BITMAPPLUSPLUS_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${EXPERIMENT_BITMAPPLUSPLUS_UNIQUE_DIR_NAME})
set(EXPERIMENT_BITMAPPLUSPLUS_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${EXPERIMENT_BITMAPPLUSPLUS_UNIQUE_DIR_NAME})
set(BitmapPlusPlus_INCLUDE_DIRS                   ${EXPERIMENT_BITMAPPLUSPLUS_PREFIX}/include)
set(BitmapPlusPlus_LIB_DIR                        ${EXPERIMENT_BITMAPPLUSPLUS_PREFIX}/lib)
set(BitmapPlusPlus_COMPONENTS                     BitmapPlusPlus)
set(BitmapPlusPlus_LIBRARIES)

foreach(TMP_BitmapPlusPlus_LIB_BASE IN LISTS BitmapPlusPlus_COMPONENTS)
    string(TOUPPER "${TMP_BitmapPlusPlus_LIB_BASE}" TMP_BitmapPlusPlus_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("BitmapPlusPlus"
                               ${TMP_BitmapPlusPlus_LIB_BASE}
                               EXPERIMENT_BITMAPPLUSPLUS_${TMP_BitmapPlusPlus_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND BitmapPlusPlus_LIBRARIES "${EXPERIMENT_BITMAPPLUSPLUS_${TMP_BitmapPlusPlus_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(BitmapPlusPlus_INCLUDE_DIRS ${BitmapPlusPlus_INCLUDE_DIRS})
cider_export_var(BitmapPlusPlus_LIBRARIES ${BitmapPlusPlus_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${EXPERIMENT_BITMAPPLUSPLUS_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${EXPERIMENT_BITMAPPLUSPLUS_PREFIX}
    -DBPP_ENABLE_INSTALL=ON
)

ExternalProject_Add(
    bitmap++
    PREFIX                  ${EXPERIMENT_BITMAPPLUSPLUS_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${EXPERIMENT_BITMAPPLUSPLUS_PREFIX}/stamp
    # Download
    GIT_REPOSITORY          https://github.com/hulevych93/BitmapPlusPlus.git
    GIT_TAG                 68c24c9
    UPDATE_DISCONNECTED     1
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${BitmapPlusPlus_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} bitmap++)

add_subdirectory(tests)

set(CMD_BITMAP
    "${CMAKE_SOURCE_DIR}/experiments/bitmap++/tests/bitmap++_tests"
    "${EXPERIMENT_BITMAPPLUSPLUS_BUILD_DIR}/src"
    "${CMAKE_BINARY_DIR}/experiments/bitmap++/tests/bitmap++_player"
    "${CMAKE_BINARY_DIR}/bin/bitmap++_player"
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}/results/bitmap++/"
    "${CMAKE_BINARY_DIR}/results/"
)
set(CMD_BITMAP ${CMD_BITMAP} PARENT_SCOPE)

list(APPEND LIBS "bitmap++\;CMD_BITMAP")
set(LIBS "${LIBS}" PARENT_SCOPE)
