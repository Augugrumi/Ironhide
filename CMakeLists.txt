cmake_minimum_required(VERSION 3.5.1)
project(ironhide)

INCLUDE(CheckLibraryExists)
INCLUDE(FindPkgConfig)

set(CMAKE_CXX_STANDARD 11)
set(VERSION "0.1")

if ((CMAKE_BUILD_TYPE MATCHES Debug) OR (CMAKE_BUILD_TYPE MATCHES DEBUG))
    set(DEBUG_BUILD 1)
    message(STATUS "Note: building in debug mode. Log will be more verbose")
else()
    set(DEBUG_BUILD 0)
    message(STATUS "Note: building in release mode. Build in debug mode for a verbose log")
endif()

add_executable(ironhide
        main.cpp
        src/proxy/bridge.cpp)

# Set library version
set_target_properties(
        ${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_NAME}_VERSION_STRING
)

# In debug mode add more compilation flags
if ((CMAKE_BUILD_TYPE MATCHES Debug)  OR (CMAKE_BUILD_TYPE MATCHES DEBUG))
    target_compile_options(${PROJECT_NAME} PRIVATE -pedantic -ansi -Wall -Werror)
endif()


# Search for basic Thread support
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE "-pthread")

# Boost system is needed
find_package(Boost 1.58 COMPONENTS system REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

# Boost thread is needed
find_package(Boost 1.58 COMPONENTS thread REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})