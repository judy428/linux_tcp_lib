
#include <thread>
#include<chrono>
#include <string>
#include <stdlib.h>
#include <atomic>
#include "../../../out/include/base/tcp_client/tcpClient.h"
#include "../structInfo.h"
#include "../../src/tcp_struct.h"
#include "../../src/tcp_client_notify.h"
#include "../../../base/src/trace.h"

using namespace base;
using namespace common;
const int SendNum = 5000000;
std::atomic<int64_t> total_recv;
class tClientCallBack :public tcp_client_notify
{
public:
    virtual void handle_msg(atp_message& msg)
    {
        //total_recv.fetch_add(1);
        total_recv++;
        return;
        switch (msg.type)
        {
        case CMD_LOGIN_RESULT:
        {
            LoginResult *n = (LoginResult *)(msg.param1);
            TRACE_SYSTEM("test","收到服务端消息:CMD_LOGIN_RESULT,n->result:%d,n->data:%s\n",n->result,n->data);
        }
        break;
        case CMD_LOGOUT_RESULT:
        {
            LogoutResult *n = (LogoutResult *)(msg.param1);
            TRACE_SYSTEM("test","收到服务端消息:CMD_LOGOUT_RESULT,result:%d\n", n->result);

        }
        break;
        case CMD_NEWUSR_JOIN:
        {
            NewUsrJoin *n = (NewUsrJoin *)(msg.param1);
            TRACE_SYSTEM("test","收到服务端消息:新用户加入,sockid:%d\n", n->sockid);
        }
        break;

        default:
            TRACE_SYSTEM("test","收到未定义消息\n");
        break;
        }
        return ;
    }
    
};
bool g_bRun = true;

//客户端连接数量
const int cCount = 200;
//发送线程数量
const int tCount = 4;
//客户端数组
easyTcpClient* client[cCount];

std::atomic<int64_t> m_total_send;

void sendThread()
{
	for (int i = 0; i < cCount; i++)
	{
		client[i] = new easyTcpClient();
        tClientCallBack* tc = new tClientCallBack();
        client[i]->registerCallBack(tc);
	}
    
	for (int i = 0; i < cCount; i++)
	{
		client[i]->InitSocket();
		client[i]->Connect("127.0.0.1", 3319);
        client[i]->Start();
        
	}
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    Login login;
    strcpy(login.usrName, "lsm");
	strcpy(login.passWd, "154029");
    common::msg_header m_header;
    m_header.type = CMD_LOGIN;
    m_header.data_size = sizeof(login);

    char* ptr = new char[sizeof(m_header) +  sizeof(login) + 1];
    
    printf("size:%ld\n",sizeof(login));

    //std::this_thread::sleep_for(std::chrono::seconds(1));
    auto beginTime = std::chrono::high_resolution_clock::now();
    for(int i = 0;i<SendNum;i++){
        strcpy(login.passWd, std::to_string(i).c_str());

        memset(ptr,0,sizeof(m_header) +  sizeof(login) + 1);
        memcpy(ptr,&m_header,sizeof(m_header));
	    memcpy(ptr + sizeof(m_header),&login,sizeof(login));

        for (int i = 0; i < cCount; i++)
        {
            auto ret = client[i]->SendData(ptr,sizeof(common::msg_header) +  sizeof(login) );
            if(ret == SOCKET_ERROR){
                TRACE_SYSTEM("test","senddata error,ret:%d",ret);
                //exit(1);
            }
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            // ret = client[i]->SendData(ptr,sizeof(Login));
            // if(ret == SOCKET_ERROR){
            //     printf("senddata error,ret:%d",ret);
            // }
            
            //TRACE_SYSTEM("test","send ptr value:%s\n",ptr + sizeof(common::msg_header));
        }
    }
	auto endTime = std::chrono::high_resolution_clock::now();
	auto elapsedTime= std::chrono::duration_cast<std::chrono::seconds>(endTime - beginTime);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    TRACE_SYSTEM("test","cast time:%d",elapsedTime.count());
    
	//const int nlen = sizeof(login);
	while(total_recv.load() < SendNum * cCount){
        std::this_thread::sleep_for(std::chrono::seconds(2));
        TRACE_SYSTEM("test","total_recv:%d",total_recv.load());
    }
    
    for (int i = 0; i < cCount; i++)
	{
		client[i]->Stop();
    }
    g_bRun = false;
    
    TRACE_SYSTEM("test","total_recv:%d",total_recv.load());
}

int sendData(std::shared_ptr<easyTcpClient> etc)
{
    Login login;
    strcpy(login.usrName, "lsm");
	strcpy(login.passWd, "154029");
    common::msg_header m_header;
    m_header.type = CMD_LOGIN;
    m_header.data_size = sizeof(login);

    char* ptr = new char[sizeof(m_header) +  sizeof(login) + 1];

    for(int i = 0;i<SendNum;i++){
        strcpy(login.passWd, std::to_string(i).c_str());

        memset(ptr,0,sizeof(m_header) +  sizeof(login) + 1);
        memcpy(ptr,&m_header,sizeof(m_header));
	    memcpy(ptr + sizeof(m_header),&login,sizeof(login));
        auto ret = etc->SendData(ptr,sizeof(common::msg_header) +  sizeof(login) );
        if(ret == SOCKET_ERROR){
            TRACE_SYSTEM("test","senddata error,ret:%d",ret);
            //exit(1);
        }
    }
    m_total_send += SendNum;
}

int stress_test()
{
    std::shared_ptr<easyTcpClient> cs[cCount];
    for(int i = 0;i<cCount;i++){
        cs[i] = std::make_shared<easyTcpClient>();
        tClientCallBack* tc = new tClientCallBack();
        cs[i]->registerCallBack(tc);

        cs[i]->InitSocket();
		cs[i]->Connect("127.0.0.1", 3319);
        cs[i]->Start();
    }
    auto startTime = std::chrono::system_clock::now();
    std::thread* th[cCount];
    for (int i = 0; i < cCount; i++)
	{
        th[i] = new std::thread(sendData,cs[i]);
    }
    
    while(total_recv.load() < SendNum * cCount){
        if( m_total_send.load() >= SendNum * cCount){
           auto sendEndTime = std::chrono::system_clock::now(); 
           TRACE_INFO("test","send cast time,seconds:%d",std::chrono::duration_cast<std::chrono::seconds>(sendEndTime - startTime ));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        //TRACE_SYSTEM("test","total_recv:%d",total_recv.load());
    }
    auto endTime = std::chrono::system_clock::now();
    auto castTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime );
    auto castTime2 = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime );
    auto s = total_recv.load() / castTime.count() * 2;
    auto s2 = total_recv.load() / castTime2.count() * 2;
    TRACE_SYSTEM("test","cast time,seconds:%d,speed:%d,cast time,milliseconds:%d,speed2:%d",castTime,s ,castTime2,s2);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    for (int i = 0; i < cCount; i++)
	{
		cs[i]->Stop();
    }
    for(int i = 0;i < cCount;i++)
    {
        th[i]->join();
    }
    TRACE_SYSTEM("test","total_recv:%d",total_recv.load());
}

int main()
{
    total_recv = 0;
    m_total_send = 0;
    trace::set_trace_level_filter(TRACE_LEVEL_SYSTEM);
#if 0
    sendThread();
	while (g_bRun){
        //Sleep(100);
        //this_thread::sleep_for(chrono::seconds(5));//sleep 5秒
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
#else
    for(int i = 0;i<20;i++){
        total_recv = 0;
        m_total_send = 0;
        stress_test();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    
#endif
    
	printf("已退出");
	return 0;
}