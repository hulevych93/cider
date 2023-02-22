gunit_project(boost)

set(GUNIT_BOOST_VERSION 1.70.0)
set(GUNIT_BOOST_ROOT_DIR boost-${GUNIT_BOOST_VERSION}-${GUNIT_3RD_PARTY_LIB_DIR_SUFFIX})
set(GUNIT_BOOST_BUILD_DIR ${GUNIT_3RD_PARTY_BUILD_DIR}/${GUNIT_BOOST_ROOT_DIR})
set(GUNIT_BOOST_INSTALL_PREFIX ${GUNIT_3RD_PARTY_INSTALL_DIR}/${GUNIT_BOOST_ROOT_DIR})
set(Boost_INCLUDE_DIRS ${GUNIT_BOOST_INSTALL_PREFIX}/include)

gunit_export_var(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})

set(TMP_DIR         ${GUNIT_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${GUNIT_BOOST_BUILD_DIR})

set(BUILD_TOOLSET_VERSION ${CMAKE_CXX_COMPILER_VERSION})

if(CMAKE_COMPILER_IS_GNUCC)
    set(BUILD_TOOLSET gcc)
elseif(MSVC)
    set(BUILD_TOOLSET msvc)
    set(BUILD_TOOLSET_VERSION 14.1)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES .*Clang.*)
    set(BUILD_TOOLSET clang)
else()
    message(FATAL_ERROR "Unknown toolset for system: ${CMAKE_SYSTEM_NAME}")
endif()

set(BUILD_VARIANT release)
if("${CMAKE_BUILD_TYPE}" STREQUAL DEBUG)
    set(BUILD_VARIANT debug)
endif()

set(BOOTSTRAP_COMMAND ${SRC_DIR}/bootstrap.sh)
if(CMAKE_HOST_WIN32)
    if(MSVC)
        set(BOOTSTRAP_COMMAND ${SRC_DIR}/bootstrap.bat vc141)
    else()
        set(BOOTSTRAP_COMMAND ${SRC_DIR}/bootstrap.bat gcc)
    endif()
endif()

set(BJAM_COMMAND "${SRC_DIR}/b2${GUNIT_HOST_EXECUTABLE_SUFFIX}")

set(BUILD_COMMAND_BASE
    ${BJAM_COMMAND}
        --build-type=minimal
        --with-headers
        -sNO_BZIP2=1 # needed when building iostreams
        -sNO_LZMA=1 # needed when building iostreams
        --hash
        ${NO_EVENT_LOG}
        toolset=${BUILD_TOOLSET}-${BUILD_TOOLSET_VERSION}
        variant=${BUILD_VARIANT}
        threading=multi
)

ExternalProject_Add(
    boost
    PREFIX              ${GUNIT_BOOST_INSTALL_PREFIX}
    TMP_DIR             ${TMP_DIR}
    STAMP_DIR           ${GUNIT_BOOST_INSTALL_PREFIX}/stamp
    # Download
    DOWNLOAD_DIR        ${GUNIT_3RD_PARTY_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP 0
    URL                 https://sourceforge.net/projects/boost/files/boost/1.70.0/boost_1_70_0.tar.bz2/download
    URL_MD5            "242ecc63507711d6706b9b0c0d0c7d4f"
    # Configure
    SOURCE_DIR          ${SRC_DIR}
    CONFIGURE_COMMAND   ${BOOTSTRAP_COMMAND}
    # Build
    BUILD_COMMAND       ${BUILD_COMMAND_BASE} --prefix=${GUNIT_BOOST_INSTALL_PREFIX} -j8 -d2 ${LIB_OPTS} install
        # Install
    INSTALL_COMMAND     ""
    BUILD_IN_SOURCE     1

    # Logging
    LOG_CONFIGURE       1
    LOG_BUILD           1
    LOG_INSTALL         1
    LOG_TEST            1
)

add_dependencies(${GUNIT_THIRDPARTY_TARGET_NAME} boost)
