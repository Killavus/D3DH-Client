#ifndef FRAME_SYNHRONIZATION_H
#define FRAME_SYNHRONIZATION_H

#include "type_definitions.h"

class FrameSynchronizer
{
public:
    FrameSynchronizer(ClientToFramesMapping &clientToFrames);
private:
    ClientToFramesMapping &clientToFrames;
};

#endif // FRAME_SYNHRONIZATION_H
