/*****************************************************************************
 common Module trade_gateway Copyright (c) 2017. All Rights Reserved.

 FileName: tcp_server_notify.cpp
 Version: 1.0
 Date: 2017.04.25

 History:
 zhujy     2017.04.25   1.0     Create
 ******************************************************************************/
#include <assert.h>
#include <stdio.h>
#include <memory>
#include "tcp_server_notify.h"
#include "common.h"

#include "../../base/src/trace.h"


namespace common
{
tcp_server_notify::tcp_server_notify()
: m_msg_header_size_(sizeof(common::msg_header)),
  mdpt_(NULL)
{
}

tcp_server_notify::~tcp_server_notify()
{	
}

void tcp_server_notify::init(tcp_server_hub* mdpt)
{
	mdpt_ = mdpt;
}

void tcp_server_notify::OnConnect(const unsigned int clientid, const char* ip, const unsigned short port)
{
	TRACE_INFO(COMMON_TAG,"server has connected with client,fd:%d,ip:%s,port:%d",clientid,ip,port);
}

void tcp_server_notify::OnDisconnect(const unsigned int clientid)
{
	TRACE_INFO(COMMON_TAG,"server has disconnected with client,fd:%d",clientid);
}

size_t tcp_server_notify::OnRecvData(const unsigned int clientid, const char* recvbuf, const size_t len)
{
	//TRACE_INFO(COMMON_TAG,"server has recv msg,clliendid:%d",clientid);
	if(clientid <= 0 || len <= 0 || recvbuf == NULL)
	{
		TRACE_ERROR(COMMON_TAG,NAUT_AT_E_TCP_SERVER_NOTIFY_PARAM_ERROR,"params error,clliendid:%d,len:%d",clientid,len);
		return -1;
	}
	char* pstr = (char*)recvbuf;
	size_t size = len;
	
	size_t readpos = 0;
	while(static_cast<int>(size - readpos) > static_cast<int>(m_msg_header_size_)) {
		common::msg_header* m_header = (common::msg_header*)(pstr + readpos);
		if (m_header->data_size <= 0) {
		    break;
		}
		
		if(static_cast<int>(size - readpos -m_msg_header_size_ - m_header->data_size) >= 0)
		{
#ifdef USE_DISPATCH


			atp_message msg;
			msg.type = m_header->type;
			//cout<<m_header->data_size<<endl;
			msg.param1 = new char[m_header->data_size + 1];
			memset(msg.param1,0,m_header->data_size + 1);
			memcpy(msg.param1,pstr + readpos + m_msg_header_size_,m_header->data_size);
			msg.param2 = new int(clientid);

			mdpt_->dispatch_message(msg);
			readpos += m_msg_header_size_ + m_header->data_size;
#else
			if(mdpt_->on_read(clientid,pstr + readpos + m_msg_header_size_,m_header)){
				readpos += m_msg_header_size_ + m_header->data_size;
			}else{
				TRACE_SYSTEM("common","on_read() error");
				break;
			}
			//mdpt_->on_read(clientid,pstr + readpos + m_msg_header_size_,m_header);
			// mdpt_->on_read(1,nullptr,nullptr);
			// readpos += m_msg_header_size_ + m_header->data_size;
#endif
		}
		else
		{
			break;
		}
	}
	return readpos;
}


void tcp_server_notify::OnError(const unsigned int clientid, const int errorid)
{
	TRACE_ERROR(COMMON_TAG,NAUT_AT_E_TCP_SERVER_NOTIFY_RECV_ERROR,"recv err msg,errcode:%d",errorid);
}
}


