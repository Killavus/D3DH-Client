#ifndef INC_3DHUMANCAPTURE_FRONTEND_H
#define INC_3DHUMANCAPTURE_FRONTEND_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "common/type_definitions.h"
#include "server/frame_processors.h"

class Frontend {
public:
  Frontend(std::shared_ptr<FrameProcessorBase> frameProcessor); 
  ~Frontend();

  void loop();
  void putData(PackOfFrames& framesPack);
private: 
  GLFWwindow *window;
  std::shared_ptr<FrameProcessorBase> frameProcessor;
};

#endif //INC_3DHUMANCAPTURE_FRONTEND_H
