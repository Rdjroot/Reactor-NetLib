# Makefile

# 编译器及其选项
CXX = g++
CXXFLAGS = -g -Wall

# 目标文件目录
OBJDIR = obj
OBJS = $(OBJDIR)/InetAddress.o $(OBJDIR)/Socket.o $(OBJDIR)/Epoll.o $(OBJDIR)/Channel.o $(OBJDIR)/EventLoop.o $(OBJDIR)/TcpServer.o $(OBJDIR)/Acceptor.o $(OBJDIR)/Connection.o $(OBJDIR)/Buffer.o $(OBJDIR)/EchoServer.o $(OBJDIR)/ThreadPool.o

# 静态库
LIB = libmylib.a

all: client echoserver

# 生成静态库
$(LIB): $(OBJS)
	ar rcs $@ $^

# 确保目标文件目录存在
$(OBJDIR):
	mkdir -p $(OBJDIR)

# 生成目标文件
# $<：代表第一个依赖文件的名字。在这里$< 代表对应的 .cpp 文件。
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 生成可执行文件 client
# $@：代表规则中的目标文件名； 在这里$@ 代表 client。
# $^：代表规则中的所有依赖文件列表。 在这里$^ 代表 client.o libmylib.a。
client: client.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

client.o: client.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 生成可执行文件 echoserver
echoserver: echoserver.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

echoserver.o: echoserver.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -lpthread

# 清理
clean:
	rm -f client echoserver $(OBJDIR)/*.o $(LIB)
	rmdir $(OBJDIR)

