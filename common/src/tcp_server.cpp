/*****************************************************************************
 common Module trade_gateway Copyright (c) 2017. All Rights Reserved.

 FileName: tcp_server.cpp
 Version: 1.0
 Date: 2017.04.10

 History:
 zhujy     2017.04.27   1.0     Create
 ******************************************************************************/

#include "tcp_server.h"

namespace common{
tcp_server::tcp_server()
:mdpt_(NULL)
,m_tcpserver_(NULL)
{

}

tcp_server::~tcp_server()
{
	stop();
}

void tcp_server::init(int port,tcp_server_hub* mdpt)
{
	mdpt_ = mdpt;
	m_port_ = port;
}

void tcp_server::start()
{
	m_tcp_notify.init(mdpt_);
	m_tcpserver_ = new tcpServer(&m_tcp_notify);
	m_tcpserver_->Listen(m_port_);
	m_tcpserver_->start();
}

void tcp_server::stop()
{
	 if (m_tcpserver_ != NULL)  {
		 m_tcpserver_->stop();
	    delete m_tcpserver_;
	    m_tcpserver_ = NULL;
	 }
}

}




