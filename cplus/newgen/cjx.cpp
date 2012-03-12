
#include <iostream>
#include <boost/thread.hpp>
#include <sscc/log/config.h>
#include "cjx_tnovr.h"
#include "cjx_order.h"



using namespace boost::asio;


int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: cjx <tnovrport> <orderport>\n";
      return 1;
    }
    SSCC_LOG_CONFIG_CONSOLE();
    
    ::boost::log::core::get()->set_filter(
        ::boost::log::filters::attr< ::sscc::log::SeverityLevel >("Severity") >= ::sscc::log::SEVERITY_LEVEL_DEBUG);

    boost::asio::io_service ioService;
    boost::asio::io_service ioTnovrService;
    TnovrServer tnovrServer(ioService, atoi(argv[1]));
    OrderServer orderServer(ioTnovrService, atoi(argv[2]), tnovrServer);
    tnovrServer.Start();
    orderServer.Start();

    boost::thread tnovrThread(boost::bind(&io_service::run, &ioTnovrService));
    ioService.run();
    
    tnovrThread.join();

  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

