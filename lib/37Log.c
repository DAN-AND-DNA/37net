#define _GNU_SOURCE
#define _DEFAULT_SOURCE


#include <37Log.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

inline Log37* CreateLog(char* path, int cachesize)
{
    Log37* p = malloc(sizeof(Log37));
    if(p == NULL)
    {
        return NULL;
    }

    p->m_pstBuffer = malloc((uint32_t)(cachesize) * sizeof(char));
    if(p->m_pstBuffer == NULL)
    {
        free(p);
        return NULL;
    }

    p->m_pstFp = fopen(path, "a");
    if(p->m_pstFp == NULL)
    {
        goto ERROR;
    }
    
    if(setvbuf(p->m_pstFp, p->m_pstBuffer, _IOFBF, (uint32_t)(cachesize)) != 0)
    {
        fclose(p->m_pstFp);
        goto ERROR;
    }

    return p;

ERROR:
    free(p->m_pstBuffer);
    free(p);
    return NULL;
}


inline void CloseLog(Log37* pstLog)
{
    fclose(pstLog->m_pstFp);
    free(pstLog->m_pstBuffer);
    free(pstLog);
}
