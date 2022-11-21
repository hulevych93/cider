gunit_project(gtest)

set(GUNIT_GTEST_VERSION                1.10.0)
set(GUNIT_GTEST_UNIQUE_DIR_NAME        gtest-${GUNIT_GTEST_VERSION}-${GUNIT_3RD_PARTY_LIB_DIR_SUFFIX})
set(GUNIT_GTEST_PREFIX                 ${GUNIT_3RD_PARTY_INSTALL_DIR}/${GUNIT_GTEST_UNIQUE_DIR_NAME})
set(GUNIT_GTEST_BUILD_DIR              ${GUNIT_3RD_PARTY_BUILD_DIR}/${GUNIT_GTEST_UNIQUE_DIR_NAME})
set(GTest_INCLUDE_DIRS                   ${GUNIT_GTEST_PREFIX}/include)
set(GTest_LIB_DIR                        ${GUNIT_GTEST_PREFIX}/lib)
set(GTest_COMPONENTS                     gtest${GUNIT_LIB_NAME_DEBUG_SUFFIX} gmock${GUNIT_LIB_NAME_DEBUG_SUFFIX})
set(GTest_LIBRARIES)

foreach(TMP_GTEST_LIB_BASE IN LISTS GTest_COMPONENTS)
    string(TOUPPER "${TMP_GTEST_LIB_BASE}" TMP_GTEST_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_name("GTest"
                               ${TMP_GTEST_LIB_BASE}
                               GUNIT_GTEST_${TMP_GTEST_LIB_BASE_UPPER}_LIB)

    list(APPEND GTest_LIBRARIES "${GUNIT_GTEST_${TMP_GTEST_LIB_BASE_UPPER}_LIB}")
endforeach()

gunit_export_var(GTest_INCLUDE_DIRS ${GTest_INCLUDE_DIRS})
gunit_export_var(GTest_LIBRARIES ${GTest_LIBRARIES})

set(TMP_DIR         ${GUNIT_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${GUNIT_GTEST_BUILD_DIR})

gunit_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DTHREADS_PTHREAD_ARG=0
    -DCMAKE_INSTALL_PREFIX=${GUNIT_GTEST_PREFIX}
    -Dgtest_force_shared_crt=ON
)

ExternalProject_Add(
    gtest
    PREFIX                  ${GUNIT_GTEST_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${GUNIT_GTEST_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${GUNIT_3RD_PARTY_DOWNLOAD_DIR}
    URL                     https://github.com/google/googletest/archive/release-1.10.0.tar.gz
    URL_MD5                 "ecd1fa65e7de707cd5c00bdac56022cd"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${GTest_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${GUNIT_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${GUNIT_THIRDPARTY_TARGET_NAME} gtest)
