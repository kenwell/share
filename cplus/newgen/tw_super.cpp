#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "tw_def.h"


using namespace boost::interprocess;
using namespace std;

class TwCommArgs
{
public:
    std::string procName;
    std::string orderHost;
    std::string orderPort;
    std::string tnovrHost;
    std::string tnovrPort;
    std::string msgNum;
    std::string speed;
    std::string groupNo;
    std::string bodySize;
};

struct shm_remove
{
    shm_remove()
    {
        shared_memory_object::remove(SHMNAME);
    }
    
    ~shm_remove()
    {
        shared_memory_object::remove(SHMNAME);
    }
};

int GenSubProc(TwCommArgs commArgs);


int main( int argc, char** argv )
{
    if (argc != 9)
    {
        std::cerr << "Usage: twsuper <orderhost> <orderport> <tnovrhost> <tnovrport> <msgnum> <speed> <groupno> <bodysize>\n";
        return 1;
    }

    //初始化参数
    TwCommArgs commArgs;
    commArgs.procName = "twcomm";
    commArgs.orderHost = argv[1];
    commArgs.orderPort = argv[2];
    commArgs.tnovrHost = argv[3];
    commArgs.tnovrPort = argv[4];
    commArgs.msgNum = argv[5];
    commArgs.speed = argv[6];
    commArgs.bodySize = argv[8];

    int groupNo = atoi(argv[7]);

    //初始化共享内存
    shm_remove remove;
    TwControlShm *control = NULL;
    shared_memory_object shm(create_only, SHMNAME, read_write);
    shm.truncate(sizeof(TwControlShm));
    mapped_region region(shm, read_write);
    control = static_cast<TwControlShm*>(region.get_address());
    if( control == NULL)
    {
        std::cerr << "create shm error!" << endl;
        return -1;
    }
    memset(control, 0, sizeof(TwControlShm));
    control->commNum = groupNo;

    //根据group no生成通信子进程
    std::cout << "Begin to generate " << groupNo << " twcomm processes" << std::endl;
    for(int i = 0; i < groupNo; i++)
    {
        stringstream sstr;
        sstr << i;
        commArgs.groupNo = sstr.str();
        if( GenSubProc(commArgs) )
        {
            std::cerr << "Gen sub proc error" << i << endl;
            return -1;
        }
    }

    //判断所有线程可以发送
    std::cout << "Begin to judge all twcomm whether is ready to send messages" << std::endl;
    while(true)
    {
        int num = 0;
        for(int i = 0; i < groupNo; i++)
        {
            if(control->isReady[i] == 1)
                num++;
        }

        if(num == groupNo)
            break;
        sleep(1);
    }
    control->allReady = 1;

    std::cout << "Twcomm begin to send messages" << std::endl;

    //判断所有线程是否完成数据发送
    while(true)
    {
        int num = 0;
        for(int i = 0; i < groupNo; i++)
        {
            if(control->isFinish[i] == 1)
                num++;
        }

        if(num == groupNo)
            break;
        std::cout << num << " twcomm process finish sending message" << std::endl;
        sleep(1);
    }
    control->allFinish = 1;
    std::cout << "Finish message sending";

}

int GenSubProc( TwCommArgs commArgs )
{
    int	childpid;
    char path[256];

    childpid = fork();
    if (childpid < 0) {
        std::cerr << "fork() failed when create sub-process! (errno = " << errno << ")" << std::endl;
        return(-1);
    }

    if (childpid == 0) {
        umask(0077);
        sprintf(path, "%s/bin/%s", getenv("HOME"), commArgs.procName.c_str());
        //sprintf(path, "./%s", commArgs.procName.c_str());
        execl(path, 
              commArgs.procName.c_str(), 
              commArgs.orderHost.c_str(),
              commArgs.orderPort.c_str(),
              commArgs.tnovrHost.c_str(),
              commArgs.tnovrPort.c_str(),
              commArgs.msgNum.c_str(),
              commArgs.speed.c_str(),
              commArgs.groupNo.c_str(),
              commArgs.bodySize.c_str(),
              NULL);
        exit(1); /* if execl failed, exit! */
    }
    return(0);
} 


