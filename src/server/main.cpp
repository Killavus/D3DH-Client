#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "data_capture.h"
#include "utils.h"
#include "type_definitions.h"

#include "server/frame_processors.h"
#include "server/pack_of_frames_to_disk_processor.h"
#include "server/pack_of_frames_frontend_processor.h"

int main(int argc, char **argv)
{
    ArgsParser parser(argc, argv);
    Config config(parser.getOption("config_path"));

    std::string withFrontend = "YES";
    
    PackOfFramesHandler frameSynchronizer(config.maxDistBetweenFramesInBatch,
                                          config.numberOfKinects,
                                          config.minNumberOfFramesInPackageToAccept);
    Server srv(config.serverEndpoint.second, 
               config.clientsEndpoints, frameSynchronizer);
    srv.performSynchronization();

    std::shared_ptr<ChainFrameProcessor> frameProcessor(new ChainFrameProcessor(frameSynchronizer));
    std::shared_ptr<PackOfFramesProcessor> toDiskProcessor(new PackOfFramesToDiskProcessor("output"));

    if (withFrontend == "YES") {
      Frontend frontend(frameProcessor);

      std::shared_ptr<PackOfFramesProcessor> guiUpdateProcessor(new PackOfFramesFrontendProcessor(frontend));
      frameProcessor->addProcessor(guiUpdateProcessor);
      frameProcessor->addProcessor(toDiskProcessor);

      frontend.loop(); 
    } else {
      frameProcessor->addProcessor(toDiskProcessor);
      frameProcessor->processFrames();
    }

    return 0;
}
