
#include "tw_order_server.h"
#include "tw_order.h"

#include <boost/format.hpp>
using boost::format;

using namespace boost::asio;


Session::Session(boost::asio::io_service& ioService, OrderClient& orderClient)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("Session")),
    m_strand(ioService),
    m_ioService(ioService),
    m_socket(ioService),
    m_orderClient(orderClient)
{
}

tcp::socket& Session::Socket()
{
    return m_socket;
}

void Session::Start()
{
    //SSCC_MLOG_INFO(m_logger, "Starting");
    async_read(m_socket,
               buffer(m_recvData, sizeof(MsgHeader)),
               m_strand.wrap(
                   boost::bind(&Session::HandleReadHeader, 
                               shared_from_this(),
                               boost::asio::placeholders::error)));
}

void Session::Close()
{
    SSCC_MLOG_INFO(m_logger, "Close");
    m_ioService.post(boost::bind(&Session::DoClose, this));
}

void Session::HandleReadHeader(const boost::system::error_code& error)
{
    if(!error)
    {
        MsgHeader *msgHeader = (MsgHeader*)m_recvData;
        //SSCC_MLOG_INFO(m_logger, format("msg header, gno:%d, no:%d") % msgHeader->groupNo % msgHeader->no);
        if( msgHeader->bodySize > 0 )
        {
            gettimeofday(&(msgHeader->time.orderRecvTime), NULL);
            async_read(m_socket,
                       buffer(m_recvData + sizeof(MsgHeader), msgHeader->bodySize),
                       m_strand.wrap(
                           boost::bind( &Session::HandleReadBody, 
                                        shared_from_this(), 
                                        boost::asio::placeholders::error)));

        }
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Handle read header error");
        DoClose();
    }
}

void Session::HandleReadBody(const boost::system::error_code& error)
{
    if(!error)
    {
        //设置发送时间，发送包
        MsgHeader *msgHeader = (MsgHeader*)m_recvData;
        m_orderClient.Write(*msgHeader);
        //设置接收包头
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&Session::HandleReadHeader, 
                                   shared_from_this(),
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Handle read body error");
        DoClose();
    }
}


void Session::DoClose()
{
    SSCC_MLOG_INFO(m_logger, "DoClose");
    m_socket.close();
}


TwOrderServer::TwOrderServer(boost::asio::io_service& ioService, short port, OrderClient& orderClient)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("TwOrderServer")),
    m_ioService(ioService),
    m_acceptor(ioService, tcp::endpoint(tcp::v4(), port)),
    m_orderClient(orderClient)
{
    SessionPtr newSession(new Session(m_ioService, orderClient));
    m_acceptor.async_accept(newSession->Socket(),
                            boost::bind(&TwOrderServer::HandleAccept, 
                                        this, 
                                        newSession,
                                        boost::asio::placeholders::error));
}

void TwOrderServer::HandleAccept(SessionPtr newSession,
                          const boost::system::error_code& error)
{
    if (!error)
    {
        //SSCC_MLOG_INFO(m_logger, "Accept new connection");
        newSession->Start();
        newSession.reset(new Session(m_ioService, m_orderClient));
        m_acceptor.async_accept(newSession->Socket(),
                                boost::bind(&TwOrderServer::HandleAccept, 
                                            this, 
                                            newSession,
                                            boost::asio::placeholders::error));
    }
    else
    {
        SSCC_MLOG_INFO(m_logger, "Accept error");
    }
}


