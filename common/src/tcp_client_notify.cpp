/*****************************************************************************
 Risk_Ctrl Module Copyright (c) 2017. All Rights Reserved.

 FileName: tcp_server_notify.cpp
 Version: 1.0
 Date: 2017.04.25

 History:
 zhujy     2017.04.25   1.0     Create
 ******************************************************************************/

#include <stdio.h>
#include <string>
#include <iostream>


#include "tcp_client_notify.h"
#include "../../base/src/trace.h"

using namespace std;
namespace common
{
tcp_client_notify::tcp_client_notify()
: m_msg_header_size_(sizeof(common::msg_header)),m_cache_size(1024 * 8)
{
    m_msg_cache = new char[m_cache_size];
}

tcp_client_notify::~tcp_client_notify()
{
    delete[] m_msg_cache;
}

void tcp_client_notify::OnConnect(easyTcpClient* self)
{
    TRACE_INFO(COMMON_TAG,"client has connected");
}

void tcp_client_notify::OnDisconnect(easyTcpClient* self)
{
    TRACE_INFO(COMMON_TAG,"client has disconnected");
}


size_t tcp_client_notify::OnRecvData(easyTcpClient* self, const char* buf, const size_t len)
{
    
    //TRACE_SYSTEM(COMMON_TAG,NAUT_AT_E_TCP_CLIENT_NOTIFY_PARAM_ERROR,"recv msg,len:%d",len);
    if(len <= 0 || buf == NULL) {
        TRACE_ERROR(COMMON_TAG,NAUT_AT_E_TCP_CLIENT_NOTIFY_PARAM_ERROR,"params error,len:%d",len);
        return -1;
    }
    char* pstr = (char*)buf;
    size_t size = len;
    size_t readpos = 0;
    while(static_cast<int>(size - readpos) > static_cast<int>(m_msg_header_size_)) {
        common::msg_header* m_header = (common::msg_header*)(pstr + readpos);
        if (m_header->data_size <= 0) {
            break;
        }
        atp_message msg;
        if(static_cast<int>(size-readpos-
                m_msg_header_size_-m_header->data_size) >= 0) {
            //如果确保回调调用是个单线程，本处可以不用每次都分配内存，使用类的私有成员。这样会降低很多时间消耗
            char* msg_info = m_msg_cache;
            if(m_cache_size < m_header->data_size + 1){
                msg_info = new char[m_header->data_size + 1];
            }
            memset(msg_info, 0, m_header->data_size + 1);
            memcpy(msg_info, pstr + readpos+m_msg_header_size_, m_header->data_size);
            msg.type = m_header->type;
            msg.param1 = msg_info;
            handle_msg(msg);
            readpos += m_msg_header_size_ + m_header->data_size;
            if(m_cache_size < m_header->data_size + 1){
                delete[]  msg_info;
            }
            msg.param1 = nullptr;
        }
        else {
            break;
        }
    }
   
    return readpos;
}

}

