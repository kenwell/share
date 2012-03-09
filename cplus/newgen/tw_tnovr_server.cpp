
#include "tw_tnovr_server.h"
#include "tw_tnovr.h"

#include <boost/format.hpp>

using boost::format;
using namespace boost::asio;


Session::Session(boost::asio::io_service& ioService, TwTnovrServer *tnovrServer)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("Session")),
    m_strand(ioService),
    m_ioService(ioService),
    m_socket(ioService),
    m_tnovrServer(tnovrServer),
    m_writeMsgs(NULL),
    m_groupNo(-1)
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

void Session::Write(const MsgHeader& msg)
{
    m_ioService.post(m_strand.wrap(
            boost::bind(&Session::DoWrite, this, msg)));
}

void Session::Write()
{
    m_ioService.post(m_strand.wrap(
            boost::bind(&Session::DoWrite, this)));
}

void Session::Close()
{
    SSCC_MLOG_INFO(m_logger, "Close");
    m_ioService.post(m_strand.wrap(
            boost::bind(&Session::DoClose, this)));
}


void Session::SetMsgQueue(MsgHeaderQueue *pwriteMsgs)
{
    m_writeMsgs = pwriteMsgs;
}

void Session::HandleReadHeader(const boost::system::error_code& error)
{
    if(!error)
    {
        MsgHeader *msgHeader = (MsgHeader*)m_recvData;
        if(m_groupNo == -1)
        {
            m_groupNo = msgHeader->groupNo;
            m_tnovrServer->AddGroupToSession(m_groupNo, shared_from_this());
        }
        if( msgHeader->bodySize > 0 )
        {
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

void Session::DoWrite( const MsgHeader& msg)
{
    //SSCC_MLOG_INFO(m_logger, "Do write");
    bool writeInProgress = !m_writeMsgs->empty();
    m_writeMsgs->push_back(msg);
    if (!writeInProgress)
    {
        memcpy(m_sendData, &(m_writeMsgs->front()), sizeof(MsgHeader) );
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + m_writeMsgs->front().bodySize ),
                    m_strand.wrap(
                        boost::bind(&Session::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
}

void Session::DoWrite()
{
    //SSCC_MLOG_INFO(m_logger, "Do write");
    bool writeInProgress = !m_writeMsgs->empty();
    if (writeInProgress)
    {
        memcpy(m_sendData, &(m_writeMsgs->front()), sizeof(MsgHeader) );
        async_write(m_socket,
                    boost::asio::buffer(m_sendData,
                                        sizeof(MsgHeader) + m_writeMsgs->front().bodySize ),
                    m_strand.wrap(
                        boost::bind(&Session::HandleWrite, 
                                    this,
                                    boost::asio::placeholders::error)));
    }
}

void Session::HandleWrite(const boost::system::error_code& error)
{
    if (!error)
    {
//        SSCC_MLOG_INFO(m_logger, "HandleWrite");
        if(!m_writeMsgs->empty())
            m_writeMsgs->pop_front();
        if (!m_writeMsgs->empty())
        {
            memcpy(m_sendData, &(m_writeMsgs->front()), sizeof(MsgHeader) );
            async_write(m_socket,
                        boost::asio::buffer(m_sendData,
                                            sizeof(MsgHeader) + m_writeMsgs->front().bodySize ),
                        m_strand.wrap( 
                            boost::bind(&Session::HandleWrite, 
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

void Session::DoClose()
{
    SSCC_MLOG_INFO(m_logger, "DoClose");
    m_socket.close();
    if(m_groupNo != -1)
        m_tnovrServer->RemoveSession(m_groupNo);
}


TwTnovrServer::TwTnovrServer(boost::asio::io_service& ioService, short port, int maxGroupNum)
 :  m_logger(SSCC_INIT_MEMBER_LOGGER("TwTnovrServer")),
    m_strand(ioService),
    m_ioService(ioService),
    m_acceptor(ioService, tcp::endpoint(tcp::v4(), port)),
    m_maxGroupNum(maxGroupNum),
    m_msgHeaderQueues(new MsgHeaderQueue[maxGroupNum])
{


    SessionPtr newSession(new Session(m_ioService, this));
    m_acceptor.async_accept(newSession->Socket(),
                            boost::bind(&TwTnovrServer::HandleAccept, 
                                        this, 
                                        newSession,
                                        boost::asio::placeholders::error));
}


void TwTnovrServer::AddGroupToSession(int32_t groupNo, SessionPtr session)
{
    m_ioService.post(m_strand.wrap(
            boost::bind(&TwTnovrServer::DoAddGroup, this, groupNo, session)));
}

void TwTnovrServer::RemoveSession(int32_t groupNo)
{
    m_groupToSession.erase(groupNo);
}

bool TwTnovrServer::LookupSession(int32_t groupNo)
{
    return m_groupToSession.find(groupNo) != m_groupToSession.end();
}

void TwTnovrServer::WriteMessage(int32_t groupNo, const MsgHeader& msgHeader)
{
    std::map<int32_t, SessionPtr>::iterator iter = m_groupToSession.find(groupNo);
    if(iter != m_groupToSession.end())
        (iter->second)->Write(msgHeader);
    else
    {
        m_ioService.post(m_strand.wrap(
                boost::bind(&TwTnovrServer::DoWrite, this, groupNo, msgHeader)));
    }
}

void TwTnovrServer::HandleAccept(SessionPtr newSession,
                          const boost::system::error_code& error)
{
    if (!error)
    {
        //SSCC_MLOG_INFO(m_logger, "Accept new connection");
        m_sessions.push_back(newSession);
        newSession->Start();
        SessionPtr newSession(new Session(m_ioService, this));
        m_acceptor.async_accept(newSession->Socket(),
                                m_strand.wrap(boost::bind(&TwTnovrServer::HandleAccept, 
                                            this, 
                                            newSession,
                                            boost::asio::placeholders::error)));
    }
    else
    {
        SSCC_MLOG_INFO(m_logger, "Accept error");
    }
}

void TwTnovrServer::DoWrite(int32_t groupNo, const MsgHeader& msgHeader)
{
    std::map<int32_t, SessionPtr>::iterator iter = m_groupToSession.find(groupNo);
    if(iter != m_groupToSession.end())
        (iter->second)->Write(msgHeader);
    else
    {
        if(groupNo >= m_maxGroupNum || groupNo < 0)
        {
            SSCC_MLOG_ERROR(m_logger, format("GroupNo %d is more than max group number %d") % groupNo % m_maxGroupNum );
        }
        else
            m_msgHeaderQueues[groupNo].push_back(msgHeader);
    }
}

void TwTnovrServer::DoAddGroup(int32_t groupNo, SessionPtr session)
{
    m_groupToSession[groupNo] = session;
    session->SetMsgQueue( &(m_msgHeaderQueues[groupNo]) );
    session->Write();
}
