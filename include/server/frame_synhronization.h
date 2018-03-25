#ifndef INC_3DHUMANCAPTURE_FRAME_SYNHRONIZATION_H
#define INC_3DHUMANCAPTURE_FRAME_SYNHRONIZATION_H

#include "type_definitions.h"

class FrameSynchronizer
{
public:
    FrameSynchronizer(ClientToFramesMapping &clientToFrames);
private:
    ClientToFramesMapping &clientToFrames;
};

#endif //INC_3DHUMANCAPTURE_FRAME_SYNHRONIZATION_H
