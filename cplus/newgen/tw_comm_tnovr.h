/**
 * @file tw_comm_tnovr.h
 * @brief 接受成交
 * @author wugl
 */

#ifndef __TW_COMM_TNOVR_H_INCLUDED__
#define __TW_COMM_TNOVR_H_INCLUDED__

#include <deque>
#include <vector>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/error_code.hpp>

#include <sscc/log/logger.h>

#include "tw_def.h"


typedef std::deque<MsgHeader> MsgHeaderQueue;

class TwCommTnovr
{
public:
    TwCommTnovr(boost::asio::io_service& io_service, std::string ipAddress, short port, int32_t msgNum, int32_t m_groupNo);

    void Start();


    void Close();

    bool IsReady();
    bool IsRun();
    void Report();

private:

    void HandleConnect(const boost::system::error_code& error);

    void HandleReadHeader(const boost::system::error_code& error);
    void HandleReadBody(const boost::system::error_code& error);

    void HandleWrite(const boost::system::error_code& error);

    void DoClose();

private:
    SSCC_DECLARE_MEMBER_LOGGER(m_logger);
    boost::asio::strand m_strand;
    boost::asio::io_service& m_ioService;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::endpoint m_ep;
    MsgHeaderQueue m_writeMsgs;
    std::vector<MsgHeader> m_recvMsgs;
    enum { max_length = 1024 };
    char m_sendData[ max_length ];
    char m_recvData[ max_length ];

    bool m_isReady;
    int32_t m_msgNum;
    int32_t m_groupNo;
    bool m_isRun;
};


#endif
