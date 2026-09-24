cmake_minimum_required(VERSION 3.20)
project(clua_std VERSION 0.5.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Lua 5.3 REQUIRED)
find_program(LUA_EXECUTABLE NAMES lua lua5.4 lua5.3 lua54 lua53 REQUIRED)

include(GNUInstallDirs)

option(CLUA_ENABLE_SANITIZERS "Build with address and undefined behavior sanitizers" OFF)
option(CLUA_ENABLE_COVERAGE "Build with gcov-compatible coverage instrumentation" OFF)

if(CLUA_ENABLE_SANITIZERS AND MSVC)
    message(FATAL_ERROR "CLUA_ENABLE_SANITIZERS is not supported with MSVC")
endif()

if(CLUA_ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

if(CLUA_ENABLE_COVERAGE AND NOT MSVC)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

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
add_library(clua_std::cl ALIAS cl)

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

file(GLOB_RECURSE CLUA_EXAMPLE_MAIN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/*/main.lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/*/*/main.lua"
)
foreach(example_file IN LISTS CLUA_EXAMPLE_MAIN_FILES)
    get_filename_component(example_dir "${example_file}" DIRECTORY)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" example_test_name "${example_file}")
    add_test(
        NAME cl_example_${example_test_name}
        COMMAND ${LUA_EXECUTABLE} ${example_file}
        WORKING_DIRECTORY ${example_dir}
    )
    set_tests_properties(cl_example_${example_test_name} PROPERTIES
        ENVIRONMENT "LUA_CPATH=$<TARGET_FILE_DIR:cl>/?$<TARGET_FILE_SUFFIX:cl>;;"
        LABELS "example"
    )
endforeach()

add_test(
    NAME cl_example_main
    COMMAND ${LUA_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/lua/main.lua 1 2 3
)
set_tests_properties(cl_example_main PROPERTIES
    ENVIRONMENT "LUA_CPATH=$<TARGET_FILE_DIR:cl>/?$<TARGET_FILE_SUFFIX:cl>;;"
    LABELS "example"
)

install(TARGETS cl
    EXPORT clua_stdTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES README.md docs/api.md docs/architecture.md docs/build-and-portability.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
install(EXPORT clua_stdTargets
    FILE clua_stdTargets.cmake
    NAMESPACE clua_std::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clua_std
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/clua_stdConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/clua_stdConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clua_std
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/clua_stdConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/clua_stdConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/clua_stdConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clua_std
)
