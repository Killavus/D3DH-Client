#include <unistd.h>

#include "data_capture.h"
#include "utils.h"
#include "type_definitions.h"

int main(int argc, char **argv)
{
    ArgsParser parser(argc, argv);
    Config config(parser.getOption("config_path"));
    
    PackOfFramesHandler frameSynchronizer(config.maxDistBetweenFramesInBatch);
    Server srv(config.serverEndpoint.second, 
               config.clientsEndpoints, frameSynchronizer);
    srv.performSynchronization();
    
    while (1)
    {
        sleep(1000);
    }
    
    return 0;
}
