# thirdparties.cmake

include(ExternalProject)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(GUNIT_ARCH x86_64)
    add_definitions(-Dx64)
elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL 4)
    set(GUNIT_ARCH x86)
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES .*Clang.*)
    set(GUNIT_COMPILER_IS_CLANG TRUE)
endif()

if(WIN32)
    gunit_export_var(GUNIT_LIB_NAME_PREFIX)
else()
    gunit_export_var(GUNIT_LIB_NAME_PREFIX lib)
endif()

if(APPLE)
    gunit_export_var(GUNIT_SHARED_LIB_NAME_SUFFIX .dylib)
elseif(UNIX)
    gunit_export_var(GUNIT_SHARED_LIB_NAME_SUFFIX .so)
elseif(WIN32)
    gunit_export_var(GUNIT_SHARED_LIB_NAME_SUFFIX .dll)
endif()

if(WIN32)
    gunit_export_var(GUNIT_STATIC_LIB_NAME_SUFFIX .lib)
else()
    gunit_export_var(GUNIT_STATIC_LIB_NAME_SUFFIX .a)
endif()

if(BUILD_SHARED_LIBS)
    gunit_export_var(GUNIT_LIB_NAME_SUFFIX ${GUNIT_SHARED_LIB_NAME_SUFFIX})
else()
    gunit_export_var(GUNIT_LIB_NAME_SUFFIX ${GUNIT_STATIC_LIB_NAME_SUFFIX})
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL DEBUG)
    gunit_export_var(GUNIT_LIB_NAME_DEBUG_SUFFIX d)
    if(MSVC)
        gunit_export_var(GUNIT_LIB_NAME_DEBUG_SUFFIX_MSVC ${GUNIT_LIB_NAME_DEBUG_SUFFIX})
    endif()
    gunit_export_var(GUNIT_LIB_NAME_DEBUG_SUFFIX_WITH_MINUS -d)
endif()

unset(GUNIT_3RD_PARTY_LIB_DIR_SUFFIX CACHE)

macro(prv_add_3rd_party_lib_dir_suffix SUFFIX)
    if(GUNIT_3RD_PARTY_LIB_DIR_SUFFIX)
        set(GUNIT_3RD_PARTY_LIB_DIR_SUFFIX "${GUNIT_3RD_PARTY_LIB_DIR_SUFFIX}-${SUFFIX}")
    else()
        set(GUNIT_3RD_PARTY_LIB_DIR_SUFFIX "${SUFFIX}")
    endif()
endmacro()

if(BUILD_SHARED_LIBS)
    set(TMP_LINK_TYPE "shared")
else()
    set(TMP_LINK_TYPE "static")
endif()

string(REPLACE " " "" TMP_GENERATOR_DESC "${CMAKE_GENERATOR}")

if(NOT DEFINED GUNIT_3RD_PARTY_LIB_DIR_SUFFIX)
  prv_add_3rd_party_lib_dir_suffix("${CMAKE_BUILD_TYPE}")
  prv_add_3rd_party_lib_dir_suffix("${GUNIT_ARCH}")
  prv_add_3rd_party_lib_dir_suffix("${TMP_LINK_TYPE}")
  prv_add_3rd_party_lib_dir_suffix("${CMAKE_CXX_COMPILER_ID}")
  prv_add_3rd_party_lib_dir_suffix("${CMAKE_CXX_COMPILER_VERSION}")
  prv_add_3rd_party_lib_dir_suffix("${TMP_GENERATOR_DESC}")
endif()

gunit_export_var(GUNIT_3RD_PARTY_LIB_DIR_SUFFIX ${GUNIT_3RD_PARTY_LIB_DIR_SUFFIX})

function(prv_def_3rd_party_lib_name BASE_NAME FULL_LIB_NAME_OUT BUILD_AS_SHARED)
    if(BUILD_AS_SHARED)
        set(SUFFIX "${GUNIT_SHARED_LIB_NAME_SUFFIX}")
    else()
        set(SUFFIX "${GUNIT_STATIC_LIB_NAME_SUFFIX}")
    endif()

    set(FULL_LIB_NAME "${GUNIT_LIB_NAME_PREFIX}${BASE_NAME}${SUFFIX}")
    set(${FULL_LIB_NAME_OUT} ${FULL_LIB_NAME} PARENT_SCOPE)
endfunction()


function(prv_def_3rd_party_lib_path COMPONENT_NAME BASE_NAME FULL_LIB_NAME_OUT BUILD_AS_SHARED)
    if(BUILD_AS_SHARED)
        set(SUFFIX "${GUNIT_SHARED_LIB_NAME_SUFFIX}")
    else()
        set(SUFFIX "${GUNIT_STATIC_LIB_NAME_SUFFIX}")
    endif()

    set(FULL_LIB_NAME "${${COMPONENT_NAME}_LIB_DIR}/${GUNIT_LIB_NAME_PREFIX}${BASE_NAME}${SUFFIX}")
    set(${FULL_LIB_NAME_OUT} ${FULL_LIB_NAME} PARENT_SCOPE)
endfunction()

# VAR is a variable name for cmake flags which are to be passed to 3rd party build system.
# Concerning ARGN see documentation for prv_append_3rd_party_specific_compiler_flags.
function(gunit_3rd_party_common_cmake_options VAR)
    set(RESULT)
    list(APPEND RESULT "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")
    list(APPEND RESULT "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
    list(APPEND RESULT "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
    list(APPEND RESULT "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
    list(APPEND RESULT "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
    list(APPEND RESULT "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
    list(APPEND RESULT "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")

    # Same as we do for our code, we must append the options to the configuration-specific variables to ensure that
    # they override the defaults and not vice versa.
    macro(prv_add_config_specific_var VAR_BASE_NAME VALUE)
        # For this to work properly in multi-generator builds the options should be appended to all of the vars
        # (_DEBUG, _RELEASE etc) simultaneously, but this may produce a command line that is too long.
        # TODO: perhaps we should do it only on iOS (which is the only platform where multi-generator builds are used).
        list(APPEND RESULT "-D${VAR_BASE_NAME}_${CMAKE_BUILD_TYPE}=${VALUE}")
    endmacro()

    prv_add_config_specific_var(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    prv_add_config_specific_var(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    prv_add_config_specific_var(CMAKE_SHARED_LINKER_FLAGS "${LD_FLAGS}")
    prv_add_config_specific_var(CMAKE_MODULE_LINKER_FLAGS "${LD_FLAGS}")
    prv_add_config_specific_var(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS}")

    list(APPEND RESULT "-DCMAKE_AR=${CMAKE_AR}")
    list(APPEND RESULT "-DCMAKE_RANLIB=${CMAKE_RANLIB}")
    if(NOT "${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
        list(APPEND RESULT "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    endif()
    list(APPEND RESULT "-DBOOST_ROOT=${GUNIT_BOOST_INSTALL_PREFIX}")
    list(APPEND RESULT "-DBoost_NO_BOOST_CMAKE=TRUE")
    list(APPEND RESULT "-DBoost_NO_SYSTEM_PATHS=TRUE")
    list(APPEND RESULT "-DBOOST_INCLUDEDIR=${GUNIT_BOOST_INSTALL_PREFIX}/include")
    list(APPEND RESULT "-DBOOST_LIBRARYDIR=${GUNIT_BOOST_INSTALL_PREFIX}/lib")

    if(APPLE)
        # @loader_path/../lib path instead of just @loader_path below workarounds a problem in 3rd
        # party packages when they provide some executables which has to be run. Since variable
        # CMAKE_INSTALL_RPATH is used both for libraries and for executables @loader_path doesn't
        # work for executables when searching for own libraries in lib directory while
        # @loader_path/../lib does work for both.
        list(APPEND RESULT "-DCMAKE_INSTALL_RPATH=@loader_path/../lib")
        list(APPEND RESULT "-DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE")

        list(APPEND RESULT "-DCMAKE_INSTALL_NAME_DIR=@rpath")
        list(APPEND RESULT -DCMAKE_BUILD_WITH_INSTALL_NAME_DIR=TRUE)
    elseif(UNIX)
        # Same logic as above for linux.
        list(APPEND RESULT "-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib")
        list(APPEND RESULT -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE)
    endif()

    # The following options are used by the toolchain files.
    list(APPEND RESULT "-DGUNIT_ARCH=${GUNIT_ARCH}")

    set(${VAR} ${RESULT} PARENT_SCOPE)
endfunction()

function(gunit_3rd_party_common_autotools_options PREFIX SHARED VAR)
    set(RESULT)
    list(APPEND RESULT "--prefix=${PREFIX}")
    list(APPEND RESULT "CC=${CMAKE_C_COMPILER}")
    list(APPEND RESULT "CXX=${CMAKE_CXX_COMPILER}")
    list(APPEND RESULT "AR=${CMAKE_AR}")
    list(APPEND RESULT "RANLIB=${CMAKE_RANLIB}")
    list(APPEND RESULT "CFLAGS=${CMAKE_C_FLAGS}")
    list(APPEND RESULT "CXXFLAGS=${CMAKE_CXX_FLAGS}")
    list(APPEND RESULT "LDFLAGS=${LD_FLAGS}")
    if(WIN32)
        list(APPEND RESULT "CONFIG_SITE=${GUNIT_MSYS_CONFIG_SITE}")
    endif()

    if(SHARED)
        list(APPEND RESULT --enable-shared=yes --enable-static=no)
    else()
        list(APPEND RESULT --enable-shared=no --enable-static=yes)
    endif()

    set("${VAR}" "${RESULT}" PARENT_SCOPE)
endfunction()

# Set 3rd-parties directories
gunit_export_var(GUNIT_3RD_PARTY_DOWNLOAD_DIR "${BUILD_FILES_DIR}/Download")
gunit_export_var(GUNIT_3RD_PARTY_BUILD_DIR "${BUILD_FILES_DIR}/Build")
gunit_export_var(GUNIT_3RD_PARTY_INSTALL_DIR "${BUILD_FILES_DIR}/Install")
gunit_export_var(GUNIT_3RD_PARTY_TMP_DIR "${BUILD_FILES_DIR}/Temp")

set(GUNIT_GEN_DIR ${GUNIT_BINARY_DIR}/generated)
set(GUNIT_TMP_DIR ${GUNIT_BINARY_DIR}/tmp)

set(GUNIT_3RD_PARTY_CMAKE_INSTALL_COMMAND "${CMAKE_COMMAND}" --build . --config "${CMAKE_BUILD_TYPE}" --target install)


