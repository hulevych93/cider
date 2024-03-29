# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(cppast)

set(CIDER_CPPAST_VERSION                3fb7c24)
set(CIDER_CPPAST_UNIQUE_DIR_NAME        cppast-${CIDER_CPPAST_VERSION}-${CIDER_3RD_PARTY_LIB_DIR_SUFFIX})
set(CIDER_CPPAST_PREFIX                 ${CIDER_3RD_PARTY_INSTALL_DIR}/${CIDER_CPPAST_UNIQUE_DIR_NAME})
set(CIDER_CPPAST_BUILD_DIR              ${CIDER_3RD_PARTY_BUILD_DIR}/${CIDER_CPPAST_UNIQUE_DIR_NAME})
set(CppAst_INCLUDE_DIRS                 ${CIDER_CPPAST_PREFIX}/include)
set(CppAst_LIB_DIR                      ${CIDER_CPPAST_PREFIX}/lib)
set(CppAst_COMPONENTS                   cppast tiny-process-library)
set(CppAst_LIBRARIES)

foreach(TMP_CppAst_LIB_BASE IN LISTS CppAst_COMPONENTS)
    string(TOUPPER "${TMP_CppAst_LIB_BASE}" TMP_CppAst_LIB_BASE_UPPER)
    prv_def_3rd_party_lib_path("CppAst"
                               ${TMP_CppAst_LIB_BASE}
                               CIDER_CPPAST_${TMP_CppAst_LIB_BASE_UPPER}_LIB
                               TRUE)

    list(APPEND CppAst_LIBRARIES "${CIDER_CPPAST_${TMP_CppAst_LIB_BASE_UPPER}_LIB}")
endforeach()

cider_export_var(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} ${LIBCLANG_LIBRARY})

cider_export_var(CppAst_INCLUDE_DIRS ${CppAst_INCLUDE_DIRS})
cider_export_var(CppAst_LIBRARIES ${CppAst_LIBRARIES})
cider_export_var(CppAst_LIB_DIR ${CppAst_LIB_DIR})

set(TMP_DIR         ${CIDER_3RD_PARTY_TMP_DIR})
set(SRC_DIR         ${CIDER_CPPAST_BUILD_DIR})

cider_3rd_party_common_cmake_options(CMAKE_OPTS)

list(APPEND CMAKE_OPTS
    -DBUILD_SHARED_LIBS=TRUE
    -DCMAKE_INSTALL_PREFIX=${CIDER_CPPAST_PREFIX}
)

if(APPLE)
    find_program(BREW_EXECUTABLE brew)
    execute_process(COMMAND ${BREW_EXECUTABLE} --prefix llvm
                    OUTPUT_VARIABLE LLVM_PREFIX_BREW
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    COMMAND_ERROR_IS_FATAL ANY
    )
    list(APPEND CMAKE_OPTS
    -DLLVM_CONFIG_BINARY=${LLVM_PREFIX_BREW}@14/bin/llvm-config
    )
elseif(UNIX)
    execute_process(COMMAND which llvm-config
                    OUTPUT_VARIABLE LLVM_EXECUTABLE
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    COMMAND_ERROR_IS_FATAL ANY
    )
    list(APPEND CMAKE_OPTS
    -DLLVM_CONFIG_BINARY=${LLVM_EXECUTABLE}
    )
endif()

ExternalProject_Add(
    cppast
    PREFIX                  ${CIDER_CPPAST_PREFIX}
    TMP_DIR                 ${TMP_DIR}
    STAMP_DIR               ${CIDER_CPPAST_PREFIX}/stamp
    # Download
    GIT_REPOSITORY          https://github.com/hulevych93/cppast.git
    GIT_TAG                 f00df66
    UPDATE_DISCONNECTED     1
    # Configure
    SOURCE_DIR              ${SRC_DIR}
    CMAKE_ARGS              ${CMAKE_OPTS}
    # Build
    BUILD_IN_SOURCE         1
    BUILD_BYPRODUCTS        ${CppAst_LIBRARIES}
    # Install
    INSTALL_COMMAND         "${CIDER_3RD_PARTY_CMAKE_INSTALL_COMMAND}"
    # Logging
    LOG_CONFIGURE           1
    LOG_BUILD               1
    LOG_INSTALL             1
)

prv_def_3rd_party_lib_name("tiny-process-library"
    CIDER_CPPAST_TINY_PROCESS_LIB_NAME
    TRUE)

ExternalProject_Add_Step(cppast
    after_install
    DEPENDEES install
    COMMAND ${CMAKE_COMMAND} -E copy "${CIDER_CPPAST_BUILD_DIR}/_deps/type_safe-src/external/debug_assert/debug_assert.hpp" "${CppAst_INCLUDE_DIRS}/debug_assert.hpp"
    COMMAND ${CMAKE_COMMAND} -E copy "${CIDER_CPPAST_BUILD_DIR}/external/tpl/${CIDER_CPPAST_TINY_PROCESS_LIB_NAME}" "${CppAst_LIB_DIR}/${CIDER_CPPAST_TINY_PROCESS_LIB_NAME}"
)

ExternalProject_Add_Step(cppast
    install_runtime
    DEPENDEES after_install
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CIDER_LIBRARY_OUTPUT_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy "${CppAst_LIBRARIES}" "${CIDER_LIBRARY_OUTPUT_DIRECTORY}"
)

add_dependencies(${CIDER_THIRDPARTY_TARGET_NAME} cppast)
