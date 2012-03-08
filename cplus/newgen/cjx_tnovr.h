
/**
 * @file cjx_tnovr.h
 * @brief 成交分发发送成交
 * @author wugl
 */

#ifndef __CJX_TNOVR_H_INCLUDED__
#define __CJX_TNOVR_H_INCLUDED__


#include <deque>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>

#include <sscc/log/logger.h>

#include "tw_def.h"

using boost::asio::ip::tcp;

typedef std::deque<MsgHeader> MsgHeaderQueue;

class TnovrServer
{
public:
    TnovrServer(boost::asio::io_service& io_service, short port);

    void Start();
    void Close();
    void Write(const MsgHeader& msg);

    
private:

    void HandleAccept( const boost::system::error_code& error );
    void HandleReadHeader(const boost::system::error_code& error);
    void HandleReadBody(const boost::system::error_code& error);

    void HandleWrite(const boost::system::error_code& error);

    void DoWrite( const MsgHeader& msg);
    void DoWrite();
    void DoClose();

private:
    SSCC_DECLARE_MEMBER_LOGGER(m_logger);
    boost::asio::strand m_strand;
    boost::asio::io_service& m_ioService;
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    enum { max_length = 1024 };
    char m_data[max_length];
    char m_recvData[max_length];
    char m_sendData[max_length];
    MsgHeaderQueue m_writeMsgs;

    bool m_isAccepted;
};

   



#endif
