gunit_project(cppast)

set(GUNIT_CPPAST_VERSION                3fb7c24)
set(GUNIT_CPPAST_UNIQUE_DIR_NAME        cppast-${GUNIT_CPPAST_VERSION}-${GUNIT_3RD_PARTY_LIB_DIR_SUFFIX})
set(GUNIT_CPPAST_PREFIX                 ${GUNIT_3RD_PARTY_INSTALL_DIR}/${GUNIT_CPPAST_UNIQUE_DIR_NAME})
set(GUNIT_CPPAST_BUILD_DIR              ${GUNIT_3RD_PARTY_BUILD_DIR}/${GUNIT_CPPAST_UNIQUE_DIR_NAME})
set(CppAst_INCLUDE_DIRS                 ${GUNIT_CPPAST_PREFIX}/include)
set(CppAst_LIB_DIR                      ${GUNIT_CPPAST_PREFIX}/lib)
set(CppAst_COMPONENTS                   cppast)
set(CppAst_LIBRARIES)

foreach(TMP_CppAst_LIB_BASE IN LISTS CppAst_COMPONENTS)
    string(TOUPPER "${TMP_CppAst_LIB_BASE}" TMP_CppAst_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_name("CppAst"
                               ${TMP_CppAst_LIB_BASE}
                               GUNIT_CPPAST_${TMP_CppAst_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND CppAst_LIBRARIES "${GUNIT_CPPAST_${TMP_CppAst_LIB_BASE_UPPER}_LIB}")
endforeach()

gunit_export_var(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} ${LIBCLANG_LIBRARY})

gunit_export_var(CppAst_INCLUDE_DIRS ${CppAst_INCLUDE_DIRS})
gunit_export_var(CppAst_LIBRARIES ${CppAst_LIBRARIES})

set(TMP_DIR         ${GUNIT_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${GUNIT_CPPAST_BUILD_DIR})

gunit_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${GUNIT_CPPAST_PREFIX}
)

ExternalProject_Add(
    cppast
    PREFIX                  ${GUNIT_CPPAST_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${GUNIT_CPPAST_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${GUNIT_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    GIT_REPOSITORY          https://github.com/foonathan/cppast.git
    GIT_TAG                 3fb7c24
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${CppAst_LIBRARIES}
    # Install
    INSTALL_COMMAND         "${GUNIT_3RD_PARTY_CMAKE_INSTALL_COMMAND}"
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

ExternalProject_Add_Step(cppast
    after_install
    DEPENDEES install
    COMMAND ${CMAKE_COMMAND} -E copy "${GUNIT_CPPAST_BUILD_DIR}/_deps/type_safe-src/external/debug_assert/debug_assert.hpp" "${CppAst_INCLUDE_DIRS}/debug_assert.hpp"
)

add_dependencies(${GUNIT_THIRDPARTY_TARGET_NAME} cppast)
