
#include "tw_tnovr.h"
#include <iostream>
#include <boost/format.hpp>
#include <sscc/log/config.h>

#include "tw_tnovr_server.h"

using boost::format;
using namespace std;

using namespace boost::asio;

TnovrClient::TnovrClient(boost::asio::io_service& ioService, std::string ipAddress, short port, TwTnovrServer& tnovrServer)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("TnovrClient")),
    m_strand(ioService),
    m_ioService(ioService),
    m_socket(ioService),
    m_ep(ip::address::from_string(ipAddress), port),
    m_tnovrServer(tnovrServer),
    m_isReady(false)
{
}

void TnovrClient::Start()
{
    //SSCC_MLOG_INFO(m_logger, "Starting");
    m_socket.async_connect(m_ep,
                           m_strand.wrap(
                               boost::bind(&TnovrClient::HandleConnect, 
                                           this,
                                           boost::asio::placeholders::error)));

}

void TnovrClient::Write(const MsgHeader& msg)
{
    m_ioService.post(m_strand.wrap(
            boost::bind(&TnovrClient::DoWrite, this, msg)));
}

void TnovrClient::Close()
{
    SSCC_MLOG_INFO(m_logger, "Close");
    m_ioService.post(m_strand.wrap( 
            boost::bind(&TnovrClient::DoClose, this)));
}

bool TnovrClient::IsReady()
{
    return m_isReady;
}

void TnovrClient::HandleConnect(const boost::system::error_code& error)
{
    SSCC_MLOG_INFO(m_logger, "Connnected to cjx tnovr");
    if (!error)
    {
        m_isReady = true;
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&TnovrClient::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else 
    {
        SSCC_MLOG_ERROR(m_logger, "Connnect Error");
        DoClose();
    }
}

void TnovrClient::HandleReadHeader(const boost::system::error_code& error)
{
    if(!error)
    {
        MsgHeader *msgHeader = (MsgHeader*)m_recvData;
        //SSCC_MLOG_INFO(m_logger, format("msg headergno:%d, no:%d") % msgHeader->groupNo % msgHeader->no);
        if( msgHeader->bodySize > 0 )
        {
            m_tnovrServer.WriteMessage(msgHeader->groupNo, *msgHeader);
            async_read(m_socket,
                       buffer(m_recvData + sizeof(MsgHeader), msgHeader->bodySize),
                       m_strand.wrap( 
                           boost::bind( &TnovrClient::HandleReadBody, 
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

void TnovrClient::HandleReadBody(const boost::system::error_code& error)
{
    if(!error)
    {
        //SSCC_MLOG_INFO(m_logger, "Read data body");

        //设置接收包头
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&TnovrClient::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "handle read body error");
        DoClose();
    }
}

void TnovrClient::DoWrite( const MsgHeader& msg)
{
    bool writeInProgress = !m_writeMsgs.empty();
    m_writeMsgs.push_back(msg);
    if (!writeInProgress)
    {
        memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                    m_strand.wrap(
                        boost::bind(&TnovrClient::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
}

void TnovrClient::HandleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
        m_writeMsgs.pop_front();
        if (!m_writeMsgs.empty())
        {
            memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
            async_write(m_socket,
                        boost::asio::buffer(m_sendData,
                                            sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                        m_strand.wrap( 
                            boost::bind(&TnovrClient::HandleWrite, 
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


void TnovrClient::DoClose()
{
    SSCC_MLOG_INFO(m_logger, "DoClose");
    m_socket.close();
}


int main( int argc, char** argv )
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Usage: twtnovr <cjxhost> <cjxport> <tnovrport>\n";
            return 1;
        }
        SSCC_LOG_CONFIG_CONSOLE();
        ::boost::log::core::get()->set_filter(
            ::boost::log::filters::attr< ::sscc::log::SeverityLevel >("Severity") >= ::sscc::log::SEVERITY_LEVEL_DEBUG);

        boost::asio::io_service io_service;

        TwTnovrServer tnovrServer(io_service, atoi(argv[3]));
        TnovrClient c(io_service, std::string(argv[1]), atoi(argv[2]), tnovrServer);
        c.Start();
        io_service.run();


    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
