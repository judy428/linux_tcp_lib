/*****************************************************************************
 common Module trade_gateway Copyright (c) 2017. All Rights Reserved.

 FileName: tcp_server_notify.h
 Version: 1.0
 Date: 2017.04.25

 History:
 zhujy     2017.04.25   1.0     Create
 ******************************************************************************/

#ifndef TCP_SERVER_NOTIFY_H_
#define TCP_SERVER_NOTIFY_H_

#include <map>
#include "../../base/src/tcp_server/notify.h"
#include "../../base/src/message_base.h"
#include "tcp_struct.h"

using namespace base;

namespace common
{

// typedef std::map<int,buf_info> m_int_buf;
// typedef std::map<int,bool> map_new_flag;
class tcp_server_hub{
public:
	virtual bool on_read(int clientid, char* ptr, common::msg_header* m_header) = 0;
};

class tcp_server_notify
: public tNotify
{
public:
	tcp_server_notify();
	virtual ~tcp_server_notify();
	void init(tcp_server_hub* mdpt);
public:
	virtual void OnConnect(const unsigned int clientid, const char* ip, const unsigned short port);
	virtual void OnDisconnect(const unsigned int clientid);
	virtual size_t OnRecvData(const unsigned int clientid, const char* buf, const size_t len);
	virtual void OnError(const unsigned int clientid, const int errorid);
private:
	const size_t m_msg_header_size_;
	tcp_server_hub* mdpt_;
};
}


#endif /* TCP_SERVER_NOTIFY_H_ */
