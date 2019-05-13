#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <37Socket.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>

inline int CreateServer(int port, char* bindaddr, int backlog)
{
    struct addrinfo stHints;
    struct addrinfo *pstServerInfo;
    struct addrinfo *pstCurrInfo;
    int iServerFd = -1;
    int iOptVal = 1;
    char szPort[6] = {0};
    bzero(&stHints, sizeof(struct addrinfo));
    stHints.ai_family = AF_INET;
    stHints.ai_socktype = SOCK_STREAM;
    stHints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV;
    snprintf(szPort, 6, "%d", port);

    if(getaddrinfo(bindaddr, szPort, &stHints, &pstServerInfo) < 0)
    {
        return -1;
    }
   

    for(pstCurrInfo = pstServerInfo; pstCurrInfo != NULL; pstCurrInfo = pstServerInfo->ai_next)
    {
        iServerFd = socket(pstCurrInfo->ai_family, pstCurrInfo->ai_socktype, pstCurrInfo->ai_protocol);
        if(iServerFd == -1 || 
           setsockopt(iServerFd, SOL_SOCKET, SO_REUSEADDR, &iOptVal, sizeof(iOptVal)) == -1 ||
           setsockopt(iServerFd, IPPROTO_TCP, TCP_NODELAY, &iOptVal, sizeof(iOptVal)) == -1 ||
           bind(iServerFd, pstServerInfo->ai_addr, pstServerInfo->ai_addrlen) == -1 ||
           listen(iServerFd, backlog) == -1)
        {
            if(iServerFd != -1)
            {
                close(iServerFd);
            }
            iServerFd = -1;
        }
        continue;
    }

    freeaddrinfo(pstServerInfo);
    return iServerFd;
}

inline int Accept(int fd, char* ip, int ip_len)
{
    if(fd == -1)
    {
        return -1;
    }

    int iClientFd = -1;
    struct sockaddr_in stClientAddr;
    socklen_t socklen = sizeof(struct sockaddr_in);

    while(1)
    {
        iClientFd = accept4(fd, (struct sockaddr*)(&stClientAddr), &socklen, SOCK_CLOEXEC | SOCK_NONBLOCK);
        if(iClientFd == -1)
        {
           if(errno == EINTR)
           {
               continue;
           }
           else
           {
               break;
           }
        }
        else
        {
            if(ip) inet_ntop(AF_INET, (void*)&(stClientAddr.sin_addr), ip, (socklen_t)(ip_len));
            break;
        }
    }

    return iClientFd;
}

inline void Close(int fd)
{
    if(fd != -1)
    {
        close(fd);
    }
}
