cider_project(fmt)

set(CIDER_FMT_VERSION                9.0.0)
set(CIDER_FMT_UNIQUE_DIR_NAME        fmt-${CIDER_FMT_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_FMT_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_FMT_UNIQUE_DIR_NAME})
set(CIDER_FMT_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_FMT_UNIQUE_DIR_NAME})
set(Fmt_INCLUDE_DIRS                   ${CIDER_FMT_PREFIX}/include)
set(Fmt_LIB_DIR                        ${CIDER_FMT_PREFIX}/lib)
set(Fmt_COMPONENTS                     fmt${CIDER_LIB_NAME_DEBUG_SUFFIX})
set(Fmt_LIBRARIES)

foreach(TMP_Fmt_LIB_BASE IN LISTS Fmt_COMPONENTS)
    string(TOUPPER "${TMP_Fmt_LIB_BASE}" TMP_Fmt_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("Fmt"
                               ${TMP_Fmt_LIB_BASE}
                               CIDER_FMT_${TMP_Fmt_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND Fmt_LIBRARIES "${CIDER_FMT_${TMP_Fmt_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(Fmt_INCLUDE_DIRS ${Fmt_INCLUDE_DIRS})
cider_export_var(Fmt_LIBRARIES ${Fmt_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_FMT_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${CIDER_FMT_PREFIX}
)

ExternalProject_Add(
    fmt
    PREFIX                  ${CIDER_FMT_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_FMT_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${CIDER_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                     https://github.com/fmtlib/fmt/archive/refs/tags/9.0.0.tar.gz
    URL_MD5                 "d56c8b0612b049bb1854f07c8b133f3c"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${Fmt_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} fmt)
