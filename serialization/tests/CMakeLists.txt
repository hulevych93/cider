cider_project(serialization_test)

cider_collect_src(LIST_OUT)

add_executable(serialization_test ${LIST_OUT})
add_test(NAME "serialization_test" COMMAND serialization_test)

add_dependencies(serialization_test ${CIDER_THIRDPARTY_TARGET_NAME})

target_include_directories(serialization_test PRIVATE ${GTest_INCLUDE_DIRS})

target_link_libraries(serialization_test
    serialization
    ${GTest_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)
