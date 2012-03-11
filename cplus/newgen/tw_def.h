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

struct ResultTime
{
   uint64_t minsvrProcTime;
   uint64_t minrttTime;
   uint64_t maxsvrProcTime;
   uint64_t maxrttTime;
   uint64_t avgsvrProcTime;
   uint64_t avgrttTime;
   uint64_t quasvrProcTime;
   uint64_t quarttTime;
   uint64_t pquasvrProcTime;
   uint64_t pquarttTime;
};

struct TwControlShm
{
    char allReady;
    int32_t commNum;
    char isReady[MAXCOMMNUM];
    char isFinish[MAXCOMMNUM];
    char allFinish;
    char isReport[MAXCOMMNUM];
    int32_t sendMsgs[MAXCOMMNUM];
    ResultTime resultTimes[MAXCOMMNUM];
};

#endif



