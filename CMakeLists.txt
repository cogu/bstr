cmake_minimum_required(VERSION 3.14)

project(bstr LANGUAGES C VERSION 0.1.1)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(../adt/cmake/BuildTypes.cmake)
    include(../adt/cmake/LeakCheck.cmake)
    include(../adt/cmake/UnitTest.cmake)
endif()

if (LEAK_CHECK)
    message(STATUS "LEAK_CHECK=${LEAK_CHECK} (BSTR)")
endif()

### Library bstr
set (BSTR_HEADER_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/bstr.h
)

set (BSTR_SOURCE_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bstr.c
)

add_library(bstr ${BSTR_HEADER_LIST} ${BSTR_SOURCE_LIST})

if (LEAK_CHECK)
    target_compile_definitions(bstr PRIVATE MEM_LEAK_CHECK)
    target_link_libraries(bstr PRIVATE cutil)
endif()
target_link_libraries(bstr PRIVATE adt)
target_include_directories(bstr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
###

### Executable bstr_unit
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(../adt ${CMAKE_CURRENT_BINARY_DIR}/adt)
    add_subdirectory(../cutil ${CMAKE_CURRENT_BINARY_DIR}/cutil)

    if(UNIT_TEST)

        set (BSTR_TEST_SUITE_LIST test/testsuite_bstr.c)

        add_executable(bstr_unit test/test_main.c ${BSTR_TEST_SUITE_LIST})
        target_link_libraries(bstr_unit PRIVATE adt bstr cutest)
        target_include_directories(bstr_unit PRIVATE
                                "${PROJECT_BINARY_DIR}"
                                "${CMAKE_CURRENT_SOURCE_DIR}/inc"
                                )
        target_compile_definitions(bstr_unit PRIVATE UNIT_TEST)

        if (LEAK_CHECK)
            target_compile_definitions(bstr_unit PRIVATE MEM_LEAK_CHECK)
            target_link_libraries(bstr_unit PRIVATE cutil)
        endif()

        enable_testing()
        add_test(bstr_test ${CMAKE_CURRENT_BINARY_DIR}/bstr_unit)
        set_tests_properties(bstr_test PROPERTIES PASS_REGULAR_EXPRESSION "OK \\([0-9]+ tests\\)")
    endif()
endif()
###
