#include <iostream>
#include "tcpClient.h"
//#include "tcpMessageHeader.h"
#include "../trace.h"
#include "../base.h"
#include "../base_error.h"


using namespace std;
namespace base {
easyTcpClient::easyTcpClient()
{
	_sock = INVALID_SOCKET;
	_isConnect = false;
	m_ReadIndex = 0;
	m_WriteIndex = 0;
	//m_DataSize = sizeof(dataHeader);
	t1 = nullptr;
}

void easyTcpClient::registerCallBack(tCliNotify* t)
{
	m_Tcn = t;
}
	//初始化socket
void easyTcpClient::InitSocket()
{
	//启动环境
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif 		
	if (_sock != INVALID_SOCKET)
	{
		//printf("<socket = %d>关闭旧连接...\n",_sock);
		TRACE_INFO("base","<socket = %d>关闭旧连接...\n",_sock);
		Close();
	}

	_sock = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	if (_sock == INVALID_SOCKET)
	{
		TRACE_INFO("base","建立socket失败...");
	}
	else
	{
		TRACE_INFO("base","建立socket成功");
	}
}

	//连接服务器
int easyTcpClient::Connect(const char *ip,unsigned short port)
{
	if (_sock == INVALID_SOCKET)
	{
		InitSocket();
	}

	struct sockaddr_in _sin = {};

	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else 
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif 
	int ret = connect(_sock, (sockaddr *)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == ret)
	{
		TRACE_SYSTEM("base","连接服务器失败");
	}
	else
	{
		_isConnect = true;
		//printf("连接服务器成功...\n");
		m_Tcn->OnConnect(this);
	}
	return ret;
}

//关闭socket
void easyTcpClient::Close()
{
	if (_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_sock);
		WSACleanup();
#else
		close(_sock);
#endif 
		_sock = INVALID_SOCKET;
	}
	_isConnect = false;
	m_Tcn->OnDisconnect(this);
}

//查询网络消息
bool easyTcpClient::OnRun()
{
	while (isRun())
	{
		//printf("_sock:%d\n",_sock);
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);

		timeval t = { 1,0 };
		int ret = select(_sock + 1, &fdReads, NULL, NULL, &t);

		if (ret < 0)
		{
			TRACE_SYSTEM("base","<socket = %d>select任务结束", _sock);
			Close();
			return false;
		}

		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);

			if (-1 == RecvData(_sock))
			{
				TRACE_SYSTEM("base","select任务失败");
				Close();
				return false;
			}
		}
		//return true;
	}
	return false;
}


//收数据 粘包 拆包
int easyTcpClient::RecvData(SOCKET cliesock)
{
	int nlen = (int)recv(cliesock, _szMsgBuf + m_WriteIndex, MAX_BUF_SIZE - m_WriteIndex, 0);
	if (nlen <= 0)
	{
		TRACE_SYSTEM("base","服务器断开连接...");
		return -1;
	}
	//消息缓冲区的数据尾部位置后移
	m_WriteIndex += nlen;
	auto retsize = m_Tcn->OnRecvData(this,_szMsgBuf + m_ReadIndex,m_WriteIndex - m_ReadIndex);
	if (retsize <= 0){
		TRACE_ERROR(BASE_TAG,-1,"OnRecvData error,connection is closed,clientid:%d",cliesock);
		return -1;
	}else{
		m_ReadIndex += retsize;
	}
	
	if (m_ReadIndex >= MAX_BUF_SIZE/2){
		__mempcpy(_szMsgBuf,_szMsgBuf + m_ReadIndex,m_WriteIndex - m_ReadIndex);
		m_WriteIndex -= m_ReadIndex;
		m_ReadIndex = 0;
	}
	return 0;
}

// //响应
// uint32_t easyTcpClient::OnNetMsg(dataHeader *header)
// {
// 	dataRecv *dr = new dataRecv();
// 	dr->cmd = header->cmd;
// 	dr->data = new char[header->dataLength + 1];
// 	__mempcpy(dr->data,header,header->dataLength);
// 	auto ret = OnRecvMsg(dr);
// 	delete[] dr->data;
// 	delete dr;
// 	return ret;
// }

//发送消息
int easyTcpClient::SendData(const char *buf,int nlen)
{
	int ret = SOCKET_ERROR;
	if (isRun() && buf)
	{
		ret = send(_sock, (const char*)buf, nlen, 0);
		if (ret == SOCKET_ERROR)
		{
			TRACE_ERROR("base",NAUT_AT_E_TCP_CLIENT_SEND,"send to server error,ret:%d,errno:%d",ret,errno);
			Close();
		}
	}
	return ret;
}

void easyTcpClient::Start(){
	t1 = new std::thread(&easyTcpClient::OnRun, this);
	//t1->detach();
}

void easyTcpClient::Stop(){
	Close();
	if (t1 != nullptr){
		t1->join();
		delete t1;
		t1 = nullptr;
	}
}
}