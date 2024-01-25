cider_project(hjson)

set(EXPERIMENT_HJSON_VERSION                2.4.1)
set(EXPERIMENT_HJSON_UNIQUE_DIR_NAME        hjson-${EXPERIMENT_HJSON_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(EXPERIMENT_HJSON_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${EXPERIMENT_HJSON_UNIQUE_DIR_NAME})
set(EXPERIMENT_HJSON_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${EXPERIMENT_HJSON_UNIQUE_DIR_NAME})
set(Hjson_INCLUDE_DIRS                   ${EXPERIMENT_HJSON_PREFIX}/include)
set(Hjson_LIB_DIR                        ${EXPERIMENT_HJSON_PREFIX}/lib)
set(Hjson_COMPONENTS                     hjson${CIDER_LIB_NAME_DEBUG_SUFFIX})
set(Hjson_LIBRARIES)

foreach(TMP_Hjson_LIB_BASE IN LISTS Hjson_COMPONENTS)
    string(TOUPPER "${TMP_Hjson_LIB_BASE}" TMP_Hjson_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("Hjson"
                               ${TMP_Hjson_LIB_BASE}
                               EXPERIMENT_HJSON_${TMP_Hjson_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND Hjson_LIBRARIES "${EXPERIMENT_HJSON_${TMP_Hjson_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(Hjson_INCLUDE_DIRS ${Hjson_INCLUDE_DIRS})
cider_export_var(Hjson_LIBRARIES ${Hjson_LIBRARIES})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${EXPERIMENT_HJSON_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${EXPERIMENT_HJSON_PREFIX}
    -DHJSON_ENABLE_INSTALL=TRUE
)

ExternalProject_Add(
    hjson
    PREFIX                  ${EXPERIMENT_HJSON_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${EXPERIMENT_HJSON_PREFIX}/stamp
    # Download
    GIT_REPOSITORY          https://github.com/hulevych93/hjson-cpp
    GIT_TAG                 2d93a8ee
    UPDATE_DISCONNECTED     1
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${Hjson_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} hjson)

add_subdirectory(tests)
