#include "cjx_order.h"
#include "cjx_tnovr.h"

#include <boost/format.hpp>
using boost::format;

using namespace boost::asio;


OrderServer::OrderServer(boost::asio::io_service& ioService, short port, TnovrServer& tnovrServer)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("OrderServer")),
    m_strand(ioService),
    m_ioService(ioService),
    m_acceptor(ioService, tcp::endpoint(tcp::v4(), port)),
    m_socket(ioService),
    m_tnovrServer( tnovrServer )
{
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
}

void OrderServer::Start()
{
    SSCC_MLOG_INFO(m_logger, "Starting");
    m_acceptor.async_accept(m_socket,
                            m_strand.wrap(
                                boost::bind(&OrderServer::HandleAccept, 
                                            this, 
                                            boost::asio::placeholders::error)));
}

void OrderServer::Close()
{
    SSCC_MLOG_INFO(m_logger, "Close");
    m_ioService.post(boost::bind(&OrderServer::DoClose, this));
}


void OrderServer::HandleAccept( const boost::system::error_code& error )
{
    SSCC_MLOG_INFO(m_logger, "Accept connections");
    if (!error)
    {
        async_read(m_socket,
                   buffer(m_data, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&OrderServer::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Accepted failed");
    }
}

void OrderServer::HandleReadHeader(const boost::system::error_code& error)
{
    if(!error)
    {
        MsgHeader *msgHeader = (MsgHeader*)m_data;
        //SSCC_MLOG_INFO(m_logger, format("msg header gno:%d, no:%d") % msgHeader->groupNo % msgHeader->no);
        if( msgHeader->bodySize > 0 )
        {
            gettimeofday(&(msgHeader->time.serverRecvTime), NULL);
            async_read(m_socket,
                       buffer(m_data + sizeof(MsgHeader), msgHeader->bodySize),
                       m_strand.wrap(
                           boost::bind( &OrderServer::HandleReadBody, 
                                        this, 
                                        boost::asio::placeholders::error)));

        }
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Read header error");
        DoClose();
    }
}

void OrderServer::HandleReadBody(const boost::system::error_code& error)
{
    if(!error)
    {
      //  SSCC_MLOG_INFO(m_logger, "Read body");
        //设置发送时间，发送包
        MsgHeader *msgHeader = (MsgHeader*)m_data;
        m_tnovrServer.Write(*msgHeader);
        //设置接收包头
        async_read(m_socket,
                   buffer(m_data, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&OrderServer::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Read body error");
        DoClose();
    }
}


void OrderServer::DoClose()
{
    SSCC_MLOG_INFO(m_logger, "DoClose");
    m_socket.close();
    m_acceptor.async_accept(m_socket,
                            m_strand.wrap(
                                boost::bind(&OrderServer::HandleAccept, 
                                            this, 
                                            boost::asio::placeholders::error)));
}


