/*****************************************************************************
 Base Copyright (c) 2021. All Rights Reserved.

 FileName: cell_manager.cpp
 Version: 1.0
 Date: 2021.5.8

 History:
 jasonzhu     2021.5.8   1.0     Create
 ******************************************************************************/

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include "cell_manager.h"
#include "../thread.h"
#include "../trace.h"
#include "../base_error.h"
#include "../tools.h"
namespace base{

cellManager::cellManager(tNotify* tN,int cpuN){
    m_ListenFd = epoll_create1(EPOLL_CLOEXEC);
    m_TNotify = tN;
    m_BindCpu = cpuN;
    m_cellTask = new srt_queue<cellTask*>();
    m_cellTask->init();

    // m_handleRead = new srt_queue<int>();
    // m_handleRead->init();

    // m_th = make_shared<std::thread>(bind(&cellManager::handleRead,this));
}

cellManager::~cellManager(){
    stop();
    // if(m_th->joinable())
    // {
    //     m_th->join();
    // }
    for(auto &it : m_Cell){
        //delete it.second;
        delClientCell(it.second);
        it.second = nullptr;
    }
    m_Cell.clear();

    TRACE_ERROR("test",-1,"m_cellTask->size():%d",m_cellTask->size());
    cellTask* cT = nullptr;
    while(m_cellTask->pop(cT)){
        if (cT->event == 'S'){
            delete [](cT->sC->ptr);
            cT->sC->ptr = nullptr;
            delete cT->sC;
            cT->sC = nullptr;
        }
        delete cT;
        cT = nullptr;
    }
}


// void cellManager::handleRead()
// {
//     int clientid;
//     while(is_running()){
//         if(m_handleRead->pop(clientid)){
//              if (m_Cell.find(clientid) == m_Cell.end())
//             {
//                 TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_CLIENT_CLOSED,"client has been closed,clientid:%d",clientid);
//                 //delTask(clientid);
//                 continue;
//             }

//         }
//     }
// }

void cellManager::addTask(clientCell* cell){
    cellTask* ct = new cellTask();
    ct->event = 'A';
    ct->clientid = cell->m_ClientFd;
    m_Cell[cell->m_ClientFd] = cell;
    m_cellTask->push(ct);
}

void cellManager::delTask(int clientid){
    TRACE_SYSTEM(BASE_TAG,"client will be deleted,clientid:%d",clientid);
    cellTask* ct = new cellTask();
    ct->event = 'D';
    ct->clientid = clientid;
    m_cellTask->push(ct);
}

int cellManager::directSend(int clientid,char* ptr,int size)
{
    return write(clientid,ptr,size);
}

int64_t cellManager::sendTask(int clientid,char* p,int len){
    char* ptr = p;
    int size = len;
    //先检查是否有数据未发送，没有的话，直接发送
    if (m_Cell.find(clientid) == m_Cell.end())
    {
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_CLIENT_CLOSED,"client has been closed,clientid:%d",clientid);
        //delTask(clientid);
        return NAUT_AT_E_TCP_SERVER_CLIENT_CLOSED;
    }

    //TRACE_SYSTEM(BASE_TAG,"clientid:%d,szie:%d,len:%d",clientid,size,len);
    cellTask* ct = new cellTask();
    ct->event = 'S';
    ct->clientid = clientid;
    ct->sC = new clientCell::sendCell();
    ct->sC->ptr = new char[size + 1];
    memset(ct->sC->ptr,0,size + 1);
    memcpy(ct->sC->ptr,ptr,size);
    ct->sC->size = size;
    ct->sC->sendPos = 0;
    auto flag = m_cellTask->push(ct);
    if (!flag){
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_PUSH_CACHE_FAILED,"insert error");
        return NAUT_AT_E_TCP_SERVER_PUSH_CACHE_FAILED;
    }
    //TRACE_SYSTEM(BASE_TAG,"clientid:%d,szie:%d,len:%d",clientid,size,len);
    return NAUT_AT_S_OK;
}

void cellManager::handleAdd(cellTask* ct){
    if (m_Cell.find(ct->clientid) == m_Cell.end())
    {
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_CLIENT_CLOSED,"client has been closed,clientid:%d",ct->clientid);
        delTask(ct->clientid);
        return;
    }

    clientCell* cell = m_Cell[ct->clientid];
    cell->m_Events = ( EPOLLET | EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLERR);
    //TRACE_SYSTEM(BASE_TAG,"handleAdd,clientid:%d,cell->m_Events:%ud",cell->m_ClientFd,cell->m_Events);
    //监听
    epoll_event event;
    memset(&event.data, 0, sizeof(event.data));
    event.data.fd = cell->m_ClientFd;
    event.events = cell->m_Events; 
    epoll_ctl(m_ListenFd, EPOLL_CTL_ADD,cell->m_ClientFd,&event);
    m_TNotify->OnConnect(cell->m_ClientFd,cell->m_Ip,cell->m_Port);
}

void cellManager::handleSend(cellTask* ct){
     if (m_Cell.find(ct->clientid) == m_Cell.end())
    {
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_CLIENT_CLOSED,"client has been closed,clientid:%d",ct->clientid);
        //delTask(ct->clientid);
        return;
    }
    
    clientCell* cell = m_Cell[ct->clientid];
    //判断是否满
    if ((cell->m_SendWritePos +1) % tcpCommon::SYSSENDBUF == cell->m_SendReadPos){
        TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_SEND_CACHE_FULL,"send cache is full,connection is closed,cliendid:%d,m_SendWritePos:%d,m_SendReadPos:%d",
            ct->clientid,cell->m_SendWritePos,cell->m_SendReadPos);
        return;
        //是否有必要删除连接，此时暂时先不删除，发送数据到客户端。
        delete [](ct->sC->ptr);
        ct->sC->ptr = nullptr;
        delete ct->sC;
        ct->sC = nullptr;
        m_TNotify->OnError(cell->m_ClientFd,-3);
        delTask(cell->m_ClientFd);
    }
    cell->m_SendBuf[cell->m_SendWritePos] = ct->sC;
    ct->sC = nullptr;
    cell->m_SendWritePos++;
    cell->m_SendWritePos = cell->m_SendWritePos%tcpCommon::SYSSENDBUF;
    //TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_SEND_CACHE_FULL,"send cache,ct->clientid:%",ct->clientid);
}

void cellManager::delClientCell(clientCell* cell){
     //
    epoll_event event;
    event.data.fd = cell->m_ClientFd;
    event.events = 0; 
    epoll_ctl(m_ListenFd, EPOLL_CTL_DEL,cell->m_ClientFd,&event);
    //
    close(cell->m_ClientFd);
    delete cell;
}

void cellManager::handleDel(cellTask* ct){
    
    if (m_Cell.find(ct->clientid) == m_Cell.end()){
        return;
    }
    //TRACE_SYSTEM(BASE_TAG,"client delete,begin,clientid:%d",ct->clientid);
    clientCell* cell = m_Cell[ct->clientid];
    delClientCell(cell);
    m_Cell.erase(ct->clientid);
    m_TNotify->OnDisconnect(ct->clientid);
    //TRACE_SYSTEM(BASE_TAG,"client delete,end,clientid:%d",ct->clientid);
}

void cellManager::handleTask(){
    cellTask* cT = nullptr;
    //TRACE_SYSTEM(BASE_TAG,"m_cellTask->size():%d",m_cellTask->size());
    while(m_cellTask->pop(cT)){
        switch (cT->event)
        {
        case 'S':
            /* code */
            handleSend(cT);
            break;
        case 'D':
            /* code */
            handleDel(cT);
            break;
        case 'A':
            /* code */
            handleAdd(cT);
            break;
        default:
            break;
        }
        delete cT;
       cT = nullptr;
    }
} 

void cellManager::checkWrite()
{
    //TRACE_SYSTEM(BASE_TAG,"m_Cell.size():%d",m_Cell.size());
    for(auto &it : m_Cell){
        clientCell* cell = it.second;
        if(cell != nullptr){
            if(cell->m_SendWritePos != cell->m_SendReadPos){
                //加入写监控
                cell->m_Events |= EPOLLOUT;
                epoll_event event;
                memset(&event.data, 0, sizeof(event.data));
                event.data.fd = cell->m_ClientFd;
                event.events = cell->m_Events; 
                epoll_ctl(m_ListenFd, EPOLL_CTL_MOD,cell->m_ClientFd,&event);
                //TRACE_SYSTEM(BASE_TAG,"insert cell->m_ClientFd:%d",cell->m_ClientFd);
            }
            else{
                //清除写监控 
                //TRACE_SYSTEM(BASE_TAG,"cell->m_ClientFd:%d",cell->m_ClientFd);
                if (cell->m_Events & EPOLLOUT){
                    cell->m_Events &= ~EPOLLOUT;
                    epoll_event event;
                    memset(&event.data, 0, sizeof(event.data));
                    event.data.fd = cell->m_ClientFd;
                    event.events = cell->m_Events; 
                    epoll_ctl(m_ListenFd, EPOLL_CTL_MOD,cell->m_ClientFd,&event);
                }
            }
        }else{
            TRACE_SYSTEM(BASE_TAG,"client has disconnected,clientid:%d",it.first);
        }
    }
}

void cellManager::start(){
    if(!is_running()){
        process_thread::start();
    }
}

void cellManager::stop(){
    if(is_running()){
        process_thread::stop();
    }
    close(m_ListenFd);
}

void cellManager::run(){
    epoll_event events[1024];
    while(is_running()){
        //auto ret = epoll_wait(m_ListenFd,events,1024,1);
        //usleep(1);
        handleTask();
        checkWrite();
        //usleep(1);
        auto ret = epoll_wait(m_ListenFd,events,1024,1);
         if (ret < 0){
            TRACE_ERROR(BASE_TAG, 0, "cellManager epoll_wait error,retcode:%d ,errno:%d",ret,errno);
            continue;
        }
        
        //TRACE_ERROR(BASE_TAG, 0, "ret:%d",ret);
        for(auto i = 0;i< ret;i++){
            doEvent(&events[i]);
        }
    }
    
}

void cellManager::doEvent(epoll_event* event){
    //TRACE_SYSTEM(BASE_TAG,"event->events:%d",event->events);
    if (event->events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)){
        TRACE_ERROR(BASE_TAG,0,"doEvent:%d",event->events);
        delTask(event->data.fd);
        return;
    }
    if(event->events & EPOLLIN ){
        doRecv(event->data.fd);
    }
    if (event->events & EPOLLOUT){
        doSend(event->data.fd);
    }
}

void cellManager::doRecv(int clientid){
    if (m_Cell.find(clientid) == m_Cell.end()){
        TRACE_SYSTEM(BASE_TAG,"client has been closed,clientid:%d",clientid);
        return;
    }
    int err = 0;
    clientCell* cell = m_Cell[clientid];
    //
    while(cell->m_RecvWritePos < tcpCommon::SYSRECVBUF){
        bool breakflag = false;
        auto bytes_read = read(cell->m_ClientFd,(void *)(cell->m_RecvBuf + cell->m_RecvWritePos),tcpCommon::SYSRECVBUF - cell->m_RecvWritePos );
        //TRACE_SYSTEM(BASE_TAG,"doRecv,clientid:%d",cell->m_ClientFd);
        if(bytes_read < 0){
            if(errno == EINTR || errno == EAGAIN){
                bytes_read = 0;
                continue;
            }else{
                //读取失败
                TRACE_ERROR(BASE_TAG, 0, "cellManager read error,clientid:%d,retcode:%d ,errno:%d",cell->m_ClientFd,bytes_read,errno);
                err = -1;
                break;
            }
        }else if (bytes_read == 0){
            TRACE_ERROR(BASE_TAG, 0, "cellManager read() return 0,client has closed,clientid:%d",cell->m_ClientFd );
            err = -1;
            break;
        }else{
            if(bytes_read < tcpCommon::SYSRECVBUF - cell->m_RecvWritePos){
                //数据已经读完，跳出循环
                breakflag = true;
            }
            cell->m_RecvWritePos += bytes_read;
        }
        //std::cout<<"recv msg:"<<cell->m_RecvBuf + cell->m_RecvReadPos<<std::endl;
        //返回给服务器端
        auto retSize = m_TNotify->OnRecvData(cell->m_ClientFd,cell->m_RecvBuf + cell->m_RecvReadPos,cell->m_RecvWritePos - cell->m_RecvReadPos);
        cell->m_RecvReadPos += retSize;
        // cell->m_RecvReadPos = cell->m_RecvWritePos;
        if(cell->m_RecvReadPos >= cell->m_RecvWritePos){
            cell->m_RecvReadPos = 0;
            cell->m_RecvWritePos = 0;
        }else if (cell->m_RecvReadPos >= tcpCommon::SYSRECVBUF / 2){
            memcpy(cell->m_RecvBuf,cell->m_RecvBuf + cell->m_RecvReadPos,cell->m_RecvWritePos - cell->m_RecvReadPos);
            cell->m_RecvWritePos -= cell->m_RecvReadPos;
            cell->m_RecvReadPos = 0;
        }

        if(cell->m_RecvWritePos >= tcpCommon::SYSRECVBUF){
            //缓冲区满
            err = -2;
            m_TNotify->OnError(cell->m_ClientFd,err);
            break;
        }
        if(breakflag){
            break;
        }
    }

    switch (err)
    {
    case 0:
        break;
    case -1:
        delTask(cell->m_ClientFd);
        break;
    default:
        break;
    }
}


void cellManager::doSend(int clientid){
    //TRACE_SYSTEM(BASE_TAG,"dosend ,clientid:%d",clientid);
    int flag = 0;
    // if(clientid == 36 || clientid == 37){
    //     TRACE_ERROR(BASE_TAG,-1,"dosend ,clientid:%d",clientid);
    // }
    //return;
    if (m_Cell.find(clientid) == m_Cell.end()){
        TRACE_ERROR(BASE_TAG,0,"client has been closed,clientid:%d",clientid);
        return;
    }
    clientCell* cell = m_Cell[clientid];
    //为了确保本链接缓冲区发送过多对其他链接的发送操作造成影响，设定最大发送次数,如果之后c++有协程，可以改成全量发送
    for(auto i = 0;i<tcpCommon::serverMaxSendTimes;i++){
        if(cell->m_SendWritePos != cell->m_SendReadPos){
        //TRACE_SYSTEM(BASE_TAG,"writepos:%d,sendpos:%d",cell->m_SendWritePos,cell->m_SendReadPos);
        auto retsize = write(cell->m_ClientFd,cell->m_SendBuf[cell->m_SendReadPos]->ptr + cell->m_SendBuf[cell->m_SendReadPos]->sendPos,
            cell->m_SendBuf[cell->m_SendReadPos]->size - cell->m_SendBuf[cell->m_SendReadPos]->sendPos);
            
            if(retsize < 0){
                if (errno == EINTR || errno == EAGAIN){
                    TRACE_SYSTEM(BASE_TAG,"doSend failed,will try again,retsize:%d,cliendid:%d,errno:%d",retsize,clientid,errno);
                    continue;
                }else{
                    TRACE_ERROR(BASE_TAG,NAUT_AT_E_TCP_SERVER_CLIENT_CLOSED,"doSend error,retsize:%d,cliendid:%d,errno:%d",retsize,clientid,errno);
                    break;
                }
            }else if (retsize == 0){
                //发送缓冲区已满
                TRACE_SYSTEM(BASE_TAG,"tcp send cache is full,will try later,cliendid:%d",clientid);
                break;
            }else{
                
                if (retsize < cell->m_SendBuf[cell->m_SendReadPos]->size - cell->m_SendBuf[cell->m_SendReadPos]->sendPos){
                    cell->m_SendBuf[cell->m_SendReadPos]->sendPos += retsize;
                    break;
                }else{
                    //释放内存
                    //TRACE_SYSTEM(BASE_TAG,"delete cache,clientid:%d,pos:%d",clientid,cell->m_SendReadPos);
                    delete[] (cell->m_SendBuf[cell->m_SendReadPos]->ptr);
                    cell->m_SendBuf[cell->m_SendReadPos]->ptr = nullptr;
                    delete cell->m_SendBuf[cell->m_SendReadPos];
                    cell->m_SendBuf[cell->m_SendReadPos] = nullptr;
                    cell->m_SendReadPos++;
                    cell->m_SendReadPos = cell->m_SendReadPos%tcpCommon::SYSSENDBUF;
                }
            }
        }
    }
}

}