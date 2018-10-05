add_executable(${PROJECT_NAME}
        "main.cpp"
        "server/server.cpp"
        "server/servertcp.cpp"
        "server/serverudp.cpp"
        "server/utils/threadpool.h"
        "server/utils/asynctaskexecutor.cpp"
        "client/clienttcp.cpp"
        "client/clientudp.cpp"
        "client/client.h" "
        "endpoint/ingress.cpp"
        "endpoint/endpoint.cpp"
        "endpoint/egress.cpp)

# Set library version
set_target_properties(
        ${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_NAME}_VERSION_STRING
)

# In debug mode add more compilation flags
if ((CMAKE_BUILD_TYPE MATCHES Debug)  OR (CMAKE_BUILD_TYPE MATCHES DEBUG))
    target_compile_options(${PROJECT_NAME} PRIVATE -pedantic -ansi -Wall -Wextra)
endif()

# Include src/ directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE "${ironhide_SOURCE_DIR}/src/server")
target_include_directories(${PROJECT_NAME} PRIVATE "${ironhide_SOURCE_DIR}/src/server/utils")

# Optional compilation flags
option(BOOST_THREAD "For boost thread pool support" ON)

set(BUFFER_SIZE 65536)

# If enabled, search and use Boost threads
if (BOOST_THREAD)
    message(STATUS "Trying to use Boost Thread (if Boost version >= 1.66...)")
    # We're going to log only with boost log
    find_package(Boost 1.66 COMPONENTS thread)
    if (Boost_FOUND)
        message(STATUS "Boost Thread enabled")
        set(HAS_BOOST_THREAD TRUE)
    else()
        message(STATUS "Boost Thread disabled")
        set(HAS_BOOST_THREAD FALSE)
    endif()
else()
    message(STATUS "Boost Thread disabled, using standard threads")
    set(HAS_BOOST_THREAD FALSE)
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

configure_file(
        "${ironhide_SOURCE_DIR}/src/config.h.in"
        "${CMAKE_BINARY_DIR}/src/config.h"
        @ONLY
)

# Specify shared library install directory
install(
        TARGETS
        ${PROJECT_NAME}
        DESTINATION
        ${CMAKE_BINARY_DIR}
)