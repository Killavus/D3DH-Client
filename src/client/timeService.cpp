#include <rpc/this_server.h>

#include "timeService.h"
#include "type_definitions.h"
#include "utils.h"

TimeService::TimeService(uint16_t port)
    : rpcSrv(port)
    , synchronized(false)
{        
    rpcSrv.bind("synchronize",
        [this]() 
        {
            timeType deliveryTime = getTime();
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

std::pair<timeType, timeType> TimeService::synchronize(timeType deliveryTime)
{
    timeType sendingTime = getTime();
    return { deliveryTime, sendingTime };
}

bool TimeService::isSynchronized()
{
    return synchronized;
}
