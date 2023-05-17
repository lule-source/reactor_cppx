#include "tcpserver.hpp"

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