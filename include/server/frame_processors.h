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
    virtual void processFrames() = 0;
    virtual ~FrameProcessorBase() = default;

protected:
    PackOfFramesHandler &frameHandler;
};

class ToFileWriter : public FrameProcessorBase
{
public:
    ToFileWriter(std::string directory, PackOfFramesHandler &frameHandler);
    void processFrames() override;
    virtual ~ToFileWriter() = default;

private:
    std::string directory;
};

class ChainFrameProcessor : public FrameProcessorBase {
public:
  ChainFrameProcessor(PackOfFramesHandler &frameHandler);
  virtual ~ChainFrameProcessor() = default;

  virtual void processFrames() override;
  void addProcessor(std::shared_ptr<PackOfFramesProcessor> processorPtr);

private:
  std::vector<std::shared_ptr<PackOfFramesProcessor>> processors;
};

#endif //INC_3DHUMANCAPTURE_FRAME_PROCESSORS_H

