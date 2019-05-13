#include <37EventLoop.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef struct Epoll37 {
    int                     m_iEfd;
    struct epoll_event*     m_EpollEvents;
} Epoll37;

int iDone = 0;

inline EventLoop37* CreateEventLoop(int iSize)
{
    EventLoop37* pstEventLoop = NULL;
    if ((pstEventLoop = malloc(sizeof(EventLoop37))) == NULL)
    {
        goto ERROR;
    }

    pstEventLoop->m_iSize = iSize;
    pstEventLoop->m_pstEvents = malloc(sizeof(Event37) * (uint32_t)(iSize));
    pstEventLoop->m_pstFiredEvents = malloc(sizeof(FiredEvent37) * (uint32_t)(iSize));

    if(pstEventLoop->m_pstEvents == NULL || pstEventLoop->m_pstFiredEvents == NULL)
    {
        goto ERROR;
    }

    Epoll37* pstEpoll = malloc(sizeof(Epoll37));
    if(pstEpoll == NULL)
    {
        goto ERROR;
    }

    pstEpoll->m_EpollEvents = malloc(sizeof(struct epoll_event) * (uint32_t)(iSize));
    if(pstEpoll->m_EpollEvents == NULL)
    {
        goto ERROR;
    }

    pstEpoll->m_iEfd = epoll_create1(EPOLL_CLOEXEC);
    if(pstEpoll->m_iEfd == -1)
    {
        goto ERROR;
    }
    
    int i;
    for(i = 0; i < pstEventLoop->m_iSize; i++)
    {
        pstEventLoop->m_pstEvents[i].m_iAction = 0;
    }

    pstEventLoop->m_pstEpoll = pstEpoll;
    return pstEventLoop;

ERROR:
    if(pstEpoll != NULL)
    {
        free(pstEpoll->m_EpollEvents);
        free(pstEpoll);
    }

    if(pstEventLoop != NULL)
    {
        free(pstEventLoop->m_pstEvents);
        free(pstEventLoop->m_pstFiredEvents);
        free(pstEventLoop);
    }

    return NULL;

}

inline int AddEvent(EventLoop37* pstEventLoop, int iFd, int iAction, Handle* pstHandle, void* pstUserData)
{
    if(iFd > pstEventLoop->m_iSize)
    {
        return -1;
    }

    Event37* e = &(pstEventLoop->m_pstEvents[iFd]);
    Epoll37* ep = pstEventLoop->m_pstEpoll;
    
    struct epoll_event ee;
    ee.events = 0;
    
    int op = EPOLL_CTL_ADD;
    if(e->m_iAction != 0)
    {
        op = EPOLL_CTL_MOD;
    }

    e->m_iAction |= iAction;
    e->m_pstUserData = pstUserData;
    
    if(iAction & EPOLLIN)
    {
        e->m_pstHandleRead = pstHandle;
    }

    if(iAction & EPOLLOUT)
    {
        e->m_pstHandleWrite = pstHandle;
    }
    ee.events = (uint32_t)(e->m_iAction);
    ee.data.fd = iFd;
    if(epoll_ctl(ep->m_iEfd, op, iFd, &ee) == -1) 
    {
        return -1;
    }

    return 0;
}

inline int DelEvent(EventLoop37* pstEventLoop, int iFd, int iAction)
{
    if(iFd > pstEventLoop->m_iSize)
    {
        return -1;
    }

    Event37* e = &(pstEventLoop->m_pstEvents[iFd]);
    Epoll37* ep = pstEventLoop->m_pstEpoll;

    struct epoll_event ee;
    ee.events = 0;
    
    if(e->m_iAction == 0)
    {
        return 0;
    }
    e->m_iAction &= ~iAction;
    ee.events = (uint32_t)(e->m_iAction);
    ee.data.fd = iFd;
    if(e->m_iAction == 0)
    {
        if(epoll_ctl(ep->m_iEfd, EPOLL_CTL_DEL, iFd, &ee) == -1)
        {
            return -1;
        }
    }
    else
    {
        if(epoll_ctl(ep->m_iEfd, EPOLL_CTL_MOD, iFd, &ee) == -1)
        {
            return -1;
        }

    }
    return 0;
}


inline int Poll(EventLoop37* pstEventLoop)
{
    Epoll37* ep = pstEventLoop->m_pstEpoll;
    int fired = epoll_wait(ep->m_iEfd, ep->m_EpollEvents, pstEventLoop->m_iSize, 10000);

    int i;
    for(i = 0; i < fired; i++)
    {
        struct epoll_event* e = ep->m_EpollEvents + i; 
        if(e->events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {
            pstEventLoop->m_pstFiredEvents[i].m_iFd  = e->data.fd;
            pstEventLoop->m_pstFiredEvents[i].m_iAction = EPOLLIN;
        }

        if(e->events & EPOLLOUT)
        {    
            pstEventLoop->m_pstFiredEvents[i].m_iFd  = e->data.fd;
            pstEventLoop->m_pstFiredEvents[i].m_iAction = EPOLLOUT;
        }
    }
    return fired;
}


inline int ProcessFired(EventLoop37* pstEventLoop)
{
    int fired = Poll(pstEventLoop);

    int i;
    for(i = 0; i < fired; i++)
    {
        int fd = pstEventLoop->m_pstFiredEvents[i].m_iFd;
        int iAction = pstEventLoop->m_pstFiredEvents[i].m_iAction;
        Event37* e = &(pstEventLoop->m_pstEvents[fd]);
        if(iAction & EPOLLIN)
        {
            e->m_pstHandleRead(pstEventLoop, fd, e->m_pstUserData, iAction);
        }
        if(iAction & EPOLLOUT)
        {
            e->m_pstHandleWrite(pstEventLoop, fd, e->m_pstUserData, iAction);
        }

        iDone++;
    }
       
    return iDone;
}


inline void Run(EventLoop37* pstEventLoop)
{
    while(!pstEventLoop->m_iStop)
    {
        ProcessFired(pstEventLoop);
    }
}

inline void Stop(EventLoop37* pstEventLoop)
{
    Epoll37* ep = pstEventLoop->m_pstEpoll;
    close(ep->m_iEfd);
    free(ep->m_EpollEvents);
    free(pstEventLoop->m_pstEpoll);
    free(pstEventLoop->m_pstEvents);
    free(pstEventLoop->m_pstFiredEvents);
    free(pstEventLoop);
}
