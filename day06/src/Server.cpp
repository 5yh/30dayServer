#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include <functional>
#include <string.h>
#include <unistd.h>

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop) : loop(_loop)
{
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    serv_sock->listen();
    serv_sock->setnonblocking();

    // 把fd包起来
    Channel *servChannel = new Channel(loop, serv_sock->getFd());
    // 模板类，存储任何可调用对象
    // bind将函数和其参数绑定在一起
    // cb被调用时实际上时调用newConnection，参数为serv_sock
    std::function<void()> cb = std::bind(&Server::newConnection, this, serv_sock);
    // 绑定为回调函数，可以更好地支持异步调用、事件驱动编程、代码解耦和延迟执行等需求。
    servChannel->setCallback(cb);
    servChannel->enableReading();
}

Server::~Server()
{
}
void Server::handleReadEvent(int sockfd)
{
    char buf[READ_BUFFER];
    while (true)
    { // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0)
        {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        }
        else if (bytes_read == -1 && errno == EINTR)
        { // 客户端正常中断、继续读取
            printf("continue reading");
            continue;
        }
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        { // 非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        }
        else if (bytes_read == 0)
        { // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd); // 关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}

void Server::newConnection(Socket *serv_sock)
{
    InetAddress *clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    clnt_sock->setnonblocking();
    Channel *clntChannel = new Channel(loop, clnt_sock->getFd());
    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clnt_sock->getFd());
    clntChannel->setCallback(cb);
    clntChannel->enableReading();
}