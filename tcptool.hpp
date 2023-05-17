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