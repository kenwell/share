/**
 * @file twdef.h
 * @brief 定义几个系统共用的头文件
 * @author wugl
 */

#ifndef __TWDEF_H_INCLUDED__
#define __TWDEF_H_INCLUDED__

#include <sys/time.h>
#include <boost/cstdint.hpp>


#define MAXCOMMNUM 1000
#define SHMNAME    "control"

struct TimeRecord
{
    timeval clientSendTime;
    timeval serverRecvTime;
    timeval serverSendTime;
    timeval clientRecvTime;
};

struct MsgHeader
{
    int32_t no;
    int32_t groupNo;
    int32_t bodySize;
    TimeRecord time;
};

struct TwControlShm
{
    char allReady;
    int32_t commNum;
    char isReady[MAXCOMMNUM];
    char isFinish[MAXCOMMNUM];
    char allFinish;
    int32_t sendMsgs[MAXCOMMNUM];
};

#endif



