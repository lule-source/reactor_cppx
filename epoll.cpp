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

Epoll::Epoll() : _epollFd(::epoll_create(MAXEVENTS)), _events(MAXEVENTS)
{
    assert(_epollFd >= 0);
}
Epoll::~Epoll()
{
    ::close(_epollFd);
}

int Epoll::add(int fd, TcpTool *tool, int events)
{
    struct epoll_event event;
    event.data.ptr = (void *)tool;
    event.events = events;
    int ret = ::epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event);
    return ret;
}

int Epoll::del(int fd, TcpTool *tool, int events)
{
    struct epoll_event event;
    event.data.ptr = (void *)tool;
    event.events = events;
    int ret = ::epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, &event);
    return ret;
}

int Epoll::mod(int fd, TcpTool *tool, int events)
{
    struct epoll_event event;
    event.data.ptr = (void *)tool;
    event.events = events;

    int ret = ::epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event);
    return ret;
}

int Epoll::wait(int timeouts)
{
    int eventsNum = ::epoll_wait(_epollFd, &*_events.begin(), static_cast<int>(_events.size()), timeouts);
    if (eventsNum == 0)
    {
    }
    else if (eventsNum < 0)
    {
    }
    return eventsNum;
}

void Epoll::handleEvents(int listenFd, int eventsNum)
{
    assert(eventsNum > 0);
    for (int i = 0; i < eventsNum; ++i)
    {
        TcpTool *tcptool = (TcpTool *)(_events[i].data.ptr);
        // int fd = tcptool->getFd();
        int fd = listenFd;
        if (fd == listenFd)
        {
            _connnectioncb;
        }
        else if (_events[i].events & EPOLLIN)
        {
            _readcb(tcptool);
        }
        else if (_events[i].events & EPOLLOUT)
        {
            _writecb(tcptool);
        }
    }
    return;
}