#ifndef INC_3DHUMANCAPTURE_DATA_CAPTURE_H
#define INC_3DHUMANCAPTURE_DATA_CAPTURE_H

#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "rpc/server.h"

#include "type_definitions.h"

class Server
{
public:
    Server(int num_clients, uint16_t port, 
           std::vector<Endpoint> clientsEndpoints,
           ClientToFramesMapping &clientToFrames);
    void performSynchronization();
    bool synchronizationFinished();
    
private:
    // to be exposed via rpc
    void pushKinectData(std::string kinId, KinectData data);
    void pushHandshakeTimes(std::string kinId, Times times);

    int num_clients;
    int num_clients_registered;
    ClientToFramesMapping &clientToFrames;
    std::vector<Endpoint> clientsEndpoints;
    std::unordered_map<std::string, std::chrono::duration<float>> localtimeOffsets;
};

#endif //INC_3DHUMANCAPTURE_DATA_CAPTURE_H

