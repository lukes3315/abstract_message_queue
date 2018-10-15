cmake_minimum_required (VERSION 2.6)
project (IPC_Server_Client)


# The version number.
set (CMAKE_CXX_STANDARD 17)
 
# add the binary tree to the search path for include files

set(INCLUDE_DIRS boost_1_68_0)
set(SERVER_SOURCES src/server/main.cpp)
set(CLIENT_SOURCES src/client/main.cpp)

include_directories("${PROJECT_BINARY_DIR}" ${INCLUDE_DIRS} include)

add_executable(server ${SERVER_SOURCES})
target_link_libraries(server)

add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client)
