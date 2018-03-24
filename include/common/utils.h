#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>
#include <vector>

#include "type_definitions.h"

#ifdef NDEBUG
#include <iostream>
#define IF_DEBUG(EXP) ;
#else
#define IF_DEBUG(EXP) EXP;
#endif

using timeType = std::uint64_t;

std::string getHostname();
std::vector<unsigned char> copyToVector(unsigned char *img, size_t size);
timeType getTime();

struct Config
{
    Config(std::string path);

    std::unordered_map<KinectId, Endpoint> clientsEndpoints;
    Endpoint serverEndpoint;
    std::uint64_t maxDistBetweenFramesInBatch;
    std::uint8_t numberOfKinects;
    std::size_t minNumberOfFramesInPackageToAccept;
};

class ArgsParser
{
public:
    ArgsParser(int argc, char **argv);
    std::string getOption(std::string optKey);
    
private:
    std::unordered_map<std::string, std::string> options;
};


#endif // UTILS_H
