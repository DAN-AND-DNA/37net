#pragma once

#include <stdio.h>

typedef struct Log37 {
    FILE*   m_pstFp; 
    char*   m_pstBuffer;
} Log37;

inline Log37* CreateLog(char* path, int cachesize);
inline void CloseLog(Log37* pstLog);
