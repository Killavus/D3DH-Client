#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

#include "type_definitions.h"

std::string getHostname();
std::vector<unsigned char> copyToVector(unsigned char *img, size_t size);

struct Config
{
    Config(std::string path);

    std::unordered_map<KinectId, Endpoint> clientsEndpoints;
    Endpoint serverEndpoint;
};

#endif // UTILS_H
