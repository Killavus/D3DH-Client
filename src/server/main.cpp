#include "data_capture.h"
#include "utils.h"
#include "type_definitions.h"

int main()
{
    Config config("../config.yaml");
    
    PackOfFramesHandler frameSynchronizer(config.maxDistBetweenFramesInBatch);
    Server srv(config.serverEndpoint.second, 
               config.clientsEndpoints, frameSynchronizer);
    srv.performSynchronization();
    
    return 0;
}
