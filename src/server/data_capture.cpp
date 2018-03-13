#include <ctime>
#include <cmath>

#include <rpc/client.h>
#include <rpc/server.h>

#include "data_capture.h"
#include "type_definitions.h"

Server::Server(int num_clients, uint16_t port, 
    std::unordered_map<KinectId, Endpoint> clientsEndpoints,
    ClientToFramesMapping &clientToFrames)
    : num_clients(num_clients)
    , num_clients_registered(0)
    , clientToFrames(clientToFrames)
    , clientsEndpoints(std::move(clientsEndpoints)) 
    {
        rpc::server srv(port);
        
        srv.bind("pushKinectData", 
            [this](KinectId kinId, RawImage rgb, RawImage depth, 
                   RawImage ir, size_t width, time_t timestamp)
            {
                pushKinectData(kinId, 
                    KinectData(std::move(rgb), std::move(depth), std::move(ir), 
                        width, ir.size() / width, timestamp));
            });
        
        srv.bind("pushHandshakeTimes",
            [this](KinectId kinId, Times times)
            {
                pushHandshakeTimes(kinId, times);
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
}

bool Server::synchronizationFinished()
{
    return num_clients_registered == num_clients;
}

void Server::pushKinectData(KinectId kinId, KinectData data)
{
    clientToFrames.putFrame(kinId, std::move(data));
}

void Server::pushHandshakeTimes(KinectId kinId, Times times)
{
    // to be implemented
}
