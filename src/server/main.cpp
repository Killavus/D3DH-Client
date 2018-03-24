#include <unistd.h>

#include "data_capture.h"
#include "frame_processors.h"
#include "utils.h"
#include "type_definitions.h"

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
    std::unique_ptr<FrameProcessorBase> frameProcessor(new ToFileWriter("/home/kin3d-1/Documents/D3DH-Client/build/output", frameSynchronizer));
    frameProcessor->processFrames();
    /*while (1)
    {
        sleep(1000);
    }*/
    
    return 0;
}
