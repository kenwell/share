
#include "tw_comm_tnovr.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/p_square_quantile.hpp>
#include <boost/accumulators/statistics/tail_quantile.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>

using namespace boost::asio;

using std::fill;
using std::string;
using std::cout;
using std::endl;
using std::stringstream;
using boost::array;
using boost::format;
using boost::lexical_cast;
using boost::accumulators::left;
using boost::accumulators::min;
using boost::accumulators::max;
using boost::accumulators::mean;
using boost::accumulators::stats;
using boost::accumulators::p_square_quantile;
using boost::accumulators::quantile;
using boost::accumulators::quantile_probability;
using boost::accumulators::accumulator_set;

TwCommTnovr::TwCommTnovr(boost::asio::io_service& ioService, std::string ipAddress, short port, int32_t msgNum, int32_t groupNo)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("TwCommTnovr")),
    m_strand(ioService),
    m_ioService(ioService),
    m_socket(ioService),
    m_ep(ip::address::from_string(ipAddress), port),
    m_isReady(false),
    m_msgNum(msgNum),
    m_groupNo(groupNo),
    m_isRun(true)
{
}

void TwCommTnovr::Start()
{
    //SSCC_MLOG_INFO(m_logger, "Starting");
    m_socket.async_connect(m_ep,
                           m_strand.wrap(
                               boost::bind(&TwCommTnovr::HandleConnect, 
                                           this,
                                           boost::asio::placeholders::error)));

}


void TwCommTnovr::Close()
{
    SSCC_MLOG_INFO(m_logger, "Close");
    m_ioService.post(m_strand.wrap( 
            boost::bind(&TwCommTnovr::DoClose, this)));
}

bool TwCommTnovr::IsReady()
{
    return m_isReady;
}

bool TwCommTnovr::IsRun()
{
    return m_isRun;
}

void TwCommTnovr::Report()
{
    FILE* out;
    
    std::string filename = boost::str( format("./result/result%03d.txt") % m_groupNo );
    if( ( out = fopen(filename.c_str(), "w+") ) == NULL )
    {
        SSCC_MLOG_ERROR(m_logger, format("can not open %1") % filename);
        return;
    }

    fprintf( out, "no, clientSendTime, ServerRecTime, ServerSendTime, ClientRecvTime, timespan\n" );
    size_t i;
    for( i = 0; i < m_recvMsgs.size(); i++)
    {
        TimeRecord rec = m_recvMsgs[i].time;
        fprintf( out, "%d, %ld, %ld, %ld, %ld, %ld\n", 
                 m_recvMsgs[i].no,
                 rec.clientSendTime.tv_sec * 1000000 + rec.clientSendTime.tv_usec,
                 rec.serverRecvTime.tv_sec * 1000000 + rec.serverRecvTime.tv_usec,
                 rec.serverSendTime.tv_sec * 1000000 + rec.serverSendTime.tv_usec,
                 rec.clientRecvTime.tv_sec * 1000000 + rec.clientRecvTime.tv_usec,
                 rec.clientRecvTime.tv_sec * 1000000 + rec.clientRecvTime.tv_usec - (rec.clientSendTime.tv_sec * 1000000 + rec.clientSendTime.tv_usec)
               );
    }

    if ( out != NULL )
        fclose( out );

}

void TwCommTnovr::HandleConnect(const boost::system::error_code& error)
{
    //SSCC_MLOG_INFO(m_logger, format("Connect to peer:%d") % m_groupNo );

    if (!error)
    {
        MsgHeader *msgHeader = (MsgHeader*)m_sendData;
        msgHeader->groupNo = m_groupNo;
        msgHeader->bodySize = 10;
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + msgHeader->bodySize ),
                    m_strand.wrap(
                        boost::bind(&TwCommTnovr::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
    else 
    {
        SSCC_MLOG_ERROR(m_logger, "Connnect Error");
        DoClose();
    }
}

void TwCommTnovr::HandleReadHeader(const boost::system::error_code& error)
{
    if(!error)
    {
        MsgHeader *msgHeader = (MsgHeader*)m_recvData;
        //SSCC_MLOG_INFO(m_logger, format("msg header,grp:%d, gno:%d, no:%d") % m_groupNo % msgHeader->groupNo % msgHeader->no);
        if( msgHeader->bodySize > 0 )
        {
            gettimeofday(&(msgHeader->time.clientRecvTime), NULL);
            m_recvMsgs.push_back(*msgHeader);
            async_read(m_socket,
                       buffer(m_recvData + sizeof(MsgHeader), msgHeader->bodySize),
                       m_strand.wrap( 
                           boost::bind( &TwCommTnovr::HandleReadBody, 
                                        this, 
                                        boost::asio::placeholders::error)));

        }
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "handle read header error");
        DoClose();
    }
}

void TwCommTnovr::HandleReadBody(const boost::system::error_code& error)
{
    if(!error)
    {
        if(m_recvMsgs.size() == (size_t)m_msgNum)
        {
            SSCC_MLOG_INFO(m_logger, format("Group:%d Calc msg latency") % m_groupNo );
            m_isRun = false;
        }
        else
            async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&TwCommTnovr::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "handle read body error");
        DoClose();
    }
}


void TwCommTnovr::HandleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
        m_isReady = true;
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&TwCommTnovr::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "handle write error");
        DoClose();
    }
}

void TwCommTnovr::DoClose()
{
    SSCC_MLOG_INFO(m_logger, "DoClose");
    m_socket.close();
    m_isReady = false;
    m_isRun = false;
}



