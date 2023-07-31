#pragma once
#include "Socket.h"
#include "SocketIO.h"
#include "InetAddress.h"

#include <memory>
#include <functional>
using std::function;
using std::shared_ptr;

namespace wdcpp
{
/*************************************************************
 *
 *  TCP 连接类
 *
 *  1. 四元组，描述了一条已建立的连接
 *  2. 作为与客户交互的接口，提供 msg 发送/接收的接口
 *  3. 首先要从 Acceptor 中获取 clientSock 再建立该连接类，此后就可以
 *     通过该类与 client 进行交互了
 *
 *************************************************************/
class EventLoop; // 前向声明（不要再本文件中直接 #include "EventLoop.h" 以防发生循环引用）
class TcpConnection;
using TcpConnectionPtr = shared_ptr<TcpConnection>; // 便于命名空间中全局访问

class TcpConnection
    : NonCopyable,
      public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionCallBack = function<void(const TcpConnectionPtr &)>;

public:
    explicit TcpConnection(int, EventLoop *);

    void send(const string &);
    string recv();
    string recvLine();
    string show();

    bool isClosed() const;

    void setConnectionCallBack(const TcpConnectionCallBack &);
    void setMessageCallBack(const TcpConnectionCallBack &);
    void setCloseCallBack(const TcpConnectionCallBack &);
    void handleConnectionCallBack();
    void handleMessageCallBack();
    void handleCloseCallBack();

    void notifyLoop(const string &);

private:
    InetAddress getLocalAddr();
    InetAddress getPeerAddr();

private:
    Socket _clientSock;
    SocketIO _sockIO;
    InetAddress _localAddr;
    InetAddress _peerAddr;
    bool _isShutDownWrite;
    TcpConnectionCallBack _onConnectionCb; // 新连接事件的事件处理器（回调函数）
    TcpConnectionCallBack _onMessageCb;    // 新消息事件的事件处理器
    TcpConnectionCallBack _onCloseCb;      // 连接断开事件的事件处理器
    EventLoop *_loopPtr;
};
};
