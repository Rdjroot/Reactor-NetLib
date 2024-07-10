# Makefile

# 编译器及其选项
CXX = g++
CXXFLAGS = -g -Wall

# 目标文件
OBJS = InetAddress.o Socket.o Epoll.o Channel.o EventLoop.o TcpServer.o Acceptor.o Connection.o Buffer.o

# 静态库
LIB = libmylib.a

all: client tcpepoll

# 生成静态库
$(LIB): $(OBJS)
	ar rcs $@ $^

# 生成目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 生成可执行文件 client
client: client.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ client.o $(LIB)

# 生成可执行文件 tcpepoll
tcpepoll: tcpepoll.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ tcpepoll.o $(LIB)

# 清理
clean:
	rm -f client tcpepoll $(OBJS) client.o tcpepoll.o $(LIB)
