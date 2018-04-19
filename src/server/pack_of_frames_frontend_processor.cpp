#include <iostream>

#include "server/pack_of_frames_frontend_processor.h"


PackOfFramesFrontendProcessor::PackOfFramesFrontendProcessor(Frontend &frontend)
    : PackOfFramesProcessor()
    , frontend(frontend)
{}

void PackOfFramesFrontendProcessor::onNewFrame(
    PackOfFrames& framesPack,
    int frameNo) {
    std::cout << "[Frontend Processor] Processing data for frontend..." << std::endl;
    frontend.putData(framesPack);
}
