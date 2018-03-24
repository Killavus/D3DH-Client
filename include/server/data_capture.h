#ifndef DATA_CAPTURE_H
#define DATA_CAPTURE_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <rpc/server.h>

#include "type_definitions.h"

using timeType = std::uint64_t;

class Server
{
public:
    Server(uint16_t port, 
           std::unordered_map<KinectId, Endpoint> clientsEndpoints,
           PackOfFramesHandler &frameSynchronizer);
    void performSynchronization();
    
private:
    // to be exposed via rpc
    void pushKinectData(KinectId kinId, KinectData data);

    rpc::server rpcSrv;
    PackOfFramesHandler &frameSynchronizer;
    std::unordered_map<KinectId, Endpoint> clientsEndpoints;
    std::unordered_map<KinectId, timeType> localtimeOffsets;
};

#endif // DATA_CAPTURE_H
