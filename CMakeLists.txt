cmake_minimum_required(VERSION 3.20)

project(bstr LANGUAGES C VERSION 0.3.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_OPTIMIZE_DEPENDENCIES ON)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/W4)
    option(ENABLE_MSVC_ANALYZE "Enable MSVC static code analysis (/analyze)" OFF)
    if(ENABLE_MSVC_ANALYZE)
        add_compile_options(/analyze)
    endif()
else()
    add_compile_options(-Wall -Wextra)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
    endif()

    option(UNIT_TEST "Unit Test Build" OFF)
    option(LEAK_CHECK "Memory Leak Detection" OFF)
endif()

set(BSTR_SANITIZERS "" CACHE STRING "Comma-separated sanitizers to enable (for example: address,undefined or thread)")
set_property(CACHE BSTR_SANITIZERS PROPERTY STRINGS "" "address" "address,undefined" "thread")

if(BSTR_SANITIZERS)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        add_compile_options(-fsanitize=${BSTR_SANITIZERS} -fno-omit-frame-pointer)
        add_link_options(-fsanitize=${BSTR_SANITIZERS})
    else()
        message(FATAL_ERROR "BSTR_SANITIZERS is only supported with GCC or Clang")
    endif()
endif()

if (UNIT_TEST)
    message(STATUS "UNIT_TEST=${UNIT_TEST} (BSTR)")
endif()

if (LEAK_CHECK)
    message(STATUS "LEAK_CHECK=${LEAK_CHECK} (BSTR)")
endif()

### Library bstr
set (BSTR_HEADER_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bstr.h
)

set (BSTR_SOURCE_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bstr.c
)

add_library(bstr ${BSTR_HEADER_LIST} ${BSTR_SOURCE_LIST})

if (LEAK_CHECK)
    target_compile_definitions(bstr PRIVATE MEM_LEAK_CHECK)
    target_link_libraries(bstr PRIVATE cutil)
endif()
target_link_libraries(bstr PUBLIC adt)
target_include_directories(bstr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
###

### Executable bstr_unit
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT TARGET adt)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../adt/CMakeLists.txt")
            add_subdirectory(../adt ${CMAKE_CURRENT_BINARY_DIR}/adt)
        else()
            message(FATAL_ERROR "adt repository not found at ../adt")
        endif()
    endif()

    if(NOT TARGET cutil)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../cutil/CMakeLists.txt")
            add_subdirectory(../cutil ${CMAKE_CURRENT_BINARY_DIR}/cutil)
        else()
            message(FATAL_ERROR "cutil repository not found at ../cutil")
        endif()
    endif()

    if(UNIT_TEST)
        set (BSTR_TEST_SUITE_LIST test/testsuite_bstr.c)
        add_executable(bstr_unit test/test_main.c ${BSTR_TEST_SUITE_LIST})
        target_link_libraries(bstr_unit PRIVATE adt bstr cutest)
        target_include_directories(bstr_unit PRIVATE
                                "${PROJECT_BINARY_DIR}"
                                "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                )
        target_compile_definitions(bstr_unit PRIVATE UNIT_TEST)

        if (LEAK_CHECK)
            target_compile_definitions(bstr_unit PRIVATE MEM_LEAK_CHECK)
            target_link_libraries(bstr_unit PRIVATE cutil)
        endif()

        enable_testing()
        add_test(NAME bstr_unit COMMAND bstr_unit)
        set_tests_properties(bstr_unit PROPERTIES
            PASS_REGULAR_EXPRESSION "OK \\([0-9]+ tests\\)"
            LABELS "unit"
        )
    endif()
endif()
###
