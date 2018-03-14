#ifndef DATA_CAPTURE_H
#define DATA_CAPTURE_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "rpc/server.h"

#include "type_definitions.h"

class Server
{
public:
    Server(uint16_t port, 
           std::unordered_map<KinectId, Endpoint> clientsEndpoints,
           ClientToFramesMapping &clientToFrames);
    void performSynchronization();
    
private:
    // to be exposed via rpc
    void pushKinectData(KinectId kinId, KinectData data);

    ClientToFramesMapping &clientToFrames;
    std::unordered_map<KinectId, Endpoint> clientsEndpoints;
    std::unordered_map<KinectId, time_t> localtimeOffsets;
};

#endif // DATA_CAPTURE_H
