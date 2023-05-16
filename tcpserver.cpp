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

class TcpTool;
class Epoll;

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
};

void TcpServer::_acceptConnection()
{
    while (1)
    {
        struct sockaddr_in clitaddr;
        socklen_t len;
        int acceptFd = ::accept(_listenFd, (struct sockaddr *)&clitaddr, &len);
        if (acceptFd == -1)
        {
            if (errno == EAGAIN)
            {
                break;
            }
            std::cout << "accept error!" << std::endl;
        }

        TcpTool *tool = new TcpTool(acceptFd); //
        _epoll->add(acceptFd, tool, (EPOLLIN | EPOLLET));
    }
}

void TcpServer::_closeConnection(TcpTool *tool)
{
    int fd = tool->getFd();
    _epoll->del(fd, tool, 0);
    delete tool;
    tool = nullptr;
}

void TcpServer::_handleWrite(TcpTool *tool)
{
    tool->write();
}

void TcpServer::_handleRead(TcpTool *tool)
{
    int fd = tool->getFd();
    int nRead = tool->read();
    if (nRead == 0 || (nRead < 0 && (errno != EAGAIN)))
    {
        _closeConnection(tool);
        return;
    }

    if (nRead < 0 && errno == EAGAIN)
    {
        _epoll->mod(fd, tool, (EPOLLIN));
        return;
    }
}

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

void TcpServer::run()
{
    // 注册回调函数
    _epoll->add(_listenFd, _listenTool.get(), (EPOLLIN | EPOLLET));
    _epoll->setNewConnection(std::bind(&TcpServer::_acceptConnection, this));
    _epoll->setWriteCb(std::bind(&TcpServer::_handleWrite, this, std::placeholders::_1));
    _epoll->setReadCb(std::bind(&TcpServer::_handleRead, this, std::placeholders::_1));

    // 开始事件循环
    while (1)
    {
        int eventsNum = _epoll->wait(-1);
        if (eventsNum > 0)
        {
            // 事件分发
            _epoll->handleEvents(_listenFd, eventsNum);
        }
    }
}

int main(int argc ,char ** argv){
    int port = 6666;
    if(argc >= 2){
        port = atoi(argv[1]);
    }

    TcpServer serv(port);
    serv.run();

    return 0;