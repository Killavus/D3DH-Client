#include <ctime>
#include <cmath>

#include <rpc/client.h>
#include <rpc/server.h>

#include "data_capture.h"
#include "type_definitions.h"

Server::Server(uint16_t port, 
    std::unordered_map<KinectId, Endpoint> clientsEndpoints,
    ClientToFramesMapping &clientToFrames)
    : clientToFrames(clientToFrames)
    , clientsEndpoints(std::move(clientsEndpoints)) 
    {
        rpc::server srv(port);
        
        srv.bind("pushKinectData", 
            [this](KinectId kinId, RawImage rgb, size_t rgbW, 
                   RawImage depth, size_t depthW,
                   RawImage ir, size_t irW, time_t timestamp)
            {
                auto timeOffset = localtimeOffsets[kinId];
                pushKinectData(kinId, 
                    KinectData(std::move(rgb), rgbW,
                        std::move(depth), depthW,
                        std::move(ir), irW, timestamp + timeOffset));
            });

        srv.async_run();
    }
    
void Server::performSynchronization()
{
    for (const auto &entry : clientsEndpoints)
    {
        const auto &kinId = entry.first;
        const auto &endpoint = entry.second;
        rpc::client client(endpoint.first, endpoint.second);

        time_t sendingTime = std::time(nullptr);
        // <client delivery time, client sending time>
        auto result = client.call("synchronize").as<std::pair<time_t, time_t>>();
        time_t deliveryTime = std::time(nullptr);

        time_t propagationTime = (result.first - sendingTime +
            deliveryTime - result.second) / 2;
        time_t timeOffset = sendingTime + propagationTime - result.first;

        localtimeOffsets[kinId] = timeOffset;
    }
    
    for (const auto &entry : clientsEndpoints)
    {
        const auto &kinId = entry.first;
        const auto &endpoint = entry.second;
        rpc::client client(endpoint.first, endpoint.second);
        client.call("syncFinished");
    }
}

void Server::pushKinectData(KinectId kinId, KinectData data)
{
    clientToFrames.putFrame(kinId, std::move(data));
}
