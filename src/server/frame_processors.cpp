#include <iostream>
#include <unistd.h>

#include "server/frame_processors.h"
#include "server/pack_of_frames_processor.h"

#include "utils.h"

FrameProcessorBase::FrameProcessorBase(GenericPackOfFramesHandler &frameHandler)
	: frameHandler(frameHandler)
{}

ChainFrameProcessor::ChainFrameProcessor(GenericPackOfFramesHandler &frameHandler) : FrameProcessorBase(frameHandler), frameCounter(0) {}

void ChainFrameProcessor::processFramesStep() {
  auto nextFrameMaybe = frameHandler.getNextPackOfFrames();

  if (nextFrameMaybe) {
    IF_DEBUG(std::cerr << "[ChainFrameProcessor] Processing next frame batch" << std::endl);
    ++frameCounter;
    for(auto &it : processors) {
      auto &frameData = *nextFrameMaybe;
      it->onNewFrame(frameData, frameCounter);
    }
  }
}

void ChainFrameProcessor::processFrames()
{
  while (true) { processFramesStep(); }
}

void ChainFrameProcessor::addProcessor(std::shared_ptr<PackOfFramesProcessor> processorPtr) {
  processors.push_back(processorPtr);
}
