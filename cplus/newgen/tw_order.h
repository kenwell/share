/**
 * @file tw_order.h
 * @brief 定义委托发送
 * @author wugl
 */

#ifndef __TW_ORDER_H_INCLUDED__
#define __TW_ORDER_H_INCLUDED__

#include <deque>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>

#include <sscc/log/logger.h>

#include "tw_def.h"


typedef std::deque<MsgHeader> MsgHeaderQueue;

class OrderClient
{
public:
    OrderClient(boost::asio::io_service& io_service, std::string ipAddress, short port);

    void Start();

    void Write(const MsgHeader& msg);

    void Close();

    bool IsReady();

private:

    void HandleConnect(const boost::system::error_code& error);

    void HandleReadHeader(const boost::system::error_code& error);
    void HandleReadBody(const boost::system::error_code& error);

    void DoWrite( const MsgHeader& msg);
    void DoWrite();

    void HandleWrite(const boost::system::error_code& error);

    void DoClose();

private:
    SSCC_DECLARE_MEMBER_LOGGER(m_logger);
    boost::asio::strand m_strand;
    boost::asio::io_service& m_ioService;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::endpoint m_ep;
    MsgHeaderQueue m_writeMsgs;
    enum { max_length = 1024 };
    char m_sendData[ max_length ];
    char m_recvData[ max_length ];

    bool m_isReady;
};


#endif
