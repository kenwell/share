/**
 * @file twdef.h
 * @brief 定义几个系统共用的头文件
 * @author wugl
 */

#ifndef __TWDEF_H_INCLUDED__
#define __TWDEF_H_INCLUDED__

#include <sys/time.h>
#include <boost/cstdint.hpp>

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


#endif



