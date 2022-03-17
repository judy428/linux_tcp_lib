/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: buffer.h
Version: 1.0
Date: 2016.1.13

History:
tcpAsio     2021.08.19   1.0     Create
******************************************************************************/
#ifndef _COMMON_H_
#define _COMMON_H_

#include <memory>
#include <thread>



using namespace std;
using namespace base;

namespace tcpCommon
{
    const int64_t serverMaxSendTimes = 1000;        /*服务端最大发送次数*/
    const int readBufSize = 1024 * 1024;

    const int64_t SYSSENDBUF = 1024*1024;     /*服务端发送缓冲区大小*/
    const int64_t SYSRECVBUF = 1024*1024;     /*服务端接收缓冲区大小*/
    const int64_t SYSLISTENMAXCONN = 20;        /*服务端监听请求队列*/

    const int64_t CLIENTCELLRECVBUF = 1024 * 1024;  /*服务器连接接收缓冲区大小*/
    const int64_t CLIENTCELLSENDBUF = 1024 * 1024;  /*服务器连接发送缓冲区大小*/
}


#endif