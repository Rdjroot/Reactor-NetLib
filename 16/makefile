all:bankclient bankserver

bankclient:bankclient.cpp
	g++ -O3 -g -o bankclient bankclient.cpp

bankserver:bankserver.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp BankServer.cpp ThreadPool.cpp Timestamp.cpp
	g++ -O3 -g -o bankserver bankserver.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp BankServer.cpp ThreadPool.cpp Timestamp.cpp -lpthread

clean:
	rm -f bankclient bankserver
