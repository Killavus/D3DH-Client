#include <unistd.h>

#include "data_capture.h"
#include "utils.h"
#include "type_definitions.h"

#include "server/frame_processors.h"
#include "server/pack_of_frames_to_disk_processor.h"

int main(int argc, char **argv)
{
    ArgsParser parser(argc, argv);
    Config config(parser.getOption("config_path"));
    
    PackOfFramesHandler frameSynchronizer(config.maxDistBetweenFramesInBatch,
                                          config.numberOfKinects,
                                          config.minNumberOfFramesInPackageToAccept);
    Server srv(config.serverEndpoint.second, 
               config.clientsEndpoints, frameSynchronizer);
    srv.performSynchronization();
    std::unique_ptr<ChainFrameProcessor> frameProcessor(new ChainFrameProcessor(frameSynchronizer));
    std::shared_ptr<PackOfFramesProcessor> toDiskProcessor(new PackOfFramesToDiskProcessor("output"));

    frameProcessor->addProcessor(toDiskProcessor);
    frameProcessor->processFrames();
   
    return 0;
}
