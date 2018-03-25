#ifndef INC_3DHUMANCAPTURE_FRAME_PROCESSORS_H
#define INC_3DHUMANCAPTURE_FRAME_PROCESSORS_H

#include <memory>
#include <vector>
#include "type_definitions.h"

class PackOfFramesProcessor;

class FrameProcessorBase
{
public:
    FrameProcessorBase(PackOfFramesHandler &frameHandler);
    virtual void processFramesStep() = 0;
    virtual void processFrames() = 0;
    virtual ~FrameProcessorBase() = default;

protected:
    PackOfFramesHandler &frameHandler;
};

class ChainFrameProcessor : public FrameProcessorBase {
public:
  ChainFrameProcessor(PackOfFramesHandler &frameHandler);
  virtual ~ChainFrameProcessor() = default;

  void processFramesStep() override;
  void processFrames() override;
  void addProcessor(std::shared_ptr<PackOfFramesProcessor> processorPtr);

private:
  int frameCounter;
  std::vector<std::shared_ptr<PackOfFramesProcessor>> processors;
};

#endif //INC_3DHUMANCAPTURE_FRAME_PROCESSORS_H

