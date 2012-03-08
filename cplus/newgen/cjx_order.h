/**
 * @file cjx_order.h
 * @brief 成交分发接收委托
 * @author wugl
 */

#ifndef __CJX_ORDER_H_INCLUDED__
#define __CJX_ORDER_H_INCLUDED__


#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>

#include <sscc/log/logger.h>

#include "tw_def.h"

using boost::asio::ip::tcp;

class TnovrServer;

class OrderServer
{
public:
    OrderServer(boost::asio::io_service& io_service, short port, TnovrServer& tnovrServer);
    void Start();
    void Close();
private:
    void HandleAccept( const boost::system::error_code& error );
    void HandleReadHeader(const boost::system::error_code& error);
    void HandleReadBody(const boost::system::error_code& error);

    void DoClose();
private:
    SSCC_DECLARE_MEMBER_LOGGER(m_logger);
    boost::asio::strand m_strand;
    boost::asio::io_service& m_ioService;
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;

    TnovrServer& m_tnovrServer;
    enum { max_length = 1024 };
    char m_data[max_length];

};

   



#endif
