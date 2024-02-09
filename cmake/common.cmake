# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

# The `cider_export_var` declares a variable in the project scope
# and also in the parent project scope if one exists.
macro(cider_export_var VARIABLE_NAME)
    set(${VARIABLE_NAME} ${ARGN})
    if(NOT PROJECT_IS_TOP_LEVEL)
        set(${VARIABLE_NAME} ${${VARIABLE_NAME}} PARENT_SCOPE)
    endif()
endmacro()

# Causes our CMake scripts to depend on the passed files - when one of them is changed, CMake is re-run.
function(cider_add_cmake_deps)
    foreach(FILE ${ARGN})
        # Note: the destination file is not important, the whole purpose of using configure_file is its side
        # effect that causes cmake to be re-run when the source file is changed.
        get_filename_component(FILE_NAME ${FILE} NAME)
        get_filename_component(DIR_NAME ${FILE} DIRECTORY)
        string(MD5 DIR_HASH ${DIR_NAME})

        configure_file(${FILE} ${CIDER_TMP_DIR}/${FILE_NAME}_${DIR_HASH} @ONLY)
    endforeach()
endfunction()

# Map list of string to list of the same length modified according to regex.
function(cider_map_list cider_map_list_LIST cider_map_list_MATCH_RE cider_map_list_REPLACE_EXPR cider_map_list_LIST_OUT_VAR)
    set(cider_map_list_NEW_LIST)
    foreach(cider_map_list_ITEM IN LISTS "${cider_map_list_LIST}")
        string(REGEX REPLACE "${cider_map_list_MATCH_RE}" "${cider_map_list_REPLACE_EXPR}" cider_map_list_NEW_ITEM "${cider_map_list_ITEM}")
        list(APPEND cider_map_list_NEW_LIST "${cider_map_list_NEW_ITEM}")
    endforeach()

    set("${cider_map_list_LIST_OUT_VAR}" "${cider_map_list_NEW_LIST}" PARENT_SCOPE)
endfunction()

# Transform a list into other list with respect to
# custom seperator value.
function(cider_join_list LIST SEPARATOR OUTPUT)
    set(join_list_RESULT)
    set(IS_FIRST TRUE)
    foreach(VALUE ${${LIST}})
        if(IS_FIRST)
            set(IS_FIRST FALSE)
            set(join_list_RESULT "${join_list_RESULT}${VALUE}")
        else()
            set(join_list_RESULT "${join_list_RESULT}${SEPARATOR}${VALUE}")
        endif()
    endforeach()

    set("${OUTPUT}" "${join_list_RESULT}" PARENT_SCOPE)
endfunction()

macro(cider_list_replace LIST OLD_VALUE NEW_VALUE)
    list(FIND ${LIST} ${OLD_VALUE} OLD_VALUE_INDEX)
    if(OLD_VALUE_INDEX GREATER_EQUAL 0)
        list(REMOVE_AT ${LIST} ${OLD_VALUE_INDEX})
        list(INSERT ${LIST} ${OLD_VALUE_INDEX} ${NEW_VALUE})
    endif()
endmacro()

macro(cider_project NAME)
    project(NAME)
    message(STATUS "Processing ${NAME}")
endmacro()
