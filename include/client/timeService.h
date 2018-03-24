#ifndef TIMESERVICE_H
#define TIMESERVICE_H

#include <memory>

#include <rpc/server.h>

using timeType = std::uint64_t;

class TimeService
{
public:
    TimeService(uint16_t port);
    bool isSynchronized();

private:
    rpc::server rpcSrv;
    bool synchronized;
    
    std::pair<timeType, timeType> synchronize(timeType deliveryTime);
};

#endif // TIMESERVICE_H
