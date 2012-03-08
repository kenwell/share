#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>


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


int GenSubProc(TwCommArgs commArgs);

int main( int argc, char** argv )
{
    if (argc != 9)
    {
        std::cerr << "Usage: twsuper <orderhost> <orderport> <tnovrhost> <tnovrport> <msgnum> <speed> <groupno> <bodysize>\n";
        return 1;
    }

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
