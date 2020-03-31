cmake_minimum_required(VERSION 3.14)

include(cmake/BuildTypes.cmake)
include(cmake/LeakCheck.cmake)

if (LEAK_CHECK)
    message(STATUS "LEAK_CHECK=${LEAK_CHECK} (BSTR)")
endif()

### Library bstr
set (BSTR_HEADER_LIST
    inc/bstr.h
)

set (BSTR_SOURCE_LIST
    src/bstr.c
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
project(bstr_unit LANGUAGES C VERSION 1.0)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(../adt ${CMAKE_CURRENT_BINARY_DIR}/adt)
    add_subdirectory(../cutil ${CMAKE_CURRENT_BINARY_DIR}/cutil)

    set (BSTR_TEST_SUITE_LIST
        test/testsuite_bstr.c
    )

    add_executable(bstr_unit test/test_main.c ${BSTR_TEST_SUITE_LIST})
    target_link_libraries(bstr_unit PRIVATE adt bstr cutest)
    target_include_directories(bstr_unit PRIVATE
                              "${PROJECT_BINARY_DIR}"
                              "${CMAKE_CURRENT_SOURCE_DIR}/inc"                              
                              )
    target_compile_definitions(bstr_unit PRIVATE
        UNIT_TEST
    )

    if (LEAK_CHECK)
        target_compile_definitions(bstr_unit PRIVATE MEM_LEAK_CHECK)
        target_link_libraries(bstr_unit PRIVATE cutil)
    endif()
endif()
###
