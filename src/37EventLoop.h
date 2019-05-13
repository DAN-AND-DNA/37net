#pragma once

typedef struct FiredEvent37 {
    int m_iFd; 
    int m_iAction;

} FiredEvent37;


typedef struct Event37 Event37;

typedef struct EventLoop37 {
    Event37*        m_pstEvents;
    FiredEvent37*   m_pstFiredEvents;
    void*           m_pstEpoll;
    int             m_iSize;
    int             m_iStop;
} EventLoop37;

typedef void Handle(EventLoop37* pstEventLoop, int iFd, void* UserData, int iAction);

typedef struct Event37 {
    int     m_iAction;
    void*   m_pstUserData;
    Handle* m_pstHandleRead;
    Handle* m_pstHandleWrite;
} Event37;


    
inline EventLoop37* CreateEventLoop(int iSize);
inline void Run(EventLoop37* pstEventLoop);
inline void Stop(EventLoop37* pstEventLoop);
inline int AddEvent(EventLoop37* pstEventLoop, int iFd, int iAction, Handle* pstHandle, void* pstUserData);
inline int DelEvent(EventLoop37* pstEventLoop, int iFd, int iAction);
