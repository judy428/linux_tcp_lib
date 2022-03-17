
#ifndef MESSAGE_CENTER_H
#define MESSAGE_CENTER_H

#include <thread>
#include <chrono>
#include <iostream>
#include "../../../base/src/message_base.h"
#include "../../../base/src/thread.h"
#include "../../../base/src/mqueue.h"
#include "../../src/tcp_server_notify.h"

using namespace std;
using namespace base;
using namespace common;
class message_center 
:public message_dispatcher
 ,public tcp_server_hub
{
public:
    message_center(srt_queue<atp_message>* mq);
public:
    virtual int dispatch_message(atp_message& msg);
    virtual bool on_read(int clientid,char* ptr,common::msg_header* m_header);
    
private:
        //lockFreeQueue<atp_message>* m_queue;
        srt_queue<atp_message>* m_queue;
};


#endif