#include <iostream>

#include <rpc/server.h>

#include "timeService.h"

TimeService::TimeService(uint16_t port)
    : synchronized(false)
{
    rpc::server srv(port);
        
    srv.bind("synchronize",
        [this]() 
        {
	    std::cout << "CLIENT SYNCHRONIZE" << std::endl;
            time_t deliveryTime = time(nullptr);
            return synchronize(deliveryTime);
        });
    
    srv.bind("syncFinished", 
        [this]()
        {
            std::cout << "CLIENT SYNCFINISHED" << std::endl;
            synchronized = true;
        });
    
    srv.async_run();
}

std::pair<time_t, time_t> TimeService::synchronize(time_t deliveryTime)
{
    time_t sendingTime = time(nullptr);
    return std::make_pair(deliveryTime, sendingTime);
}

bool TimeService::isSynchronized()
{
    return synchronized;
}
