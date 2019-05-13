#include <37EventLoop.h>
#include <37Socket.h>
#include <37Log.h>
#include <stddef.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/time.h>

//Log37* pstLog;
EventLoop37* pstEventLoop;
uint8_t buffer[1];
uint64_t count;

void CloseHandle(int i)
{
    printf("get ctrl+c server is close\n");
    if(pstEventLoop != NULL)
    {
        pstEventLoop->m_iStop = 1;
    }
}


void WriteToClient(EventLoop37* pstEventLoop, int iFd, void* UserData, int iAction)
{
    ssize_t r = write(iFd, buffer, 1);
    if(r <= 0)
    {
        DelEvent(pstEventLoop, iFd, EPOLLIN | EPOLLOUT);
        Close(iFd);
        return;
    }

}

void ReadFromClient(EventLoop37* pstEventLoop, int iFd, void* UserData, int iAction)
{
    if(read(iFd, buffer, 1) <= 0)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        printf("%ld client offline\n", tv.tv_sec);
        DelEvent(pstEventLoop, iFd, EPOLLIN | EPOLLOUT);
        Close(iFd);
        return;
    }

    /*
    ssize_t r = write(iFd, buffer, 1);


    if(r == 0)
    {
        if(AddEvent(pstEventLoop, iFd, EPOLLOUT, WriteToClient, buffer) != 0)
        {
            DelEvent(pstEventLoop, iFd, EPOLLIN | EPOLLOUT);
            Close(iFd);
            return;
        }
    }
    else if(r < 0)
    {
        DelEvent(pstEventLoop, iFd, EPOLLIN | EPOLLOUT);
        Close(iFd);
        return;
    }
    */

    count++;

}

void AcceptHandle(EventLoop37* pstEventLoop, int iFd, void* UserData, int iAction)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int iClientFd;
    char szClientIp[128];
    iClientFd = Accept(iFd, szClientIp, 128);
    if(iClientFd != -1)
    {
        printf("%ld client online :%s\n", tv.tv_sec, szClientIp);
        if(AddEvent(pstEventLoop, iClientFd, EPOLLIN, ReadFromClient, NULL) != 0)
        {
            Close(iClientFd);
        }
    }
}


int main()
{
    count = 0;
    int sfd = CreateServer(7737, "0.0.0.0", 128);
    pstEventLoop = CreateEventLoop(10000);
    //pstLog = CreateLog("./debug.log", 4096);

    if(sfd != -1 && pstEventLoop != NULL)
    {
        signal(SIGPIPE, SIG_IGN);
        signal(SIGINT, CloseHandle);
        if(AddEvent(pstEventLoop, sfd, EPOLLIN, AcceptHandle, NULL) == 0)
        {
            Run(pstEventLoop); 
        }
 
      //  CloseLog(pstLog);
        Close(sfd);
        Stop(pstEventLoop);
    }
    printf("count :%lu\n",  count);
    return 0;
}
