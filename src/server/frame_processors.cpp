#include <iostream>
#include <unistd.h>

#include "server/frame_processors.h"
#include "server/pack_of_frames_processor.h"

#include "utils.h"

FrameProcessorBase::FrameProcessorBase(PackOfFramesHandler &frameHandler)
	: frameHandler(frameHandler)
{}

ChainFrameProcessor::ChainFrameProcessor(PackOfFramesHandler &frameHandler) : FrameProcessorBase(frameHandler) {}

void ChainFrameProcessor::processFrames()
{
  int frameCounter = 0;
  while (true) {
    auto nextFrameMaybe = frameHandler.getNextPackOfFrames();

    if (!nextFrameMaybe) {
      IF_DEBUG(std::cerr << "[ChainFrameProcessor] SLEEPING... Waiting for next frame batch" << std::endl);
      sleep(1);
    }
    else {
			IF_DEBUG(std::cerr << "[ChainFrameProcessor] Processing next frame batch" << std::endl);

      for(auto &it : processors) {
        auto &frameData = *nextFrameMaybe;
        it->onNewFrame(frameData, frameCounter);
      }
    }

    ++frameCounter;
  }
}

void ChainFrameProcessor::addProcessor(std::shared_ptr<PackOfFramesProcessor> processorPtr) {
  processors.push_back(processorPtr);
}
