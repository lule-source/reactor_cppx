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

using namespace std;

#define BUFFERSIE 1024

class TcpServer;
class Epoll;

// =================handle==================
// 处理读写的class, 具体的handle实例
class TcpTool
{
public:
    TcpTool(int fd) : _fd(fd)
    {
        assert(_fd >= 0);
    }
    ~TcpTool() { close(_fd); }

    int getFd() { return _fd; }
    int read();
    int write();

private:
    int _fd;
};

int TcpTool::read()
{
    char buffer[BUFFERSIE];
    memset(buffer, 0, sizeof(buffer));
    int ret = recv(_fd, buffer, sizeof(buffer), 0);
    if (ret == 0)
    { // 断开了链接
        std::cout << "link error!" << std::endl;
    }
    else if (ret > 0)
    {
        std::cout << "recv from client" << std::endl;
    }
    else if (ret == -1)
    {
        std::cout << "recv error" << std::endl;
        std::cout << strerror(errno) << std::endl;
    }
    return ret;
}

int TcpTool::write()
{
    char buffer[BUFFERSIE];
    memset(buffer, 0, sizeof(buffer));
    const char *str = "this is csl server!";
    memcpy(buffer, str, sizeof(str));
    int ret = send(_fd, buffer, sizeof(buffer), 0);
    if (ret > 0)
    {
        std::cout << "send success!" << std::endl;
    }
    else if (ret == -1)
    {
        std::cout << "send error" << std::endl;
    }

    return ret;
}