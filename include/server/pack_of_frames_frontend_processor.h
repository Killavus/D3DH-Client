#ifndef INC_3DHUMANSCAPTURE_PACK_OF_FRAMES_FRONTEND_PROCESSOR_H
#define INC_3DHUMANSCAPTURE_PACK_OF_FRAMES_FRONTEND_PROCESSOR_H
#include "server/frontend.h"
#include "server/pack_of_frames_processor.h"


class PackOfFramesFrontendProcessor : public PackOfFramesProcessor {
public:
  PackOfFramesFrontendProcessor(Frontend &frontend);
  virtual ~PackOfFramesFrontendProcessor() = default;
  void onNewFrame(PackOfFrames& framesPack, int frameNo) override;

private:
  Frontend &frontend;
};
#endif //INC_3DHUMANSCAPTURE_PACK_OF_FRAMES_FRONTEND_PROCESSOR_H
