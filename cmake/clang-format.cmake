# clang-format.cmake

if(NOT CLANGFORMAT_EXECUTABLE)
    find_program(CLANGFORMAT_EXECUTABLE "clang-format")
endif()

function(cider_run_clang_format)
    if(CLANGFORMAT_EXECUTABLE)
        cider_collect_src(ALL_SRC)

        foreach(SFILE ${ALL_SRC})
            execute_process(COMMAND ${CLANGFORMAT_EXECUTABLE} -style=Chromium -i ${SFILE})
        endforeach()
    endif()
endfunction()

# run clang-formatting
if(APPLE)
cider_run_clang_format()
endif()
