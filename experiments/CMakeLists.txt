# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

function(add_experiment_driver target_name)
    add_executable(${target_name}_driver coverage_driver.cpp)

    target_include_directories(${target_name}_driver PRIVATE ${Python3_INCLUDE_DIRS} ${PyBind_INCLUDE_DIRS})

    target_link_libraries(${target_name}_driver PUBLIC
      coverage
      scripting
      ${target_name}_tests
      metaheuristics
      pipelines
      agent-model
      agent-q-learning
      agent-sarsa-learning
      synthesis
      monte-carlo
      greedy-r
      dynamic-slicer
      mathplot-log
      math-stat
      ${Python3_LIBRARIES}
    )
endfunction()

function(add_experiment_target lib_name target_name mode cmd_list_var)
    set(options)
    set(oneValueArgs GROUP)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "" ${ARGN})

    set(cmd_list "${${cmd_list_var}}")

    set(cmd
        ${lib_name}_driver
        "${mode}"
        ${cmd_list}
    )

    if (ARG_GROUP)
        list(APPEND cmd "${ARG_GROUP}")
    endif()

    add_custom_target(${lib_name}_${target_name}
        COMMAND ${cmd}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/${lib_name}/tests/${lib_name}_tests"
        DEPENDS ${lib_name}_player
        VERBATIM
    )
endfunction()

# Helper to add a target definition safely
function(add_target_def name mode)
    set(options)
    set(oneValueArgs GROUP)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "" ${ARGN})

    if (ARG_GROUP)
        list(APPEND TARGET_DEFS "${name}\;${mode}\;GROUP\;${ARG_GROUP}")
    else()
        list(APPEND TARGET_DEFS "${name}\;${mode}")
    endif()

    set(TARGET_DEFS "${TARGET_DEFS}" PARENT_SCOPE)
endfunction()

set(GROUP_QLEG        1)
set(GROUP_QLB         2)
set(GROUP_SLEG        3)
set(GROUP_SLB         4)
set(GROUP_MCTS        5)
set(GROUP_QLEG_VS_QLB 6)
set(GROUP_RAND        7)
set(GROUP_GREEDY_R    8)
set(GROUP_DSL         9)
set(GROUP_SELECTED    10)
set(GROUP_TARGET      11)
set(GROUP_ALL         12)

# Reset the list
set(TARGET_DEFS)

# === HS Modes ===
add_target_def(HS 0)
add_target_def(HS1 1)
add_target_def(HS2 2)
add_target_def(HS3 3)

# === CS / RAND ===
add_target_def(CS 10)
add_target_def(RAND 20)

# === MCTS Modes ===
add_target_def(MCTS1 30 GROUP ${GROUP_MCTS})
add_target_def(MCTS2 31 GROUP ${GROUP_MCTS})
add_target_def(MCTS3 32 GROUP ${GROUP_MCTS})

# === GRR (Greedy Random) ===
add_target_def(GR 39 GROUP ${GROUP_GREEDY_R})
add_target_def(GRR1 40 GROUP ${GROUP_GREEDY_R})
add_target_def(GRR2 41 GROUP ${GROUP_GREEDY_R})
add_target_def(GRR3 42 GROUP ${GROUP_GREEDY_R})

# === DSL ===
add_target_def(DSL_FM1 35 GROUP ${GROUP_DSL})
add_target_def(DSL_FM2 36 GROUP ${GROUP_DSL})
add_target_def(DSL_FM3 37 GROUP ${GROUP_DSL})

add_target_def(DSL 45 GROUP ${GROUP_DSL})
add_target_def(DSL_TR 15 GROUP ${GROUP_DSL})

add_target_def(DSL_POST 46 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(DSL_FM1_POST 47 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(DSL_FM2_POST 48 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(DSL_FM3_POST 49 GROUP ${GROUP_QLEG_VS_QLB})

# === QL / SARSA ===
add_target_def(QL 50)
add_target_def(SARSA 51)

# === QLEG ===
add_target_def(QLEG1 60 GROUP ${GROUP_QLEG})
add_target_def(QLEG2 61 GROUP ${GROUP_QLEG})
add_target_def(QLEG3 62 GROUP ${GROUP_QLEG})

# === SLEG ===
add_target_def(SLEG1 65)
add_target_def(SLEG2 66)
add_target_def(SLEG3 67)

# === QLB ===
add_target_def(QLB1 70 GROUP ${GROUP_QLB})
add_target_def(QLB2 71 GROUP ${GROUP_QLB})
add_target_def(QLB3 72 GROUP ${GROUP_QLB})

# === SLB ===
add_target_def(SLB1 75)
add_target_def(SLB2 76)
add_target_def(SLB3 77)

add_target_def(QL_STAT 80)

# === CBOX Grouped ===
add_target_def(CBOX_RAND     90 GROUP ${GROUP_RAND})
add_target_def(CBOX_QLEG     90 GROUP ${GROUP_QLEG})
add_target_def(CBOX_QLB      90 GROUP ${GROUP_QLB})
add_target_def(CBOX_SLEG     90 GROUP ${GROUP_SLEG})
add_target_def(CBOX_SLB      90 GROUP ${GROUP_SLB})
add_target_def(CBOX_MCTS     90 GROUP ${GROUP_MCTS})
add_target_def(CBOX_GRR      90 GROUP ${GROUP_GREEDY_R})
add_target_def(CBOX_DSL      90 GROUP ${GROUP_DSL})
add_target_def(CBOX_SELECT   90 GROUP ${GROUP_SELECTED})
add_target_def(CBOX_ALL      90 GROUP ${GROUP_ALL})

# === CSTEP Grouped ===
add_target_def(CSTEP_QLEG       100 GROUP ${GROUP_QLEG})
add_target_def(CSTEP_QLB        100 GROUP ${GROUP_QLB})
add_target_def(CSTEP_SLEG       100 GROUP ${GROUP_SLEG})
add_target_def(CSTEP_SLB        100 GROUP ${GROUP_SLB})
add_target_def(CSTEP_MCTS       100 GROUP ${GROUP_MCTS})
add_target_def(CSTEP_QLEG_VS_QLB 100 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(CSTEP_SELECT 100 GROUP ${GROUP_SELECTED})

# === LBAR Grouped ===
add_target_def(LBAR_QLEG       110 GROUP ${GROUP_QLEG})
add_target_def(LBAR_QLB        110 GROUP ${GROUP_QLB})
add_target_def(LBAR_SLEG       110 GROUP ${GROUP_SLEG})
add_target_def(LBAR_SLB        110 GROUP ${GROUP_SLB})
add_target_def(LBAR_MCTS       110 GROUP ${GROUP_MCTS})
add_target_def(LBAR_QLEG_VS_QLB 110 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(LBAR_SELECT      110 GROUP ${GROUP_SELECTED})

# === TIMES BAR Grouped ===
add_target_def(TIMES_QLEG       135 GROUP ${GROUP_QLEG})
add_target_def(TIMES_QLB        135 GROUP ${GROUP_QLB})
add_target_def(TIMES_SLEG       135 GROUP ${GROUP_SLEG})
add_target_def(TIMES_SLB        135 GROUP ${GROUP_SLB})
add_target_def(TIMES_MCTS       135 GROUP ${GROUP_MCTS})
add_target_def(TIMES_QLEG_VS_QLB 135 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(TIMES_SELECT      135 GROUP ${GROUP_SELECTED})
add_target_def(TIMES_ALL         135 GROUP ${GROUP_ALL})

# === EXEC_TIMES BAR Grouped ===
add_target_def(EXEC_TIMES_QLEG       137 GROUP ${GROUP_QLEG})
add_target_def(EXEC_TIMES_QLB        137 GROUP ${GROUP_QLB})
add_target_def(EXEC_TIMES_SLEG       137 GROUP ${GROUP_SLEG})
add_target_def(EXEC_TIMES_SLB        137 GROUP ${GROUP_SLB})
add_target_def(EXEC_TIMES_MCTS       137 GROUP ${GROUP_MCTS})
add_target_def(EXEC_TIMES_QLEG_VS_QLB 137 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(EXEC_TIMES_SELECT      137 GROUP ${GROUP_SELECTED})
add_target_def(EXEC_TIMES_ALL         137 GROUP ${GROUP_ALL})

# === EFFICIENCY RADAT Grouped ===
add_target_def(RADAR_SELECT   138 GROUP ${GROUP_SELECTED})

# === COMPRESSION BAR Grouped ===
add_target_def(COMPR_QLEG        136 GROUP ${GROUP_QLEG})
add_target_def(COMPR_QLB         136 GROUP ${GROUP_QLB})
add_target_def(COMPR_DSL         136 GROUP ${GROUP_DSL})
add_target_def(COMPR_SLEG        136 GROUP ${GROUP_SLEG})
add_target_def(COMPR_SLB         136 GROUP ${GROUP_SLB})
add_target_def(COMPR_MCTS        136 GROUP ${GROUP_MCTS})
add_target_def(COMPR_QLEG_VS_QLB 136 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(COMPR_SELECT      136 GROUP ${GROUP_SELECTED})
add_target_def(COMPR_ALL         136 GROUP ${GROUP_ALL})

# === HM Grouped ===
add_target_def(HM_QLEG     130 GROUP ${GROUP_QLEG})
add_target_def(HM_QLB      130 GROUP ${GROUP_QLB})
add_target_def(HM_SLEG     130 GROUP ${GROUP_SLEG})
add_target_def(HM_SLB      130 GROUP ${GROUP_SLB})
add_target_def(HM_MCTS     130 GROUP ${GROUP_MCTS})
add_target_def(HM_GRR      130 GROUP ${GROUP_GREEDY_R})
add_target_def(HM_DSL      130 GROUP ${GROUP_DSL})
add_target_def(HM_SELECT   130 GROUP ${GROUP_SELECTED})
add_target_def(HM_ALL      130 GROUP ${GROUP_ALL})

# === SHOW ===
add_target_def(SHOW_QLEG     170 GROUP ${GROUP_QLEG})
add_target_def(SHOW_QLB      170 GROUP ${GROUP_QLB})
add_target_def(SHOW_SLEG     170 GROUP ${GROUP_SLEG})
add_target_def(SHOW_SLB      170 GROUP ${GROUP_SLB})
add_target_def(SHOW_MCTS     170 GROUP ${GROUP_MCTS})
add_target_def(SHOW_GRR      170 GROUP ${GROUP_GREEDY_R})
add_target_def(SHOW_DSL      170 GROUP ${GROUP_DSL})
add_target_def(SHOW_SELECT   170 GROUP ${GROUP_SELECTED})
add_target_def(SHOW_TARGET   170 GROUP ${GROUP_TARGET})
add_target_def(SHOW_ALL      170)

# === EFT ===
add_target_def(EFT 120)

# === AGG ===
add_target_def(AGG 140 GROUP ${GROUP_SELECTED})

# === VER ===
add_target_def(VER 145 GROUP ${GROUP_QLEG})

# === CALC_COMPR ===
add_target_def(CALC_COMPR_QLEG        146 GROUP ${GROUP_QLEG})
add_target_def(CALC_COMPR_QLB         146 GROUP ${GROUP_QLB})
add_target_def(CALC_COMPR_SLEG        146 GROUP ${GROUP_SLEG})
add_target_def(CALC_COMPR_SLB         146 GROUP ${GROUP_SLB})
add_target_def(CALC_COMPR_MCTS        146 GROUP ${GROUP_MCTS})
add_target_def(CALC_COMPR_QLEG_VS_QLB        146 GROUP ${GROUP_QLEG_VS_QLB})
add_target_def(CALC_COMPR_SELECT      146 GROUP ${GROUP_SELECTED})
add_target_def(CALC_COMPR_ALL         146 GROUP ${GROUP_ALL})

# === REM Grouped ===
add_target_def(RM_GRR  150 GROUP ${GROUP_GREEDY_R})
add_target_def(RM_DSL  150 GROUP ${GROUP_DSL})
add_target_def(RM_RAND  150 GROUP ${GROUP_RAND})

# === CleanUp ===
add_target_def(CLEAN_QLEG     160 GROUP ${GROUP_QLEG})
add_target_def(CLEAN_QLB      160 GROUP ${GROUP_QLB})
add_target_def(CLEAN_SLEG     160 GROUP ${GROUP_SLEG})
add_target_def(CLEAN_SLB      160 GROUP ${GROUP_SLB})
add_target_def(CLEAN_MCTS     160 GROUP ${GROUP_MCTS})
add_target_def(CLEAN_GRR      160 GROUP ${GROUP_GREEDY_R})
add_target_def(CLEAN_DSL      160 GROUP ${GROUP_DSL})
add_target_def(CLEAN_SELECT   160 GROUP ${GROUP_SELECTED})
add_target_def(CLEAN_ALL      160)

add_target_def(MOD      180)

set(LIBS "" CACHE INTERNAL "List of libraries for experiment targets")

add_subdirectory(hjson)
add_subdirectory(pugixml)
add_subdirectory(bitmap++)

if(STAGING)
  add_subdirectory(Argh)
endif()

foreach(lib_def IN LISTS LIBS)
    list(GET lib_def 0 lib_name)
    list(GET lib_def 1 cmd_var)

    foreach(def IN LISTS TARGET_DEFS)
        string(REPLACE ";" ";" parts "${def}")
        list(GET parts 0 tgt)
        list(GET parts 1 mode)
        list(LENGTH parts len)
        if (len GREATER 2)
            list(GET parts 3 group)
            add_experiment_target(${lib_name} ${tgt} ${mode} ${cmd_var} GROUP ${group})
        else()
            add_experiment_target(${lib_name} ${tgt} ${mode} ${cmd_var})
        endif()
    endforeach()
endforeach()

add_custom_target(hjson_RUN)
add_dependencies(hjson_RUN hjson_DSL_FM1 hjson_DSL_FM2 hjson_DSL_FM3)

add_custom_target(bitmap++_RUN)
add_dependencies(bitmap++_RUN bitmap++_DSL_FM1 bitmap++_DSL_FM2 bitmap++_DSL_FM3)
