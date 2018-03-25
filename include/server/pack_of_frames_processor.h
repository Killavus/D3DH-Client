#ifndef INC_3DHUMANCAPTURE_PACK_OF_FRAMES_PROCESSOR_H
#define INC_3DHUMANCAPTURE_PACK_OF_FRAMES_PROCESSOR_H
#include "common/type_definitions.h"

class PackOfFramesProcessor {
public:
  virtual void onNewFrame(PackOfFrames& framePack, int frameNo) = 0;
  virtual ~PackOfFramesProcessor() = default;
};


#endif //INC_3DHUMANCAPTURE_PACK_OF_FRAMES_PROCESSOR_H
