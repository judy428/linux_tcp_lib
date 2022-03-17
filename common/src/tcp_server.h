/*****************************************************************************
 common Module Copyright (c) 2017. All Rights Reserved.

 FileName: tcp_server.h
 Version: 1.0
 Date: 2017.04.10

 History:
 zhujy     2017.04.27   1.0     Create
 ******************************************************************************/

#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include"tcp_server_notify.h"
#include "../../base/src/tcp_server/server.h"

using namespace base;
namespace common
{
class tcp_server
{
public:
	tcp_server();
	virtual ~tcp_server();
public:
	void start();
	void stop();
	void init(int port,tcp_server_hub* mdpt);
	int64_t senddata(int clientid,char* src,int size){return m_tcpserver_->SendData(clientid,src,size);};
private:
	tcp_server_hub* mdpt_;
	base::tcpServer*    m_tcpserver_;
	int m_port_;
	tcp_server_notify m_tcp_notify;
};
}

#endif /* TCP_SERVER_H_ */
