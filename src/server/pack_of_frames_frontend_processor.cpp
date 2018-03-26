#include "server/pack_of_frames_frontend_processor.h"


PackOfFramesFrontendProcessor::PackOfFramesFrontendProcessor(Frontend &frontend)
    : PackOfFramesProcessor()
    , frontend(frontend)
{}

void PackOfFramesFrontendProcessor::onNewFrame(
    PackOfFrames& framesPack __attribute__((unused)), 
    int frameNo __attribute__((unused))) {}
