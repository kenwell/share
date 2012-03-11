#include <unistd.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>
#include <sscc/log/config.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "tw_comm_order.h"
#include "tw_comm_tnovr.h"

using namespace std;
using namespace boost::interprocess;
using boost::format;

SSCC_DECLARE_FILE_LOGGER(f_logger, "tw_comm");



int main( int argc, char** argv )
{
    try
    {
        if (argc != 9)
        {
            std::cerr << "Usage: twcomm <orderhost> <orderport> <tnovrhost> <tnovrport> <msgnum> <speed> <groupno> <bodysize>\n";
            return 1;
        }

        int32_t msgNum = atoi(argv[5]);
        int32_t groupNo = atoi(argv[7]);
        int32_t bodysize = atoi(argv[8]);
        int32_t speed = atoi(argv[6]);


        std::string str = "./log/twcomm";
        str += argv[7];
        SSCC_LOG_CONFIG_FILE(str);
        ::boost::log::core::get()->set_filter(
            ::boost::log::filters::attr< ::sscc::log::SeverityLevel >("Severity") >= ::sscc::log::SEVERITY_LEVEL_DEBUG);


        //打开共享内存，并判断组号是否超出所有通信进程数目
        TwControlShm *control = NULL;
        shared_memory_object shm(open_only, SHMNAME, read_write);
        mapped_region region(shm, read_write);
        control = static_cast<TwControlShm*>(region.get_address());
        if(control == NULL)
        {
            SSCC_FLOG_ERROR(f_logger, "open shm error");
            return -1;
        }
        if(groupNo >= control->commNum)
        {
            SSCC_FLOG_ERROR(f_logger, format("groupno %d is more than comm num %d") % groupNo % control->commNum );
            return -1;
        }

        boost::asio::io_service orderIoService;
        boost::asio::io_service tnovrIoService;
        TwCommOrder order(orderIoService, std::string(argv[1]), atoi(argv[2]));
        TwCommTnovr tnovr(tnovrIoService, std::string(argv[3]), atoi(argv[4]), msgNum, groupNo); 
        order.Start();
        tnovr.Start();
        boost::thread_group threadGroup;
        threadGroup.create_thread(boost::bind(&boost::asio::io_service::run, &orderIoService));
        threadGroup.create_thread(boost::bind(&boost::asio::io_service::run, &tnovrIoService));

        //判断order和tnovr是否和服务器建立连接
        while(!order.IsReady() || !tnovr.IsReady())
            usleep(1000);

        //设置已经建立连接
        control->isReady[groupNo] = 1;

        //等待所有twcomm进程完成和服务器的连接创建
        while(!control->allReady)
            usleep(1000);

        //发送委托
        double time = (double)1000 / (double)speed;
        timeval sendTime;
        gettimeofday(&sendTime, NULL);
        for(int i = 0; i < msgNum; i++)
        {
            timeval now;
            gettimeofday(&now, NULL);

            long span = (now.tv_sec - sendTime.tv_sec) * 1000 + (now.tv_usec - sendTime.tv_usec) / 1000;
            if(span == 0)
                span = 1;
            long need = long(time * (double)(i+1));

            if( i!= 0 && (i % 5000) == 0)
            {
                long speed = i * 1000 / span;
                stringstream sstr;
                sstr << "span:" << span << " num:" << i << " speed:" << speed;
                SSCC_FLOG_INFO(f_logger, sstr.str());
            }

            if(span < need )
                usleep( (need - span) * 1000);
            MsgHeader msg;
            msg.no = i;
            msg.groupNo = groupNo;
            msg.bodySize = bodysize;
            order.Write(msg);
        }

        SSCC_FLOG_INFO(f_logger, format("Finish sending %d messages") % msgNum);

        //等待接受完所有成交
        while(tnovr.IsRun())
        {
            sleep(1);
        }
        //已经接受所有成交
        control->isFinish[groupNo] = 1;
        SSCC_FLOG_INFO(f_logger, format("Finish recieving %d messages") % msgNum);

        //等待所有twomm进程接受完成交
        while(!control->allFinish)
        {
            sleep(1);
        }
        SSCC_FLOG_INFO(f_logger, "All finish to calc latency");

        //统计延迟数据
        ResultTime resultTime = tnovr.Report();
        control->resultTimes[groupNo] = resultTime;
        control->isReport[groupNo] = 1;
        

        SSCC_FLOG_INFO(f_logger, "Finsih report and close order and tnovr thread");
        order.Close();
        tnovr.Close();

        threadGroup.join_all();
        SSCC_FLOG_INFO(f_logger, "Quit");

    }
    catch (std::exception& e)
    {
        stringstream sstr;
        sstr << "Exception: " << e.what() << std::endl;;
        SSCC_FLOG_ERROR(f_logger, sstr.str());
    }

    return 0;

}


