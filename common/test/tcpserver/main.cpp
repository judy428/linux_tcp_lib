#include <thread>
#include <chrono>
#include <iomanip> 
#include <stdlib.h>
#include "../../src/tcp_server.h"
#include "message_center.h"
#include "../structInfo.h"
#include "../../../base/src/trace.h"
#include "../../../base/src/mqueue.h"
#include "./ring_buffer/ringBuffer.h"

using namespace common;
using namespace base;
using namespace std;
const int MAXSENDBUF = 1024 *4;
const int threadNum = 30;

tcp_server* m_tcp_server_ = nullptr;

atomic<int64_t> total_recv_num ;
atomic<int64_t> total_send_num;
bool isRunning;

void handlemsg(ringBuffer<atp_message>& m_queue,bool isRunning)
{
    atp_message msg;
    common::msg_header m_header;
    char* ptr =  new char[MAXSENDBUF];
    char data[100];
    
    while (isRunning)
    {
        if(m_queue.pop(msg)){
            // delete[] (msg.param1);
            //     // delete[] msg.param1;
            // delete msg.param2;
            // continue;
            // cout<<"delete"<<endl;
            // delete[] (msg.param1);
            // delete (msg.param2);
            // msg.param1 = nullptr;
            // msg.param2 = nullptr;
            // continue;
            if(msg.param1 != nullptr){
                
                switch (msg.type)
                {
                case CMD_LOGIN:
                {
                    //delete[] (msg.param1);
                    Login* lg = (Login *)(msg.param1);
                    //delete lg;
                    //TRACE_SYSTEM("test","recv msg,lg->usrName:%s,lg->passWd:%s",lg->usrName,lg->passWd);
                    
                    //char data[100];
                    sprintf(data,"wsc:%d",total_recv_num++);
                        LoginResult* lr = new LoginResult();
                    lr->result = 2;
                    strcpy(lr->data,data);
                    
                    m_header.data_size = sizeof(LoginResult);
                    m_header.type = CMD_LOGIN_RESULT;
                    memset(ptr,0,sizeof(m_header) +  sizeof(LoginResult) + 1);
                    memcpy(ptr,&m_header,sizeof(m_header));
                    memcpy(ptr + sizeof(m_header),lr,sizeof(LoginResult));
                    
                    auto ret = m_tcp_server_->senddata(*((int *)(msg.param2) ),ptr,sizeof(m_header) +  sizeof(LoginResult));
                    total_send_num++;
                    if(ret != 0){
                        cout<<"senddata() error,errcode:"<<ret<<endl;
                    }
                    //delete []ptr;
                    //ptr = nullptr;
                    delete lr;
                    lr = nullptr;
                    // memset(msg.param1,'\0',sizeof(Login) + 1);
                    // delete[] msg.param1;
                    // msg.param1 = nullptr;
                    //delete lg;
                    //lg = nullptr;
                    // TRACE_SYSTEM("test","recv msg,lg->usrName:%s,lg->passWd:%s",lg->usrName,lg->passWd);
                }
                break;
                default:
                    break;
                }

                //释放内存
                //cout<<"address:"<<msg.param1<<endl;
                // delete[] msg.param1;
                delete[] (msg.param1);
                // delete[] msg.param1;
                delete (int32_t *)(msg.param2);
                msg.param1 = nullptr;
                msg.param2 = nullptr;
            }
            
        }else{
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
            //cout<<"now total_recv_num:"<<total_recv_num<<",total_send_num"<<total_send_num<<endl;
           
        }
    }
    delete[] ptr;
}
int main(){
    srt_queue<atp_message> m_queue;
    m_queue.init();

    total_recv_num = 0;
    total_send_num = 0;
    message_center m_msg(&m_queue);
   
    m_tcp_server_ = new tcp_server();
    m_tcp_server_->init(3319,&m_msg);
    m_tcp_server_->start();
    ringBuffer<atp_message> mq[threadNum];
    std::thread th[threadNum];
    isRunning = true;
    
    for(int i = 0;i<threadNum;i++){
        th[i] = std::thread(handlemsg,std::ref(mq[i]),isRunning);
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    
    
    //cout<<"-------------------------------size:"<<m_queue.size()<<endl;
    atp_message msg;
    //sleep(1);
    int total_num = 100;
    //int recv_msg = 0;
    // std::this_thread::sleep_for(std::chrono::minutes(10));
    // return 0;
    int insertTh = 0;
    while (1)
    {
        if(m_queue.pop(msg)){
            if(msg.param1 != nullptr){
                insertTh = (++insertTh) % threadNum;
                mq[insertTh].push(msg);
            }
        }else{
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
            //cout<<"now total_recv_num:"<<total_recv_num<<",total_send_num"<<total_send_num<<endl;
            total_num--;
        }
        
    }
    
    cout<<"now total_recv_num:"<<total_recv_num<<",total_send_num"<<total_send_num<<endl;
    isRunning = false;
    for(int i = 0;i<threadNum;i++){
        if(th[i].joinable()){
            th[i].join();
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    
    m_tcp_server_->stop();
    delete m_tcp_server_;
    cout<<"finished"<<endl;
    
}