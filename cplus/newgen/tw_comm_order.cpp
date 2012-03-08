
#include "tw_comm_order.h"


using namespace boost::asio;

TwCommOrder::TwCommOrder(boost::asio::io_service& ioService, std::string ipAddress, short port)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("TwCommOrder")),
    m_strand(ioService),
    m_ioService(ioService),
    m_socket(ioService),
    m_ep(ip::address::from_string(ipAddress), port),
    m_isReady(false)
{
}

void TwCommOrder::Start()
{
 //   SSCC_MLOG_INFO(m_logger, "Starting");
    m_socket.async_connect(m_ep,
                           m_strand.wrap(
                               boost::bind(&TwCommOrder::HandleConnect, 
                                           this,
                                           boost::asio::placeholders::error)));

}

void TwCommOrder::Write(const MsgHeader& msg)
{
    m_ioService.post(m_strand.wrap(
            boost::bind(&TwCommOrder::DoWrite, this, msg)));
}

void TwCommOrder::Close()
{
    SSCC_MLOG_INFO(m_logger, "Close");
    m_ioService.post(m_strand.wrap( 
            boost::bind(&TwCommOrder::DoClose, this)));
}

bool TwCommOrder::IsReady()
{
    return m_isReady;
}

void TwCommOrder::HandleConnect(const boost::system::error_code& error)
{
   // SSCC_MLOG_INFO(m_logger, "Connect to peer");
    if (!error)
    {
        m_isReady = true;
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&TwCommOrder::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));

    }
    else 
    {
        SSCC_MLOG_ERROR(m_logger, "Connnect Error");
        DoClose();
    }
}

void TwCommOrder::HandleReadHeader(const boost::system::error_code& error)
{
    if(!error)
    {
   //     SSCC_MLOG_INFO(m_logger, "handle msg header");
        MsgHeader *msgHeader = (MsgHeader*)m_recvData;
        if( msgHeader->bodySize > 0 )
        {
            async_read(m_socket,
                       buffer(m_recvData + sizeof(MsgHeader), msgHeader->bodySize),
                       m_strand.wrap( 
                           boost::bind( &TwCommOrder::HandleReadBody, 
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

void TwCommOrder::HandleReadBody(const boost::system::error_code& error)
{
    if(!error)
    {
 //       SSCC_MLOG_INFO(m_logger, "handle msg body");
        //设置接收包头
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&TwCommOrder::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "handle read body error");
        DoClose();
    }
}

void TwCommOrder::DoWrite( const MsgHeader& msg)
{
    bool writeInProgress = !m_writeMsgs.empty();
    m_writeMsgs.push_back(msg);
    if (!writeInProgress)
    {
        memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
        MsgHeader *msgHeader = (MsgHeader*)m_sendData;
        gettimeofday(&(msgHeader->time.clientSendTime), NULL);
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                    m_strand.wrap(
                        boost::bind(&TwCommOrder::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
}

void TwCommOrder::HandleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
//        SSCC_MLOG_INFO(m_logger, "HandleWrite");
        m_writeMsgs.pop_front();
        if (!m_writeMsgs.empty())
        {
            memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
            MsgHeader *msgHeader = (MsgHeader*)m_sendData;
            gettimeofday(&(msgHeader->time.clientSendTime), NULL);
            async_write(m_socket,
                        boost::asio::buffer(m_sendData,
                                            sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                        m_strand.wrap( 
                            boost::bind(&TwCommOrder::HandleWrite, 
                                        this,
                                        boost::asio::placeholders::error)));
        }
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "handle write error");
        DoClose();
    }
}


void TwCommOrder::DoClose()
{
    SSCC_MLOG_INFO(m_logger, "DoClose");
    m_socket.close();
    m_isReady = false;
}

