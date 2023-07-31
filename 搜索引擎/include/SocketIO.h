#pragma once
#include <string>

namespace wdcpp
{
/*************************************************************
 *
 *  IO 操作类
 *
 *  1. 提供了多种封装好的 IO 操作
 *  2. 实现具体的 IO 操作
 *
 *************************************************************/
class SocketIO
{
public:
    explicit SocketIO(int);

    size_t readn(void *, size_t);
    size_t writen(const void *, size_t);
    size_t readline(char *, size_t);

private:
    int _fd;
};
};
