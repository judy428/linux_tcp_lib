/*****************************************************************************
 Base Copyright (c) 2021. All Rights Reserved.

 FileName: client_cell.cpp
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
#include "../thread.h"
#include "../trace.h"
#include "../base_error.h"
#include "client_cell.h"

namespace base{


clientCell::~clientCell(){
    if(m_SendReadPos != m_SendWritePos){
        TRACE_SYSTEM(BASE_TAG,"free buf,clientid:%d,m_SendReadPos:%d,m_SendWritePos:%d",m_ClientFd, m_SendReadPos,m_SendWritePos);
    }
    while (m_SendReadPos != m_SendWritePos)
    {
        //TRACE_SYSTEM(BASE_TAG,"free buf,clientid:%d,m_SendReadPos:%d,m_SendWritePos:%d",m_ClientFd, m_SendReadPos,m_SendWritePos);
        delete [](m_SendBuf[m_SendReadPos]->ptr);
        m_SendBuf[m_SendReadPos]->ptr = nullptr;
        delete m_SendBuf[m_SendReadPos];
        m_SendBuf[m_SendReadPos] = nullptr;
        m_SendReadPos++;
        m_SendReadPos %= tcpCommon::CLIENTCELLSENDBUF;
    }
}

}