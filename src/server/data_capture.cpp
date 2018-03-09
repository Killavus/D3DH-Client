#include "data_capture.h"

Server::Server(int num_clients, uint16_t port, 
    std::vector<Endpoint> clientsEndpoints,
    ClientToFramesMapping &clientToFrames)
    : num_clients(num_clients)
    , num_clients_registered(0)
    , clientToFrames(clientToFrames)
    , clientsEndpoints(std::move(clientsEndpoints)) 
    {       
    }
    
void Server::performSynchronization()
{
    // to be implemented
}

bool Server::synchronizationFinished()
{
    return num_clients_registered == num_clients;
}

void Server::pushKinectData(std::string kinId, KinectData data)
{
    clientToFrames.putFrame(kinId, std::move(data));
}

void Server::pushHandshakeTimes(std::string kinId, Times times)
{
    // to be implemented
}
