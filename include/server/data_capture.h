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
    Server(int num_clients, uint16_t port, 
           std::unordered_map<KinectId, Endpoint> clientsEndpoints,
           ClientToFramesMapping &clientToFrames);
    void performSynchronization();
    bool synchronizationFinished();
    
private:
    // to be exposed via rpc
    void pushKinectData(KinectId kinId, KinectData data);
    void pushHandshakeTimes(KinectId kinId, Times times);

    int num_clients;
    int num_clients_registered;
    ClientToFramesMapping &clientToFrames;
    std::unordered_map<KinectId, Endpoint> clientsEndpoints;
    std::unordered_map<KinectId, time_t> localtimeOffsets;
};

#endif // DATA_CAPTURE_H
