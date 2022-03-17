/*****************************************************************************
 Base Copyright (c) 2021. All Rights Reserved.

 FileName: client_cell.h
 Version: 1.0
 Date: 2021.5.8

 History:
 jasonzhu     2021.5.8   1.0     Create
 ******************************************************************************/

#ifndef _CLIENT_CELL_H_
#define _CLIENT_CELL_H_

//#include <thread>
#include <atomic>
#include <iostream>
#include <sys/epoll.h>
#include <poll.h>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>
#include <queue>

#include "notify.h"
#include "../thread.h"
#include "../mqueue.h"
#include "../tcp_common/common.h"
// #include "../tcp_common/send_cell.h"
// #include "../tcp_common/micro_buffer.h"

using namespace std;
//using namespace tcpCommon;
namespace base{

// #define RECVBUF (1024 * 1024)
// #define SENDBUF (1024 * 1024)


class clientCell {
public:
    struct sendCell{
        char* ptr;
        int size;
        int sendPos = 0;
        atomic<bool> enableRead;
    };
public:
    clientCell(int listenfd){m_ClientFd = listenfd;}
    ~clientCell();

    int m_ClientFd;
    char m_RecvBuf[tcpCommon::CLIENTCELLRECVBUF];
    sendCell* m_SendBuf[tcpCommon::CLIENTCELLSENDBUF];
    uint32_t m_RecvReadPos = 0;
    uint32_t m_RecvWritePos = 0;
    uint32_t m_SendReadPos = 0;
    uint32_t m_SendWritePos = 0;
    uint32_t m_Events;
    char m_Ip[21];              //对端ip
    unsigned short m_Port;        //对端port

    //shared_ptr<micro_buffer> m_read_buffer;
    //shared_ptr<send_cell> m_write_cell;
};




}

#endif