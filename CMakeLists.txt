cmake_minimum_required(VERSION 3.16)
project(networking-cpp)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(ENABLE_SANITIZERS "Build with AddressSanitizer and UBSan" OFF)

add_library(project_options INTERFACE)

target_compile_options(project_options INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Wshadow>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(project_options INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(project_options INTERFACE
            -fsanitize=address,undefined)
endif()

# Boost.Asio
find_package(Threads REQUIRED)
find_package(Boost 1.74 QUIET)

if(Boost_FOUND)
    message(STATUS "Boost ${Boost_VERSION} found: Asio experiments enabled")

    add_library(asio_deps INTERFACE)
    target_link_libraries(asio_deps INTERFACE Boost::headers Threads::Threads)
    target_compile_definitions(asio_deps INTERFACE BOOST_ASIO_NO_DEPRECATED)

    if(WIN32)
        target_link_libraries(asio_deps INTERFACE ws2_32 mswsock)
        target_compile_definitions(asio_deps INTERFACE _WIN32_WINNT=0x0A00)
    endif()
else()
    message(WARNING "Boost not found: Asio experiments will be skipped "
            "(install libboost-dev or equivalent)")
endif()

# functions for the experiments
function(add_experiment name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE project_options )
endfunction()

function(add_asio_experiment name)
    if(NOT Boost_FOUND)
        message(STATUS "Skipping ${name}: Boost not found")
        return()
    endif()
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE project_options asio_deps)
endfunction()

# auto discover the experiments
set(EXPERIMENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB children RELATIVE ${EXPERIMENTS_DIR} CONFIGURE_DEPENDS
        ${EXPERIMENTS_DIR}/*)
list(SORT children)

foreach(child ${children})
    if(IS_DIRECTORY ${EXPERIMENTS_DIR}/${child}
            AND EXISTS ${EXPERIMENTS_DIR}/${child}/CMakeLists.txt)
        add_subdirectory(src/${child})
    endif()
endforeach()