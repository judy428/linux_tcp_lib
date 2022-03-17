#ifndef _easyTcpClient_hpp_
#define _easyTcpClient_hpp_

#ifdef  _WIN32
	#define WIN_32_LEAN_AND_MEAN
	#define FD_SETSIZE 10240

	#include <WinSock2.h>
	#include <windows.h>
	#pragma comment(lib,"ws2_32.lib")

#else 
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <thread>


namespace base {

#ifndef RECV_BUFF_SIZE
#define RECV_BUF_SIZE 10240
#endif // !RECV_BUFF_SIZE
#ifndef MAX_BUF_SIZE  
#define MAX_BUF_SIZE (RECV_BUF_SIZE * 5)
#endif // !RECV_BUFF_SIZE

class easyTcpClient;
class tCliNotify
{
public:
    virtual void OnConnect(easyTcpClient* self) = 0;
	virtual void OnDisconnect(easyTcpClient* self) = 0;
	virtual size_t OnRecvData(easyTcpClient* self, const char* buf, const size_t len) = 0;
};

class easyTcpClient
{
public:
	easyTcpClient();
	void registerCallBack(tCliNotify* t);
	virtual ~easyTcpClient()
	{
        Stop();
		//Close();
	}

	//初始化socket
	void InitSocket();

	//连接服务器
	int Connect(const char *ip,unsigned short port);

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET && _isConnect;
	}

	//收数据 粘包 拆包
	int RecvData(SOCKET cliesock);

    // virtual uint32_t OnRecvMsg(dataRecv *dr)=0;

	//发送消息
	int SendData(const char *buf,int nlen);
    //启动
    void Start();

    //结束
    void Stop();
private:
    //查询网络消息
	bool OnRun();
    //关闭socket
	void Close();
    //响应
	// uint32_t OnNetMsg(dataHeader *header);
private:
    SOCKET _sock;
	// //接收缓冲区
	// char _szRecv[RECV_BUF_SIZE] = {};
	//第二缓冲区 ， 消息缓冲区
	char _szMsgBuf[MAX_BUF_SIZE] = {};
	//
	uint32_t m_ReadIndex = 0;
    uint32_t m_WriteIndex = 0;
    uint32_t m_DataSize = 0;
	tCliNotify* m_Tcn;
	bool _isConnect;
    std::thread* t1;
};

}
#endif