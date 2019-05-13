#pragma once

inline int CreateServer(int port, char* bindaddr, int backlog);
inline int Accept(int fd, char* ip, int ip_len);
inline void Close(int fd);
