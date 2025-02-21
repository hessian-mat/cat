cmake_minimum_required(VERSION 3.10)
project(cat LANGUAGES C)
enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(
        -Wall
        -Wextra
    )
endif()

if(WIN32)
    file(TO_CMAKE_PATH "$ENV{USERPROFILE}/cat" INSTALL_PATH)
    set(CMAKE_INSTALL_PREFIX ${INSTALL_PATH})
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

file(GLOB LIB_SOURCES "src/*.c")
include_directories("src/include")

add_library(cat STATIC ${LIB_SOURCES})
add_library(cat_shared SHARED ${LIB_SOURCES})
if(WIN32)
    set_target_properties(cat PROPERTIES OUTPUT_NAME "cat_static")
    set_target_properties(cat_shared PROPERTIES OUTPUT_NAME "cat")
else()
    set_target_properties(cat_shared PROPERTIES OUTPUT_NAME "cat")
endif()

file(GLOB TEST_SOURCES "test/test_*.c")
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src} test/unity.c)
    target_link_libraries(${test_name} cat)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

install(TARGETS cat cat_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY src/include/
    DESTINATION include
)
