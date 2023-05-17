
#include "epoll.hpp"

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