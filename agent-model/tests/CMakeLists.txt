cider_project(agent-model-test)

cider_collect_src(LIST_OUT)

add_executable(agent-model-test ${LIST_OUT})
add_test(NAME "agent-model-test" COMMAND agent-model-test)

add_dependencies(agent-model-test ${CIDER_THIRDPARTY_TARGET_NAME})

target_include_directories(agent-model-test PRIVATE ${GTest_INCLUDE_DIRS})

target_link_libraries(agent-model-test
    agent-model
    recorder
    serialization
    ${GTest_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)
