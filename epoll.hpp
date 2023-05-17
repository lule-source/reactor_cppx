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
#define MAXEVENTS 1024

class Epoll
{
public:
    Epoll();
    ~Epoll();

    using NewConnectionCallback = std::function<void()>;
    using HandleReadCallback = std::function<void(TcpTool *)>;
    using HandleWriteCallback = std::function<void(TcpTool *)>;
    using CloseConnectionCallback = std::function<void(TcpTool *)>;
    int add(int fd, TcpTool *tool, int events);
    int mod(int fd, TcpTool *tool, int events);
    int del(int fd, TcpTool *tool, int events);
    int wait(int timeouts);

    void handleEvents(int listenFd, int eventsNum);

    // 事件回调函数
    void setNewConnection(const NewConnectionCallback &cb)
    {
        _connnectioncb = cb;
    }

    void setCloseConnection(const CloseConnectionCallback &cb)
    {
        _closecb = cb;
    }

    void setWriteCb(const HandleWriteCallback &cb)
    {
        _writecb = cb;
    }

    void setReadCb(const HandleReadCallback &cb)
    {
        _readcb = cb;
    }

private:
    using eventList = std::vector<struct epoll_event>;
    int _epollFd;
    eventList _events; // 事件集合
    NewConnectionCallback _connnectioncb;
    HandleReadCallback _readcb;
    HandleWriteCallback _writecb;
    CloseConnectionCallback _closecb;
};