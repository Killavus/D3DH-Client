#include <rpc/this_server.h>

#include "timeService.h"
#include "utils.h"

TimeService::TimeService(uint16_t port)
    : rpcSrv(port)
    , synchronized(false)
{        
    rpcSrv.bind("synchronize",
        [this]() 
        {
            time_t deliveryTime = time(nullptr);
            return synchronize(deliveryTime);
        });
    
    rpcSrv.bind("syncFinished", 
        [this]()
        {
            synchronized = true;
            rpc::this_server().stop();
        });
    
    rpcSrv.async_run();
}

std::pair<time_t, time_t> TimeService::synchronize(time_t deliveryTime)
{
    time_t sendingTime = time(nullptr);
    return { deliveryTime, sendingTime };
}

bool TimeService::isSynchronized()
{
    return synchronized;
}
