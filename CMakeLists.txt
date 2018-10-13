cmake_minimum_required (VERSION 2.6)
project (IPC_Server_Client)


# The version number.
set (CMAKE_CXX_STANDARD 17)
 
# add the binary tree to the search path for include files

set(INCLUDE_DIRS cereal-1.2.2/include/ boost_1_68_0)
set(SERVER_SOURCES src/server/main.cpp)
set(CLIENT_SOURCES src/client/main.cpp)
set(COMMON_SOURCES src/common/IpcInterface.cpp)

set(PROJ_DEPS $ENV{HOME}/navisens/navisens-dependencies/build/boost/libboost_universal_osx.a)

include_directories("${PROJECT_BINARY_DIR}" ${INCLUDE_DIRS} include)

add_executable(server ${COMMON_SOURCES} ${SERVER_SOURCES})
target_link_libraries(server ${PROJ_DEPS})

add_executable(client ${COMMON_SOURCES} ${CLIENT_SOURCES})
target_link_libraries(client ${PROJ_DEPS})
