#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "data_capture.h"
#include "utils.h"
#include "type_definitions.h"

#include "server/frame_processors.h"
#include "server/pack_of_frames_as_binary_processor.h"
#include "server/pack_of_frames_to_disk_processor.h"
#include "server/pack_of_frames_frontend_processor.h"

int main(int argc, char **argv)
{
    ArgsParser parser(argc, argv);
    Config config(parser.getOption("config_path"));

    PackOfFramesHandler frameSynchronizer(config.maxDistBetweenFramesInBatch,
                                          config.clientsEndpoints.size(),
                                          config.minNumberOfFramesInPackageToAccept,
                                          getTime());
    Server srv(config.serverEndpoint.second,
               config.clientsEndpoints, frameSynchronizer);
    srv.performSynchronization();

    auto frameProcessor =
        std::make_shared<ChainFrameProcessor>(frameSynchronizer);
    auto toDiskProcessor =
        std::make_shared<PackOfFramesToDiskProcessor>(config.outputDirectory);
    auto asBinaryProcessor =
        std::make_shared<PackOfFramesAsBinaryProcessor>(config.outputDirectory);    

    if (config.withFrontend)
    {
        Frontend frontend(frameProcessor);

        auto guiUpdateProcessor =
            std::make_shared<PackOfFramesFrontendProcessor>(frontend);
        frameProcessor->addProcessor(guiUpdateProcessor);
        frameProcessor->addProcessor(toDiskProcessor);

        frontend.loop();
    }
    else
    {
        frameProcessor->addProcessor(toDiskProcessor);
        frameProcessor->addProcessor(asBinaryProcessor);
        frameProcessor->processFrames();
    }

    return 0;
}
