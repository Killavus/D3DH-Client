#ifndef TIMESERVICE_H
#define TIMESERVICE_H

#include <memory>

#include <rpc/server.h>

class TimeService
{
public:
    TimeService(uint16_t port);
    bool isSynchronized();

private:
    rpc::server rpcSrv;
    bool synchronized;
    
    std::pair<time_t, time_t> synchronize(time_t deliveryTime);
};

#endif // TIMESERVICE_H
