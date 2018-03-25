#ifndef INC_3DHUMANCAPTURE_PACK_OF_FRAMES_TO_DISK_PROCESSOR_H
#define INC_3DHUMANCAPTURE_PACK_OF_FRAMES_TO_DISK_PROCESSOR_H
#include <string>

#include "server/pack_of_frames_processor.h"

class PackOfFramesToDiskProcessor : public PackOfFramesProcessor {
public:
  PackOfFramesToDiskProcessor(const std::string& directoryName);
  void onNewFrame(PackOfFrames& framePack, int frameNo) override;
  virtual ~PackOfFramesToDiskProcessor() = default;
private:
  std::string directory;
};
#endif //INC_3DHUMANCAPTURE_PACK_OF_FRAMES_TO_DISK_PROCESSOR_H
