#ifndef FRAME_PROCESSORS_H
#define FRAME_PROCESSORS_H

#include "type_definitions.h"

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

#endif // FRAME_PROCESSORS_H

