#include <climits>
#include <unistd.h>

#include <yaml-cpp/yaml.h>

#include "utils.h"

std::string getHostname()
{
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    return hostname;
}

Config::Config(std::string path)
{
    YAML::Node config = YAML::LoadFile(path);
    
    for (auto it = config["Clients"].begin(); it != config.end(); ++it)
    {
        auto hostname = (*it)["Hostname"].as<std::string>();
        auto ip = (*it)["Ip"].as<std::string>();
        auto port = (*it)["Port"].as<int>();
        
        clientsEndpoints[hostname] = std::make_pair(ip, port);
    }
    
    auto ip = config["Server"]["Ip"].as<std::string>();
    auto port = config["Server"]["Port"].as<int>();
    serverEndpoint = std::make_pair(ip, port);
}
