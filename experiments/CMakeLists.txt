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
      ${PLOT_LIBS}
    )
endfunction()

add_subdirectory(hjson)
add_subdirectory(pugixml)
add_subdirectory(BitmapPlusPlus)

if(STAGING)
  add_subdirectory(Argh)
endif()
