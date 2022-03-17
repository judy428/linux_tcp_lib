/*****************************************************************************
 Base Copyright (c) 2021. All Rights Reserved.

 FileName: server.cpp
 Version: 1.0
 Date: 2021.5.8

 History:
 jasonzhu     2021.5.8   1.0     Create
 ******************************************************************************/

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include "server.h"
#include "../thread.h"
#include "../trace.h"
#include "../base_error.h"
#include "../tools.h"
#include "../tcp_common/common.h"

namespace base{


// int bind_cpu(long threadId,int cpuN)
// {
//     return 0;
//     cpu_set_t cpu_set;
//     CPU_ZERO(&cpu_set);
//     CPU_SET(cpuN,&cpu_set);
//     return pthread_setaffinity_np(threadId,sizeof(cpu_set),&cpu_set);
// }

tcpServer::tcpServer(tNotify* n )
{
    m_ListenFd = 0;
    m_Port = 0;
 
    m_SocketFd = INVALID_SOCKET;
    m_TNotify = n;
    // m_loopnum = std::min(sysconf(_SC_NPROCESSORS_ONLN) * 2 + 2, 16L);
    
    int cpuN = sysconf(_SC_NPROCESSORS_ONLN);
    int loopnum = cpuN * 2 + 2;
    int currCpu = 0;
    m_BindCpu = currCpu++ % cpuN;
    TRACE_INFO(BASE_TAG,"cellManager num:",loopnum);
    cout<<"cellManager num:"<<loopnum<<endl;
    for(auto i = 0;i< loopnum;i++){
        cellManager* c = new cellManager(m_TNotify,currCpu++ % cpuN);
        m_CellManager.push_back(c);
    }
    
}

tcpServer::~tcpServer(){
    if(is_running()) {
        stop();
    }
}

uint32_t tcpServer::Listen(uint32_t port)
{
    
    if (port <= 0){
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_PORT,"params port is error,input is:%d",port);
        return NAUT_AT_E_TCP_SERVER_PORT;
    }
    if (m_SocketFd != INVALID_SOCKET){
        TRACE_SYSTEM(BASE_TAG,"close old socket");
        close(m_SocketFd);
    }
    m_Port = port;

    ////socket
    m_SocketFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (m_SocketFd <= 0){
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_SOCKET,"socker() error,retcode:%d,errno:%d",m_SocketFd,errno);
        exit(NAUT_AT_E_TCP_SERVER_SOCKET);
    }
    
    //
    //系统缓冲区 发送缓冲为32KB，接收缓冲512KB，加大接收缓冲区防止接收堵塞服务端
    int buflen(tcpCommon::SYSSENDBUF);
    ::setsockopt(m_SocketFd, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen));

    buflen = tcpCommon::SYSRECVBUF;
    ::setsockopt(m_SocketFd, SOL_SOCKET, SO_RCVBUF, &buflen, sizeof(buflen));
    
    //端口重用，快速重启服务
    int opt = 1;
    ::setsockopt(m_SocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//端口复用
    ::setsockopt(m_SocketFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof (opt)); //这个设置可有可无
    //禁用nagle算法，提高实时率
    setsockopt(m_SocketFd,IPPROTO_TCP,TCP_NODELAY, &opt, sizeof(opt));

    //设置非阻塞
    int x(fcntl(m_SocketFd, F_GETFL, 0));
    fcntl(m_SocketFd, F_SETFL, x | O_NONBLOCK);

    //bind
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if ( 0 != bind(m_SocketFd,(sockaddr*)&serv_addr,(socklen_t)sizeof(sockaddr_in))){

        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_BIND,"bind() error,retcode:%d,errno:%d",m_SocketFd,errno);
        close(m_SocketFd);
        exit(NAUT_AT_E_TCP_SERVER_BIND);
    }

    if (0 != listen(m_SocketFd,tcpCommon::SYSLISTENMAXCONN)){
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_LISTEN,"listen() error,retcode:%d,errno:%d",m_SocketFd,errno);
        close(m_SocketFd);
        exit(NAUT_AT_E_TCP_SERVER_LISTEN);
    }

    m_ListenFd = epoll_create1(EPOLL_CLOEXEC);
    if (m_ListenFd <= 0){
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_EPOLL_CREATE,"epoll_create1() error,retcode:%d,errno:%d",m_ListenFd,errno);
        close(m_SocketFd);
        exit(NAUT_AT_E_TCP_SERVER_EPOLL_CREATE);
    }

    //add epoll
    epoll_event event;
    event.data.u64 = 0; //清空数据
    event.data.fd = m_SocketFd;    //绑定client指针
    event.events = POLLIN | POLLPRI; 
    epoll_ctl(m_ListenFd, EPOLL_CTL_ADD, m_SocketFd, &event);  
    return NAUT_AT_S_OK;
}

void tcpServer::run(){
    // //bind cpu
    // if(bind_cpu(curr_thread_id(),m_BindCpu) != 0)
    // {
    //     TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_BIND_CPU,"listen epoll bind cpu failed,thread_id:%d",curr_thread_id());
    // }

    epoll_event events;
    while(is_running()){
        int ret =epoll_wait(m_ListenFd,&events,sizeof(events)/sizeof(epoll_event),1000);
        if (ret < 0){
            TRACE_ERROR(BASE_TAG, 0, "epoll_wait error,%d",errno); 
            continue;
        }
        if (ret == 0){
            continue; 
        }
        if(events.data.fd == m_SocketFd){
            TRACE_SYSTEM(BASE_TAG, "new connection");
            HandleAccept();
        }
    }
    TRACE_SYSTEM(BASE_TAG,  "tcp server end");
}


void tcpServer::start(){
    
    for(auto &it : m_CellManager){
        it->start();  
    }
    process_thread::start(); 
}
void tcpServer::stop(){
    process_thread::stop();
    for(auto &i : m_CellManager){
        delete i;
    }
    //TRACE_SYSTEM(BASE_TAG, "m_CellManager delete");
	::close(m_ListenFd);
    close(m_SocketFd);
    m_SocketFd = INVALID_SOCKET;
    m_ListenFd = -1; 
}

void tcpServer::HandleAccept(){
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    socklen_t addrlen = (socklen_t)sizeof(addr);
    auto clientid = accept4(m_SocketFd,(sockaddr *)&addr,&addrlen,SOCK_NONBLOCK | SOCK_CLOEXEC);
    // auto clientid = accept(m_SocketFd,(sockaddr *)&addr,&addrlen);
    if (clientid <= 0){
        TRACE_ERROR(BASE_TAG, 0, "accept4 error,errno:%d",errno);
        return;
    }
    
    //系统缓冲区 发送缓冲为32KB，接收缓冲512KB，加大接收缓冲区防止接收堵塞服务端
    int buflen(tcpCommon::SYSSENDBUF);
    ::setsockopt(clientid, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen));

    buflen = tcpCommon::SYSRECVBUF;
    ::setsockopt(clientid, SOL_SOCKET, SO_RCVBUF, &buflen, sizeof(buflen));

    // //设置tcp_no_delay
    int optval = 1;

    //设置keeplive
    int keepalive = 1; // 开启keepalive属性
    int keepidle = 30; // 如该连接在30秒内没有任何数据往来,则进行探测
    int keepinterval = 10; // 探测时发包的时间间隔为5 秒
    int keepcount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
    ::setsockopt(clientid, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
    ::setsockopt(clientid, SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    ::setsockopt(clientid, SOL_TCP, TCP_KEEPINTVL, &keepinterval, sizeof(keepinterval));
    ::setsockopt(clientid, SOL_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount));

     
    //设置非阻塞
    int x(fcntl(clientid, F_GETFL, 0));
    fcntl(clientid, F_SETFL, x | O_NONBLOCK);

    clientCell* cell = new clientCell(clientid);
    cell->m_ClientFd = clientid;
    //cell->m_Ip = addr.sin_addr;
    cell->m_Port = addr.sin_port;
    ::inet_ntop(AF_INET, &addr.sin_addr, cell->m_Ip, sizeof(cell->m_Ip));
    CurrCellManager(cell->m_ClientFd)->addTask(cell);
    
}

int64_t tcpServer::SendData( int clientid,char* buf, const size_t len){
    CurrCellManager(clientid)->sendTask(clientid,buf,len);
}

}