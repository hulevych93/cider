cider_project(gtest)

set(CIDER_GTEST_VERSION                1.10.0)
set(CIDER_GTEST_UNIQUE_DIR_NAME        gtest-${CIDER_GTEST_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_GTEST_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_GTEST_UNIQUE_DIR_NAME})
set(CIDER_GTEST_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_GTEST_UNIQUE_DIR_NAME})
set(GTest_INCLUDE_DIRS                   ${CIDER_GTEST_PREFIX}/include)
set(GTest_LIB_DIR                        ${CIDER_GTEST_PREFIX}/lib)
set(GTest_COMPONENTS                     gtest${CIDER_LIB_NAME_DEBUG_SUFFIX} gmock${CIDER_LIB_NAME_DEBUG_SUFFIX})
set(GTest_LIBRARIES)

foreach(TMP_GTEST_LIB_BASE IN LISTS GTest_COMPONENTS)
    string(TOUPPER "${TMP_GTEST_LIB_BASE}" TMP_GTEST_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("GTest"
                               ${TMP_GTEST_LIB_BASE}
                               CIDER_GTEST_${TMP_GTEST_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND GTest_LIBRARIES "${CIDER_GTEST_${TMP_GTEST_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(GTest_INCLUDE_DIRS ${GTest_INCLUDE_DIRS})
cider_export_var(GTest_LIBRARIES ${GTest_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_GTEST_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DTHREADS_PTHREAD_ARG=0
    -DCMAKE_INSTALL_PREFIX=${CIDER_GTEST_PREFIX}
    -Dgtest_force_shared_crt=ON
)

ExternalProject_Add(
    gtest
    PREFIX                  ${CIDER_GTEST_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_GTEST_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${CIDER_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                     https://github.com/google/googletest/archive/release-1.10.0.tar.gz
    URL_MD5                 "ecd1fa65e7de707cd5c00bdac56022cd"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${GTest_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} gtest)
