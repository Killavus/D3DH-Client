#include <climits>
#include <cstring>
#include <iostream>
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

ArgsParser::ArgsParser(int argc, char ** argv)
{
    for (int i = 0; i < argc; ++i)
    {
        auto option = std::string(argv[i]);
        auto pos = option.find_first_of('=');
        auto optionName = option.substr(0, pos);
        auto optionVal = option.substr(pos + 1);
        
        options[optionName] = optionVal;
    }
}

std::string ArgsParser::getOption(std::string optKey)
{
    auto it = options.find(optKey);
    if (it != options.end())
        return it->second;
    
    std::cerr << "Option " << optKey << " wasn't provided" << std::endl;
    std::cerr << "Usage: " << optKey << "=val" << std::endl;
    exit(1);
}

