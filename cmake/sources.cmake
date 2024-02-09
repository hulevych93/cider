# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

# Excludes are passed as ARGN
function(prv_collect_src SOURCE_FOLDER SRC_LIST_OUT)
    set(SRC_LIST)

    file(GLOB_RECURSE SRC_LIST
            "${SOURCE_FOLDER}/*.cpp"
            "${SOURCE_FOLDER}/*.h"
            "${SOURCE_FOLDER}/*.hpp")

    set(LIST_FILTERED)
    foreach(SOURCE ${SRC_LIST})
      set(FOUND "-1")
      foreach(EXCLUDE ${ARGN})
        string(FIND ${SOURCE} ${EXCLUDE} FOUND)
        if(NOT("${FOUND}" STREQUAL "-1"))
            break()
        endif()
      endforeach()
      if("${FOUND}" STREQUAL "-1")
          list(APPEND LIST_FILTERED ${SOURCE})
      endif()
    endforeach()

    # Note: at least in the unity build mode the order in which source files are added to targets is important.
    # So, we sort them to make sure that the order stays the same on different runs of CMake.
    list(SORT LIST_FILTERED)

    set(${SRC_LIST_OUT} ${LIST_FILTERED} PARENT_SCOPE)
endfunction()

function(cider_collect_src LIST_OUT)
    prv_collect_src(${PROJECT_SOURCE_DIR} TMP_SRC_LIST ${ARGN})
    set(${LIST_OUT} ${TMP_SRC_LIST} PARENT_SCOPE)
endfunction()

function(cider_collect_test_src SRC_LIST_OUT)
    prv_collect_src(${PROJECT_SOURCE_DIR}/tests SRC_LIST ${ARGN})
    set(${SRC_LIST_OUT} ${SRC_LIST} PARENT_SCOPE)
endfunction()
