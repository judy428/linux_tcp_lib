/*****************************************************************************
 Base Copyright (c) 2021. All Rights Reserved.

 FileName: server.h
 Version: 1.0
 Date: 2021.5.8

 History:
 jasonzhu     2021.5.8   1.0     Create
 ******************************************************************************/

#ifndef _SERVER_H_
#define _SERVER_H_

//#include <thread>
#include <atomic>
#include <iostream>
#include <sys/epoll.h>
#include <poll.h>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>
#include <queue>

#include "../thread.h"
#include "../mqueue.h"
#include "cell_manager.h"

using namespace std;
namespace base{

#define INVALID_SOCKET (-1)

// #define MAXCONN 10
// #define MAXEVENTS 1
// #define SYSSENDBUF (1024 * 1024)
// #define SYSRECVBUF (1024 * 1024)

class tcpServer 
: public process_thread
{
public:
    tcpServer(tNotify* n);
    uint32_t Listen(uint32_t prot);
    ~tcpServer();
    //void OnRun();
    

public:
    virtual void start();
    virtual void stop();
    virtual void run();
    //void closeServer();
public:
    //virtual void run();

	// virtual void start();
	// virtual void stop();
public:
    void CloseClient( int clientid);
    int64_t SendData( int clientid,char* buf, const size_t len);  //此函数将copy buf，内部调用SendBlock
private:
    void HandleAccept();
    cellManager* CurrCellManager( int clientid){return m_CellManager[clientid%m_CellManager.size()];};
private:
  
    int m_ListenFd; //监听fd
    int m_SocketFd;
    uint32_t m_Port;
    
    int m_BindCpu;
    std::vector<cellManager*> m_CellManager;
    tNotify* m_TNotify;
};
}




#endif