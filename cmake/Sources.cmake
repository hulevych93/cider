# sources.cmake

# Excludes are passed as ARGN
function(prv_collect_sources SOURCE_FOLDER SRC_LIST_OUT)
    set(SRC_LIST)

    # Note: header files must also be included here for Qt moc compiler to handle them.
    file(GLOB_RECURSE SRC_LIST
            "${SOURCE_FOLDER}/*.cpp"
            "${SOURCE_FOLDER}/*.h"
            "${SOURCE_FOLDER}/*.rdl"
            "${SOURCE_FOLDER}/*.json")

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

# Excludes are passed as ARGN
function(gunit_collect_src LIST_OUT)
    set(IGNORED_LIST)
    list(APPEND IGNORED_LIST ${ARGN})

    if(UNIX)
        list(APPEND IGNORED_LIST "Windows/")

        if(APPLE)
            list(APPEND IGNORED_LIST "Linux/")
        else()
            list(APPEND IGNORED_LIST "MacOS/")
        endif()
    endif()

    if(WIN32)
        list(APPEND IGNORED_LIST "Linux/")
        list(APPEND IGNORED_LIST "MacOS/")
        list(APPEND IGNORED_LIST "Unix/")
    endif()

    prv_collect_sources(${PROJECT_SOURCE_DIR} TMP_SRC_LIST ${IGNORED_LIST})
    set(${LIST_OUT} ${TMP_SRC_LIST} PARENT_SCOPE)
endfunction()

function(gunit_collect_only_cpp LIST_OUT)
    gunit_collect_src(TMP_SRC_LIST ".json" ".rdl")
    set(${LIST_OUT} ${TMP_SRC_LIST} PARENT_SCOPE)
endfunction()

function(gunit_collect_only_platform_cpp LIST_OUT)
    set(SRC_LIST)

    file(GLOB_RECURSE SRC_LIST
            "${SOURCE_FOLDER}/*.cpp"
            "${SOURCE_FOLDER}/*.h")

    set(APPEND_LIST)

    if(UNIX AND NOT APPLE)
        list(APPEND APPEND_LIST "Linux/")
    endif()

    if(WIN32)
        list(APPEND APPEND_LIST "Windows/")
    endif()

    set(LIST_FILTERED)
    foreach(SOURCE ${SRC_LIST})
      set(FOUND "-1")
      foreach(INCLUDE ${APPEND_LIST})
        string(FIND ${SOURCE} ${INCLUDE} FOUND)
        if(NOT("${FOUND}" STREQUAL "-1"))
            list(APPEND LIST_FILTERED ${SOURCE})
            break()
        endif()
      endforeach()
    endforeach()

    # Note: at least in the unity build mode the order in which source files are added to targets is important.
    # So, we sort them to make sure that the order stays the same on different runs of CMake.
    list(SORT LIST_FILTERED)

    set(${SRC_LIST_OUT} ${LIST_FILTERED} PARENT_SCOPE)
endfunction()

function(gunit_collect_rdls LIST_OUT)
    prv_collect_sources(${GUNIT_RDLS_DIR} TMP_SRC_LIST)
    set(${LIST_OUT} ${TMP_SRC_LIST} PARENT_SCOPE)
endfunction()

function(gunit_collect_test_src SRC_LIST_OUT)
    prv_collect_sources(${PROJECT_SOURCE_DIR}/tests SRC_LIST ${ARGN})
    set(${SRC_LIST_OUT} ${SRC_LIST} PARENT_SCOPE)
endfunction()
