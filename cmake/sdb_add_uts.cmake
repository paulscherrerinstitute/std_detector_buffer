cmake_minimum_required(VERSION 3.17)

function (sbd_add_uts TARGET)
    if (BUILD_TESTING)
        add_test(
            NAME ${TARGET}_uts
            COMMAND
                $<TARGET_FILE:${TARGET}>
                $<$<BOOL:$ENV{CI}>:--gtest_output=xml:${CMAKE_BINARY_DIR}/${TARGET}.xml>
        )
    endif()
endfunction()
