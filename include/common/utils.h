#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>
#include <vector>

#include "type_definitions.h"

#ifdef NDEBUG
#define IF_DEBUG(EXP) ;
#else
#include <iostream>
#define IF_DEBUG(EXP) EXP;
#endif


std::string getHostname();
std::vector<unsigned char> copyToVector(unsigned char *img, size_t size);
timeType getTime();
std::string imgTypeToStr(ImageType type);

struct Config
{
    Config(std::string path);
    void printReadedData();

    std::unordered_map<KinectId, Endpoint> clientsEndpoints;
    std::unordered_map<KinectId, std::string> clientCalibrationPaths;

    Endpoint serverEndpoint;
    std::uint64_t maxDistBetweenFramesInBatch;
    std::size_t minNumberOfFramesInPackageToAccept;
    std::string outputDirectory;
    bool withFrontend;
    Mode mode;
    int maxNumFramesToBeSent;
    bool isPlayer;
    std::string playerPath;
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
