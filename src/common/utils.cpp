#include <climits>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include <yaml-cpp/yaml.h>

#include "type_definitions.h"
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

timeType getTime()
{
    return std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string imgTypeToStr(ImageType type)
{
    switch (type)
    {
        case ImageType::RGB:
            return "rgb";
        case ImageType::DEPTH:
            return "depth";
        default:
            return "ir";
    };
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
    withFrontend = config["Server"]["WithFrontend"].as<bool>();

    auto calibrations = config["Server"]["CameraCalibrations"];
    for(auto it = calibrations.begin(); it != calibrations.end(); ++it) {
        auto hostname = (*it)["Hostname"].as<std::string>();
        auto path = (*it)["Path"].as<std::string>();

        clientCalibrationPaths[hostname] = path;
    }

    maxDistBetweenFramesInBatch =
        config["MaxDistBetweenFramesInBatch"].as<std::uint64_t>();
    minNumberOfFramesInPackageToAccept =
        config["MinNumberOfFramesInPackageToAccept"].as<std::size_t>();

    outputDirectory = config["OutputDirectory"].as<std::string>();
    mode = static_cast<Mode>(config["Mode"].as<int>());
    maxNumFramesToBeSent = config["MaxNumFramesToBeSent"].as<int>();
    isPlayer = config["Server"]["IsPlayer"].as<bool>();
    playerPath = config["Server"]["PlayerDirectory"].as<std::string>();

    printReadedData();
}

void Config::printReadedData()
{
    std::cerr << "CLIENTS" << std::endl;
    for (auto &client : clientsEndpoints)
    {
        std::cerr << client.first << " " << client.second.first
            << " " << client.second.second << std::endl << std::endl;
    }

    std::cerr << "SERVER" << std::endl;
    std::cerr << serverEndpoint.first  << " " << serverEndpoint.second
        << std::endl << std::endl;

    std::cerr << "WithFrontend: " << withFrontend << std::endl;
    std::cerr << "MaxDistBetweenFramesInBatch: "
        << maxDistBetweenFramesInBatch << std::endl;
    std::cerr << "MinNumberOfFramesInPackageToAccept: "
        << minNumberOfFramesInPackageToAccept << std::endl;
    std::cerr << "OutputDirectory: " << outputDirectory << std::endl;
    std::cerr << "Mode: " << static_cast<int>(mode) << std::endl;
    std::cerr << "MaxNumFramesToBeSent: " << maxNumFramesToBeSent << std::endl;
    std::cerr << "-----------------------------------------------"
        << std::endl << std::endl;
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

