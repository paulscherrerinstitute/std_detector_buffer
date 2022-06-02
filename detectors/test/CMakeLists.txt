add_executable(detectors-tests main.cpp)
set_target_properties(detectors-tests PROPERTIES OUTPUT_NAME detectors_tests)
target_link_libraries(detectors-tests
        detectors
        gtest
        )
