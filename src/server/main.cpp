#include "data_capture.h"
#include "utils.h"

int main()
{
    Config config("../config.yaml");
    
    //tutaj bartek musisz zmienic strukture
    ClientToFramesMapping clientToFrames;
    Server srv(config.serverEndpoint.second, 
               config.clientsEndpoints, clientToFrames);
    srv.performSynchronization();
    
    return 0;
}
