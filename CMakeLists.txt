cmake_minimum_required(VERSION 3.20)
project(clua_std VERSION 0.3.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Lua 5.3 REQUIRED)

# A static Lua archive must have been built with position-independent code to
# be linkable into this shared module. Keep configuration usable with either
# shared or PIC static Lua, but make the common failure mode visible early.
foreach(lua_library IN LISTS LUA_LIBRARIES)
    if(lua_library MATCHES "\\.a$")
        message(STATUS "Lua static archive detected: ${lua_library}; it must be PIC for module linking")
    endif()
endforeach()

add_library(cl MODULE
    csrc/cl.c
    csrc/core.c
)

target_include_directories(cl PRIVATE
    ${LUA_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/csrc
)

target_link_libraries(cl PRIVATE ${LUA_LIBRARIES})

if(MSVC)
    target_compile_options(cl PRIVATE /W4 /WX)
else()
    target_compile_options(cl PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Lua's require("cl") looks for cl.so (or the platform equivalent).
set_target_properties(cl PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lua"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lua"
)

add_executable(core_tests
    tests/test_core.c
    csrc/core.c
)
target_include_directories(core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/csrc)
if(MSVC)
    target_compile_options(core_tests PRIVATE /W4 /WX)
else()
    target_compile_options(core_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_test(NAME cl_core_tests COMMAND core_tests)
add_test(
    NAME cl_lua_tests
    COMMAND ${LUA_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cl.lua
)
set_tests_properties(cl_lua_tests PROPERTIES
    ENVIRONMENT "LUA_CPATH=$<TARGET_FILE_DIR:cl>/?$<TARGET_FILE_SUFFIX:cl>;;"
)
