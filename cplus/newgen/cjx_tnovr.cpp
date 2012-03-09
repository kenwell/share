
#include "cjx_tnovr.h"

using namespace boost::asio;

TnovrServer::TnovrServer(boost::asio::io_service& ioService, short port)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("TnovrServer")),
    m_strand(ioService),
    m_ioService(ioService),
    m_acceptor(ioService, tcp::endpoint(tcp::v4(), port)),
    m_socket(ioService),
    m_isAccepted(false)
{
}

void TnovrServer::Start()
{
    SSCC_MLOG_INFO(m_logger, "Starting");
    m_acceptor.async_accept(m_socket,
                            m_strand.wrap(
                                boost::bind(&TnovrServer::HandleAccept, 
                                            this, 
                                            boost::asio::placeholders::error)));
}

void TnovrServer::HandleAccept( const boost::system::error_code& error )
{
    SSCC_MLOG_INFO(m_logger, "Accept connections");
    if (!error)
    {
        m_isAccepted = true;
        m_ioService.post(m_strand.wrap(
            boost::bind(&TnovrServer::DoWrite, this)));
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&TnovrServer::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Accepted failed");
    }
}

void TnovrServer::Close()
{
    SSCC_MLOG_INFO(m_logger, "Close");
    m_ioService.post(m_strand.wrap(
            boost::bind(&TnovrServer::DoClose, this)));
}


void TnovrServer::Write(const MsgHeader& msg)
{
    m_ioService.post(m_strand.wrap(
            boost::bind(&TnovrServer::DoWrite, this, msg)));
}



void TnovrServer::HandleReadHeader(const boost::system::error_code& error)
{
    if(!error)
    {
        MsgHeader *msgHeader = (MsgHeader*)m_recvData;
        if( msgHeader->bodySize > 0 )
        {
            async_read(m_socket,
                       buffer(m_recvData + sizeof(MsgHeader), msgHeader->bodySize),
                       m_strand.wrap( boost::bind( &TnovrServer::HandleReadBody, 
                                                   this, 
                                                   boost::asio::placeholders::error)));

        }
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Handle read header error");
        DoClose();
    }
}

void TnovrServer::HandleReadBody(const boost::system::error_code& error)
{
    if(!error)
    {
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&TnovrServer::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Handle Read body error");
        DoClose();
    }
}

void TnovrServer::DoWrite( const MsgHeader& msg)
{
    bool writeInProgress = !m_writeMsgs.empty();
    m_writeMsgs.push_back(msg);
    if (!writeInProgress & m_isAccepted)
    {
        memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
        //设置发送时间，发送包
        MsgHeader *msgHeader = (MsgHeader*)m_sendData;
        gettimeofday(&(msgHeader->time.serverSendTime), NULL);
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                    m_strand.wrap(
                        boost::bind(&TnovrServer::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
}

void TnovrServer::DoWrite()
{
    bool writeInProgress = !m_writeMsgs.empty();
    if (writeInProgress & m_isAccepted)
    {
        memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
        //设置发送时间，发送包
        MsgHeader *msgHeader = (MsgHeader*)m_sendData;
        gettimeofday(&(msgHeader->time.serverSendTime), NULL);
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                    m_strand.wrap(
                        boost::bind(&TnovrServer::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
}

void TnovrServer::HandleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
        if (!m_writeMsgs.empty())
        {
            memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
            async_write(m_socket,
                        boost::asio::buffer(m_sendData,
                                            sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                        m_strand.wrap( 
                            boost::bind(&TnovrServer::HandleWrite, 
                                        this,
                                        boost::asio::placeholders::error)));
        }
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "Handle write error");
        DoClose();
    }
}


void TnovrServer::DoClose()
{
    SSCC_MLOG_INFO(m_logger, "DoClose");
    m_socket.close();
    m_isAccepted = false;
    m_acceptor.async_accept(m_socket,
                            m_strand.wrap(
                                boost::bind(&TnovrServer::HandleAccept, 
                                            this, 
                                            boost::asio::placeholders::error)));
}


