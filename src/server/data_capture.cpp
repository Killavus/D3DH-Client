#include <iostream>
#include <ctime>
#include <cmath>

#include <rpc/client.h>
#include <rpc/server.h>

#include "data_capture.h"
#include "type_definitions.h"
#include "utils.h"

Server::Server(uint16_t port, 
    std::unordered_map<KinectId, Endpoint> clientsEndpoints,
    PackOfFramesHandler &frameSynchronizer)
    : rpcSrv(port)
    , frameSynchronizer(frameSynchronizer)
    , clientsEndpoints(std::move(clientsEndpoints)) 
    {   
        rpcSrv.bind("pushKinectData", 
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

        rpcSrv.async_run();
    }
    
void Server::performSynchronization()
{
    int numOfCalls = 100;
    for (const auto &entry : clientsEndpoints)
    {
        const auto &kinId = entry.first;
        const auto &endpoint = entry.second;
        rpc::client client(endpoint.first, endpoint.second);
        
        IF_DEBUG(std::cerr << "Connecting to client" 
            << endpoint.first << " " << endpoint.second << std::endl);
        
        for (int i = 0; i < numOfCalls; ++i)
        {
            time_t sendingTime = std::time(nullptr);
            // result = <client delivery time, client sending time>
            auto result = client.call("synchronize").as<std::pair<time_t, time_t>>();
            time_t deliveryTime = std::time(nullptr);

            time_t propagationTime = (result.first - sendingTime +
                deliveryTime - result.second) / 2;
            time_t timeOffset = sendingTime + propagationTime - result.first;
            
            auto it = localtimeOffsets.find(kinId);
            if (it != localtimeOffsets.end())
                it->second += timeOffset;
            else
                localtimeOffsets[kinId] = 0;
        }
        
        for (auto &timeOffsetEntry : localtimeOffsets)
        {
            timeOffsetEntry.second /= numOfCalls;
            IF_DEBUG(std::cerr << "Timeline offset for " 
                << timeOffsetEntry.first << ": "
                << timeOffsetEntry.second << std::endl);
        }
    }
    
    for (const auto &entry : clientsEndpoints)
    {
        const auto &endpoint = entry.second;
        rpc::client client(endpoint.first, endpoint.second);
        client.call("syncFinished");
    }

    IF_DEBUG(std::cerr << "Sync finished" << std::endl);
}

void Server::pushKinectData(KinectId kinId, KinectData data)
{
    IF_DEBUG(
        static int counter = 0;
        if (counter % 30 == 0)
        {
            ++counter; 
            std::cerr << "Frame arrived" << std::endl;
        }
    );
    
    frameSynchronizer.putFrame(kinId, std::move(data));
}
