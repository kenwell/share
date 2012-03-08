/**
 * @file tw_tnovr_server.h
 * @brief ·¢ËÍ³É½»
 * @author wugl
 */

#ifndef __TW_TNOVR_SEND_H_INCLUDED__
#define __TW_TNOVR_SEND_H_INCLUDED__

#include <deque>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <sscc/log/logger.h>

#include "tw_def.h"

using boost::asio::ip::tcp;

class TnovrClient;
class TwTnovrServer;
typedef std::deque<MsgHeader> MsgHeaderQueue;

class Session : public boost::enable_shared_from_this< Session >
{
public:
    Session(boost::asio::io_service& ioService, TwTnovrServer *tnovrServer);

    tcp::socket& Socket();

    void Start();
    void Write(const MsgHeader& msg);
    void Close();

private:

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
    tcp::socket m_socket;
    TwTnovrServer *m_tnovrServer;
    MsgHeaderQueue m_writeMsgs;
    enum { max_length = 1024 };
    char m_sendData[ max_length ];
    char m_recvData[max_length];
    int32_t m_groupNo;

};


typedef boost::shared_ptr<Session> SessionPtr;

class TwTnovrServer
{
public:
    TwTnovrServer(boost::asio::io_service& ioService, short port);

    void AddGroupToSession(int32_t groupNo, SessionPtr session);
    void RemoveSession(int32_t groupNo);
    bool LookupSession(int32_t groupNo);
    void WriteMessage(int32_t groupNo, const MsgHeader& msgHeader);

private:
    void HandleAccept(SessionPtr newSession,
                      const boost::system::error_code& error);
private:
    SSCC_DECLARE_MEMBER_LOGGER(m_logger);
    boost::asio::io_service& m_ioService;
    tcp::acceptor m_acceptor;
    std::vector<SessionPtr> m_sessions;
    std::map<int32_t, SessionPtr> m_groupToSession;
};



#endif
