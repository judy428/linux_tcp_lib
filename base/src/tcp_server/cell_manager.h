/*****************************************************************************
 Base Copyright (c) 2021. All Rights Reserved.

 FileName: cell_manager.h
 Version: 1.0
 Date: 2021.5.8

 History:
 jasonzhu     2021.5.8   1.0     Create
 ******************************************************************************/

#ifndef _CELL_MANAGER_H_
#define _CELL_MANAGER_H_

//#include <thread>
#include <atomic>
#include <iostream>
#include <sys/epoll.h>
#include <poll.h>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>
#include <queue>
#include <thread>
#include <memory>

#include "client_cell.h"
#include "../thread.h"
#include "../mqueue.h"
#include "../tcp_common/common.h"

using namespace std;
namespace base{

class cellManager 
    : public process_thread{

    struct cellTask{
        char event;
        int clientid;
        clientCell::sendCell* sC;
    };
public:
    cellManager(tNotify* tN,int cpuN = 0);
    ~cellManager();
public:
    void Handle_recv();
    void Handle_send();
    //void OnRun();
public:
    void start();
    void stop();
    void run();
    
public:
    //void OnRecvData(clientid);

    void addTask(clientCell* cell);
    void delTask(int clientid);
    int64_t sendTask(int clientid,char* src,int size);
public:
    void handleTask();
    void handleAdd(cellTask* ct);
    void handleDel(cellTask* ct);
    void handleSend(cellTask* ct);

private:
    void doEvent(epoll_event* event);
    inline void delClientCell(clientCell* cell);
    void checkWrite();
private:
    void doRecv(int clientid);
    void doSend(int clientid);
    //void doDel(int clientid);
    inline int directSend(int clientid,char* ptr,int size);
private:
    // void handleRead();
private:
    unordered_map<int,clientCell*> m_Cell;
    int m_ListenFd; //监听fd
    int m_BindCpu;
    //ringBuffer<cellTask*>* m_cellTask;
    srt_queue<cellTask*>* m_cellTask;
    // srt_queue<int>* m_handleRead;
    tNotify* m_TNotify;

    // shared_ptr<std::thread> m_th; //出来接受到客户端的数据
};     //管理类

}

#endif