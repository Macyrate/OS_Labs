#ifndef SHAREBUFFER_H
#define SHAREBUFFER_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <strings.h>
#include <iostream>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <list>
#include <queue>
#include <memory>

#define SHM_PACK_LEN 4096
#define SHM_SIZE 30

#define SHM_PROJ_ID 0x6666
#define SEM_PROJ_ID 0x7777

enum SharedBufferType{
    TypeProducer,
    TypeConsumer
};

enum SharedBufferMode{
    ModeBlock,
    ModeUnblock
};

class SharedBuffer
{
public:
    explicit SharedBuffer(SharedBufferType type, SharedBufferMode mode = ModeBlock);
    virtual ~SharedBuffer();

    int put(const void *buf, const uint len);
    int get(void *buf, const uint len);

private:
    SharedBuffer(const SharedBuffer &);
    SharedBuffer & operator = (const SharedBuffer &);

private:
    typedef struct{
        unsigned int offset;
        unsigned int queueLen;
    }SharedBufferHeadSt;

private:
    int shmAttach();
    int shmDetach();
    int sem_p();
    int sem_v();

private:
    void *m_shmAddr;
    unsigned int m_rdOffset;
    unsigned int m_wrOffset;
    int m_shmId;
    int m_semId;
    key_t m_shmKey;
    key_t m_semKey;
    SharedBufferType m_shmType;
    SharedBufferMode m_shmMode;
};

typedef std::shared_ptr<SharedBuffer> SharedBufferPtr;

#define verbose(fmt, args...)\
{\
    printf("%s-%d: ", __FUNCTION__, __LINE__);\
    printf(fmt, ##args);\
}\

#endif