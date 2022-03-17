/*****************************************************************************
 Base Copyright (c) 2021. All Rights Reserved.

 FileName: notify.h
 Version: 1.0
 Date: 2021.5.8

 History:
 jasonzhu     2021.5.8   1.0     Create
 ******************************************************************************/

#ifndef _NOTIFY_H_
#define _NOTIFY_H_

//#include <thread>
#include <atomic>
#include <iostream>
#include <sys/epoll.h>
#include <poll.h>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>
#include <queue>

using namespace std;
namespace base{

class tNotify{
    public: //保证OnConnect，OnRecvData，OnDisconnect调用顺序，都在同一个EventLoop线程
    virtual void OnConnect(const unsigned int clientid, const char* ip, const unsigned short port) = 0;
    virtual void OnDisconnect(const unsigned int clientid) = 0;
    virtual size_t OnRecvData(const unsigned int clientid, const char* buf, const size_t len) = 0;
    virtual void OnError(const unsigned int clientid, const int errorid) = 0; //(errorid > 0); -2:接受缓冲区已满; -3:发送接收缓存已满; 
};

}

#endif