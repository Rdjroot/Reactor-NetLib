# 配置项目名称，编译版本、标志
cmake_minimum_required(VERSION 3.10)

project(EchoServerProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3 -pg")

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 把所有源文件列在LIB_SOURCES变量中
set(LIB_SOURCES
    src/InetAddress.cpp
    src/Socket.cpp
    src/Epoll.cpp
    src/Channel.cpp
    src/EventLoop.cpp
    src/TcpServer.cpp
    src/Acceptor.cpp
    src/Connection.cpp
    src/Buffer.cpp
    src/EchoServer.cpp
    src/ThreadPool.cpp
    src/Timestamp.cpp
    src/Logger.cpp
)

# 目标执行文件
add_executable(client client.cpp)
add_executable(echoserver echoserver.cpp ${LIB_SOURCES})

# 链接系统库 pthread
target_link_libraries(echoserver pthread)

# 确保log目录存在
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/log)

# 自定义clean_all目标
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeTmp
)

# 自定义clean_logs目标
add_custom_target(clean_logs
    COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_SOURCE_DIR}/log/*.log
)
