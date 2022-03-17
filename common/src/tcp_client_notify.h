/*****************************************************************************
 Risk_Ctrl Module trade_gateway Copyright (c) 2017. All Rights Reserved.

 FileName: tcp_server_notify.h
 Version: 1.0
 Date: 2017.04.25

 History:
 zhujy     2017.04.25   1.0     Create
 ******************************************************************************/

#ifndef TCP_CLIENT_NOTIFY_H_
#define TCP_CLIENT_NOTIFY_H_

#include "../../base/src/tcp_client/tcpClient.h"
#include "common.h"
#include "tcp_struct.h"
#include "../../base/src/message_base.h"

using namespace base;
namespace common
{
class tcp_client_notify
        : public base::tCliNotify
{
public:
    tcp_client_notify();
    virtual ~tcp_client_notify();
private:
    virtual void OnConnect(easyTcpClient* self);
    virtual void OnDisconnect(easyTcpClient* self);
    virtual size_t OnRecvData(easyTcpClient* self, const char* buf, const size_t len);
    virtual void handle_msg(atp_message& msg) = 0;
private:
    const size_t m_msg_header_size_;
    char* m_msg_cache;
    int64_t m_cache_size;
};
}


#endif /* TCP_SERVER_NOTIFY_H_ */
