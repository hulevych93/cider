gunit_project(lua)

set(GUNIT_LUA_VERSION                5.3.2)
set(GUNIT_LUA_UNIQUE_DIR_NAME        lua-${GUNIT_LUA_VERSION}-${GUNIT_3RD_PARTY_LIB_DIR_SUFFIX})
set(GUNIT_LUA_PREFIX                 ${GUNIT_3RD_PARTY_INSTALL_DIR}/${GUNIT_LUA_UNIQUE_DIR_NAME})
set(GUNIT_LUA_BUILD_DIR              ${GUNIT_3RD_PARTY_BUILD_DIR}/${GUNIT_LUA_UNIQUE_DIR_NAME})
set(Lua_INCLUDE_DIRS                   ${GUNIT_LUA_PREFIX}/include)
set(Lua_LIB_DIR                        ${GUNIT_LUA_PREFIX}/lib)
set(Lua_COMPONENTS                     lua)
set(Lua_LIBRARIES)

foreach(TMP_Lua_LIB_BASE IN LISTS Lua_COMPONENTS)
    string(TOUPPER "${TMP_Lua_LIB_BASE}" TMP_Lua_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("Lua"
                               ${TMP_Lua_LIB_BASE}
                               GUNIT_LUA_${TMP_Lua_LIB_BASE_UPPER}_LIB
                               FALSE)

    list(APPEND Lua_LIBRARIES "${GUNIT_LUA_${TMP_Lua_LIB_BASE_UPPER}_LIB}")
endforeach()

gunit_export_var(Lua_INCLUDE_DIRS ${Lua_INCLUDE_DIRS})
gunit_export_var(Lua_LIBRARIES ${Lua_LIBRARIES})

set(TMP_DIR         ${GUNIT_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${GUNIT_LUA_BUILD_DIR})

gunit_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DCMAKE_INSTALL_PREFIX=${GUNIT_LUA_PREFIX}
)

ExternalProject_Add(
    lua
    PREFIX                  ${GUNIT_LUA_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${GUNIT_LUA_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR            ${GUNIT_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                     https://drive.google.com/uc?export=download&id=1xC3C4-TW2RTNqrYHGVcNr32lNbjXKqVQ
    URL_MD5                 "e505b44513201ee8401f0c056e0ec8fa"
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${Lua_LIBRARIES}
    # Install
    INSTALL_COMMAND         ${GUNIT_3RD_PARTY_CMAKE_INSTALL_COMMAND}
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

add_dependencies(${GUNIT_THIRDPARTY_TARGET_NAME} lua)
