#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "type_definitions.h"

std::string getHostName();

struct Config
{
    Config(std::string path);

    std::unordered_map<KinectId, Endpoint> clientsEndpoints;
    Endpoint serverEndpoint;
};

#endif // UTILS_H
