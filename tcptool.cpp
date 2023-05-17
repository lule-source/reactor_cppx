#include "tcptool.hpp"

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