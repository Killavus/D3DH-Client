#include <climits>
#include <cstring>
#include <unistd.h>

#include <yaml-cpp/yaml.h>

#include "utils.h"

std::string getHostname()
{
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    return hostname;
}

std::vector<unsigned char> copyToVector(unsigned char *img, size_t size)
{
    std::vector<unsigned char> result;
    result.resize(size);
    std::memcpy(&result[0], img, size * sizeof(unsigned char));
    return result;
}

Config::Config(std::string path)
{
    YAML::Node config = YAML::LoadFile(path);
    
    auto clients = config["Clients"];
    for (auto it = clients.begin(); it != clients.end(); ++it)
    {
        auto hostname = (*it)["Hostname"].as<std::string>();
        auto ip = (*it)["Ip"].as<std::string>();
        auto port = (*it)["Port"].as<uint16_t>();
        
        clientsEndpoints[hostname] = std::make_pair(ip, port);
    }
    
    auto ip = config["Server"]["Ip"].as<std::string>();
    auto port = config["Server"]["Port"].as<uint16_t>();
    serverEndpoint = std::make_pair(ip, port);

    maxDistBetweenFramesInBatch = config["MaxDistBetweenFramesInBatch"].as<std::uint64_t>();
}
