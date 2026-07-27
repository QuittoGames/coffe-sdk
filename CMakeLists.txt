cmake_minimum_required(VERSION 3.20)
project(coffee-sdk
    VERSION 0.1.0
    DESCRIPTION "Coffee SDK — Personal software ecosystem platform"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-c++98-compat")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

option(COFFEE_BUILD_TESTS "Build tests" OFF)

# Helper: only creates a library if sources exist, otherwise INTERFACE
function(add_coffee_library name)
    set(options SHARED)
    set(oneValueArgs)
    set(multiValueArgs SOURCES LINK)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARGS_SOURCES)
        if(ARGS_SHARED)
            add_library(${name} SHARED ${ARGS_SOURCES})
        else()
            add_library(${name} ${ARGS_SOURCES})
        endif()
        target_include_directories(${name} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
        if(ARGS_LINK)
            target_link_libraries(${name} PUBLIC ${ARGS_LINK})
        endif()
    else()
        add_library(${name} INTERFACE)
        target_include_directories(${name} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
        if(ARGS_LINK)
            target_link_libraries(${name} INTERFACE ${ARGS_LINK})
        endif()
    endif()
endfunction()

# ============================================================
# Core — interfaces e contratos (header-only)
# ============================================================
add_library(coffee_core INTERFACE)
target_include_directories(coffee_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(coffee_core INTERFACE cxx_std_20)

# ============================================================
# Runtime, Services, OS, Adapter — criados só se houver .cpp
# ============================================================
file(GLOB_RECURSE _runtime_src src/runtime/*.cpp)
file(GLOB_RECURSE _services_src src/services/*.cpp)
file(GLOB_RECURSE _os_src src/core/modules/os/*.cpp)
file(GLOB_RECURSE _c_adapter_src src/adapter/c/*.cpp)

add_coffee_library(coffee_runtime SOURCES ${_runtime_src} LINK coffee_core)
add_coffee_library(coffee_services SOURCES ${_services_src} LINK coffee_runtime)
add_coffee_library(coffee_os SOURCES ${_os_src} LINK coffee_core)
add_coffee_library(coffee_adapter_c SHARED SOURCES ${_c_adapter_src} LINK coffee_core)

if(_c_adapter_src)
    set_target_properties(coffee_adapter_c PROPERTIES
        EXPORT_NAME coffee_adapter_c
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

# ============================================================
# Umbrella target
# ============================================================
add_library(coffee INTERFACE)
target_link_libraries(coffee INTERFACE
    coffee_core
    coffee_runtime
    coffee_services
    coffee_os
)

# ============================================================
# Tests
# ============================================================
if(COFFEE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================
# Install (apenas headers por enquanto)
# ============================================================
install(DIRECTORY include/ DESTINATION include)
