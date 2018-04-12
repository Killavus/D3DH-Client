#ifndef INC_3DHUMANCAPTURE_PACK_OF_FRAMES_AS_BINARY_PROCESSOR_H
#define INC_3DHUMANCAPTURE_PACK_OF_FRAMES_AS_BINARY_PROCESSOR_H

#include <string>

#include "server/pack_of_frames_processor.h"

class PackOfFramesAsBinaryProcessor : public PackOfFramesProcessor {
public:
  PackOfFramesAsBinaryProcessor(const std::string& directoryName);
  void onNewFrame(PackOfFrames& framePack, int frameNo) override;
  virtual ~PackOfFramesAsBinaryProcessor() = default;
private:
  std::string directory;
};
#endif //INC_3DHUMANCAPTURE_PACK_OF_FRAMES_AS_BINARY_PROCESSOR_H

