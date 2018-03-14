#include <rpc/server.h>

#include "timeService.h"

TimeService::TimeService(uint16_t port)
    : synchronized(false)
{
    rpc::server srv(port);
        
    srv.bind("synchronize",
        [this]() 
        {
            time_t deliveryTime = time(nullptr);
            return synchronize(deliveryTime);
        });
    
    srv.bind("syncFinished", 
        [this]()
        {
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
