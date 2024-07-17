# Makefile

# 编译器及其选项
CXX = g++
CXXFLAGS = -g -Wall

# 目标文件目录
OBJDIR = obj
OBJS = $(OBJDIR)/InetAddress.o $(OBJDIR)/Socket.o $(OBJDIR)/Epoll.o $(OBJDIR)/Channel.o $(OBJDIR)/EventLoop.o $(OBJDIR)/TcpServer.o $(OBJDIR)/Acceptor.o $(OBJDIR)/Connection.o $(OBJDIR)/Buffer.o $(OBJDIR)/EchoServer.o $(OBJDIR)/ThreadPool.o

# 动态库
LIB = libmylib.so

all: client echoserver

# 生成动态库
$(LIB): $(OBJS)
	$(CXX) -shared -o $@ $^

# 确保目标文件目录存在
$(OBJDIR):
	mkdir -p $(OBJDIR)

# 生成目标文件
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

# 生成可执行文件 client
client: client.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lmylib

client.o: client.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 生成可执行文件 echoserver
echoserver: echoserver.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lmylib -lpthread

echoserver.o: echoserver.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -lpthread

# 清理
clean:
	rm -f client echoserver $(OBJDIR)/*.o $(LIB)
	rmdir $(OBJDIR)

