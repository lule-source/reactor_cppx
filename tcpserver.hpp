#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <sys/epoll.h>
#include <chrono>
#include <cassert>
#include "epoll.hpp"
#include "tcptool.hpp"


// 起到了一个reactor角色作用
class TcpServer
{
public:
    TcpServer(int port) : _port(port), // 6666
                          _listenFd(createListenFd(_port)),
                          _listenTool(new TcpTool(_listenFd)),
                          _epoll(new Epoll())
    {
        // 非阻塞
        assert(_listenFd >= 0);
    }

    void run();

private:
    // 连接、断开
    void _acceptConnection();
    void _closeConnection(TcpTool *tool);
    // 处理读写
    void _handleWrite(TcpTool *tool);
    void _handleRead(TcpTool *tool);

private:
    using EpollPtr = std::unique_ptr<Epoll>;
    using ListenTcpToolPtr = std::unique_ptr<TcpTool>;

private:
    int _port;
    int _listenFd;
    ListenTcpToolPtr _listenTool; //
    EpollPtr _epoll;              // 只能有一个epoll实例，所以使用unique_ptr  自动释放

    int createListenFd(int port)
{
    // 防止port
    int _listenFd = 0;
    _listenFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    struct sockaddr_in _servAddr;
    port = ((port <= 1024) || (port >= 65536)) ? 6666 : port;
    ::memset(&_servAddr, 0, sizeof(_servAddr));
    _servAddr.sin_family = AF_INET;
    _servAddr.sin_port = ::htons((unsigned short)port);
    _servAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    if ((::bind(_listenFd, (struct sockaddr *)&_servAddr, sizeof(_servAddr))) == -1)
    {
        std::cout << "bind error!" << std::endl;
        return -1;
    }

    if ((::listen(_listenFd, 5)) == -1)
    {
        std::cout << "listen error!" << std::endl;
        return -1;
    }

    return _listenFd;
}
};