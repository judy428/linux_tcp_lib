
#include "message_center.h"

#include "../structInfo.h"

using namespace base;

message_center::message_center(srt_queue<atp_message>* mq){
    m_queue = mq ;//new lockFreeQueue<atp_message>(1 << 10, nullptr);
}

bool message_center::on_read(int clientid, char* ptr, common::msg_header* m_header){
    atp_message msg;
    msg.type = m_header->type;
    
    //cout<<m_header->data_size<<endl;
    int32 size = (m_header->data_size/32 + 1)*32;  
    msg.param1 = new char[size];
    // delete[] msg.param1;
    // return true;
    //memset(msg.param1,0,m_header->data_size + 1);
    //memcpy(msg.param1,ptr,m_header->data_size);
    msg.param2 = new int32_t(clientid);
    
    return dispatch_message(msg) == 0? true:false;
}

 int message_center::dispatch_message(atp_message& msg){
    // delete [](msg.param1);
    // delete (int *)(msg.param2);
    // msg.param1 = nullptr;
    // msg.param2 = nullptr;
    // return 0;
    // delete[] msg.param1;
    // delete msg.param2;
    // return 0;
    if(m_queue->push(msg)){
    //    atp_message msg2;
    //    if(m_queue->pop(msg2)){
    //        if(msg2.param1 != nullptr){
    //             delete[] msg2.param1;
    //             delete msg2.param2;
    //        }  
    //    } 
        // delete[] msg.param1;
        // delete msg.param2;
        // atp_message msg1;
        // m_queue->pop(msg1);
        // delete [](msg1.param1);
        // delete (int *)(msg1.param2);
        // msg1.param1 = nullptr;
        // msg1.param2 = nullptr;
        return 0;
    }else{
        cout<<"eeeeeeeeeeeeeeeeeeeeee"<<endl;
    }
    return  -1;
}