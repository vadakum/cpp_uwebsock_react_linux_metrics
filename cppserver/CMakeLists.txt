cmake_minimum_required(VERSION 3.10.0)
project(cppserver VERSION 0.1.0 LANGUAGES C CXX)

# Set the C++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCE_FILES "src/*.cpp")

add_executable(cppserver ${SOURCE_FILES})

# --- Library Integration Section ---
set(UWEBSOCK_INC_PATH "/usr/local/include/uWebSockets/")
set(JSON_INC_PATH "${CMAKE_SOURCE_DIR}/json")
set(LINUX_SYS_STATS_INC_PATH "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/include/tp")

set(UWEBSOCK_LIB_PATH "/usr/local/lib/")

target_include_directories(cppserver PUBLIC ${UWEBSOCK_INC_PATH})
target_include_directories(cppserver PRIVATE ${LINUX_SYS_STATS_INC_PATH} ${JSON_INC_PATH})

target_link_directories(cppserver PUBLIC ${UWEBSOCK_LIB_PATH})
target_link_libraries(cppserver PUBLIC uSockets ssl crypto z pthread)