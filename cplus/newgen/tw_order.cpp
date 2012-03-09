#include "tw_order.h"

#include <boost/thread.hpp>
#include <iostream>
#include <sscc/log/config.h>

#include "tw_order_server.h"

using namespace std;

using namespace boost::asio;

OrderClient::OrderClient(boost::asio::io_service& ioService, std::string ipAddress, short port)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("OrderClient")),
    m_strand(ioService),
    m_ioService(ioService),
    m_socket(ioService),
    m_ep(ip::address::from_string(ipAddress), port),
    m_isReady(false)
{
}

void OrderClient::Start()
{
    SSCC_MLOG_INFO(m_logger, "Starting");
    m_socket.async_connect(m_ep,
                           m_strand.wrap(
                               boost::bind(&OrderClient::HandleConnect, 
                                           this,
                                           boost::asio::placeholders::error)));

}

void OrderClient::Write(const MsgHeader& msg)
{
    m_ioService.post(m_strand.wrap(
            boost::bind(&OrderClient::DoWrite, this, msg)));
}

void OrderClient::Close()
{
    SSCC_MLOG_INFO(m_logger, "Close");
    m_ioService.post(m_strand.wrap( 
            boost::bind(&OrderClient::DoClose, this)));
}

bool OrderClient::IsReady()
{
    return m_isReady;
}

void OrderClient::HandleConnect(const boost::system::error_code& error)
{
    SSCC_MLOG_INFO(m_logger, "Connect to peer");
    if (!error)
    {
        m_isReady = true;
        m_ioService.post(m_strand.wrap(
            boost::bind(&OrderClient::DoWrite, this)));
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&OrderClient::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else 
    {
        SSCC_MLOG_ERROR(m_logger, "Connnect Error");
        DoClose();
    }
}

void OrderClient::HandleReadHeader(const boost::system::error_code& error)
{
    if(!error)
    {
   //     SSCC_MLOG_INFO(m_logger, "handle msg header");
        MsgHeader *msgHeader = (MsgHeader*)m_recvData;
        if( msgHeader->bodySize > 0 )
        {
            gettimeofday(&(msgHeader->time.serverRecvTime), NULL);
            async_read(m_socket,
                       buffer(m_recvData + sizeof(MsgHeader), msgHeader->bodySize),
                       m_strand.wrap( 
                           boost::bind( &OrderClient::HandleReadBody, 
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

void OrderClient::HandleReadBody(const boost::system::error_code& error)
{
    if(!error)
    {
 //       SSCC_MLOG_INFO(m_logger, "handle msg body");
        //设置接收包头
        async_read(m_socket,
                   buffer(m_recvData, sizeof(MsgHeader)),
                   m_strand.wrap(
                       boost::bind(&OrderClient::HandleReadHeader, 
                                   this,
                                   boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_ERROR(m_logger, "handle read body error");
        DoClose();
    }
}

void OrderClient::DoWrite( const MsgHeader& msg)
{
    //SSCC_MLOG_INFO(m_logger, "Do write");
    bool writeInProgress = !m_writeMsgs.empty();
    m_writeMsgs.push_back(msg);
    if (!writeInProgress && m_isReady)
    {
        memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                    m_strand.wrap(
                        boost::bind(&OrderClient::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
}

void OrderClient::DoWrite()
{
    bool writeInProgress = !m_writeMsgs.empty();
    if (writeInProgress && m_isReady)
    {
        memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                    m_strand.wrap(
                        boost::bind(&OrderClient::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
}

void OrderClient::HandleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
//        SSCC_MLOG_INFO(m_logger, "HandleWrite");
        m_writeMsgs.pop_front();
        if (!m_writeMsgs.empty())
        {
            memcpy(m_sendData, &(m_writeMsgs.front()), sizeof(MsgHeader) );
            async_write(m_socket,
                        boost::asio::buffer(m_sendData,
                                            sizeof(MsgHeader) + m_writeMsgs.front().bodySize ),
                        m_strand.wrap( 
                            boost::bind(&OrderClient::HandleWrite, 
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


void OrderClient::DoClose()
{
    SSCC_MLOG_INFO(m_logger, "DoClose");
    m_socket.close();
    m_isReady = false;
}



int main( int argc, char** argv )
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Usage: tworder <host> <sendport> <serverport>\n";
            return 1;
        }
        SSCC_LOG_CONFIG_CONSOLE();
        ::boost::log::core::get()->set_filter(
            ::boost::log::filters::attr< ::sscc::log::SeverityLevel >("Severity") >= ::sscc::log::SEVERITY_LEVEL_DEBUG);

        io_service clientIoService;
        io_service serverIoService;

        OrderClient c(clientIoService, std::string(argv[1]), atoi(argv[2]));

        TwOrderServer orderServer(serverIoService, atoi(argv[3]), c);
        c.Start();

        
        boost::thread_group threadGroup;
        for(int i =0; i < 1; i++)
        {
            threadGroup.create_thread(boost::bind(&io_service::run, &serverIoService));
        }
        
        clientIoService.run();

        threadGroup.join_all();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;

}
