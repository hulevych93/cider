
set(TESTED_FILES ${EXPERIMENT_HJSON_BUILD_DIR}/src/hjson_value.cpp
    ${EXPERIMENT_HJSON_BUILD_DIR}/src/hjson_encode.cpp
    ${EXPERIMENT_HJSON_BUILD_DIR}/src/hjson_decode.cpp
    ${EXPERIMENT_HJSON_BUILD_DIR}/src/hjson_parsenumber.cpp)

set_source_files_properties(${TESTED_FILES}
                            PROPERTIES GENERATED TRUE)

add_subdirectory(recorder)
add_subdirectory(player)
