cmake_minimum_required(VERSION 3.22)

project (socket)

# general flags (consider -Wextra)
SET(CMAKE_CXX_FLAGS "-Wall -fpermissive")

# c/c++ standard if not using the default
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 12.0)
   # for intel systems add -fsanitize=address. Note this is not supported
   # on apple's aarch64 under gcc. Maybe with clang/xcode. Also, consider 
   # -fsanitize-address-use-after-scope
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 16.0)
   # matches clang and apple/clang
   
   # Four options exist - (un)comment as desired. Only one at a time
   # 1. compile a binary
   # 2. run clang's static analyser
   # 3. run sanitize (ASAN) looking for address errors
   # 4. run clang's linter, clang-tidy

   # Opt 1: build binaries
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

   # Opt 2: static analyzer
   #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g --analyze -Xanalyzer -analyzer-output=html -o report")

   # Opt 3: to run sanitize on your code
   #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fsanitize-address-use-after-scope")

   # Opt 4: to use clang-tidy w/o database
   set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
   set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=* --")

else ()
   # if using any other compiler/version, fail
   message(FATAL_ERROR "Using the wrong compiler, clang 16+, or gcc 12+")
endif()

find_package(Boost 1.55 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)

# if the project has specialized package configurations
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules)

# hold over from another build - shows how to check for libraries
#find_package(Rabbitmqc REQUIRED)
#include_directories(SYSTEM ${Rabbitmqc_INCLUDE_DIRS})
#find_package(SimpleAmqpClient REQUIRED)
#include_directories(SYSTEM ${SimpleAmqpClient_INCLUDE_DIRS})


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("*** build output dir: ${CMAKE_BINARY_DIR} ***")

include_directories(BEFORE src ${CMAKE_CURRENT_BINARY_DIR})

include(CheckIncludeFile)

add_subdirectory(src)
add_subdirectory(test)
