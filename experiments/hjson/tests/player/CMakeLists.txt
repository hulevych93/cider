cider_project(hjson_player)

set(GEN_DIR ${CIDER_GEN_DIR}/hjson)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${GTest_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_executable(hjson_player hjson_play.cpp ${TESTED_FILES})
target_link_libraries(hjson_player PUBLIC
  hjson_wrap
  hjson-models
  scripting
  recorder
  ${Lua_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

if(HJSON_ENABLE_COVERAGE)
  setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE hjson_player
        EXECUTABLE_ARGS "${CMAKE_BINARY_DIR}/experiments/hjson/tests/script.lua"
        BASE_DIRECTORY "${EXPERIMENT_HJSON_BUILD_DIR}/src"
        EXCLUDE "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*")
endif()
