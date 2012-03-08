
/**
 * @file tworderserver.h
 * @brief 定义委托接受
 * @author wugl
 */

#ifndef __TW_ORDER_SERVER_H_INCLUDED__
#define __TW_ORDER_SERVER_H_INCLUDED__


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <sscc/log/logger.h>

#include "tw_def.h"

using boost::asio::ip::tcp;

class OrderClient;

class Session : public boost::enable_shared_from_this< Session >
{
public:
    Session(boost::asio::io_service& ioService, OrderClient& orderClient);

    tcp::socket& Socket();

    void Start();
    void Close();

private:

    void HandleReadHeader(const boost::system::error_code& error);
    void HandleReadBody(const boost::system::error_code& error);

    void DoClose();

private:
    SSCC_DECLARE_MEMBER_LOGGER(m_logger);
    boost::asio::strand m_strand;
    boost::asio::io_service& m_ioService;
    tcp::socket m_socket;
    OrderClient &m_orderClient;
    enum { max_length = 1024 };
    char m_recvData[max_length];
};


typedef boost::shared_ptr<Session> SessionPtr;

class TwOrderServer
{
public:
    TwOrderServer(boost::asio::io_service& ioService, short port, OrderClient& orderClient);

    void HandleAccept(SessionPtr newSession,
                      const boost::system::error_code& error);

private:
    SSCC_DECLARE_MEMBER_LOGGER(m_logger);
    boost::asio::io_service& m_ioService;
    tcp::acceptor m_acceptor;
    OrderClient& m_orderClient;
};



#endif
