# clang-format.cmake

if(NOT CLANGFORMAT_EXECUTABLE)
    find_program(CLANGFORMAT_EXECUTABLE "clang-format")
endif()

function(gunit_run_clang_format)
    if(CLANGFORMAT_EXECUTABLE)
        gunit_collect_src(ALL_SRC)

        foreach(SFILE ${ALL_SRC})
            execute_process(COMMAND ${CLANGFORMAT_EXECUTABLE} -style=Chromium -i ${SFILE})
        endforeach()
    endif()
endfunction()

# run clang-formatting
gunit_run_clang_format()
