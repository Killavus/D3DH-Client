#ifndef INC_3DHUMANCAPTURE_FRONTEND_H
#define INC_3DHUMANCAPTURE_FRONTEND_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <unordered_map>
#include <Phelsuma/include/ShaderProgram.h>

#include "common/type_definitions.h"
#include "server/frame_processors.h"

enum ViewType
{
  VIEW_RGB = 0,
  VIEW_IR,
  VIEW_DEPTH,
  VIEW_PCLOUD,
  VIEW_UNKNOWN
};

struct KinectOGLData
{
  KinectId id;
  bool rgbReady, irReady, depthReady;
  GLuint rgbTex, irTex, depthTex;

  KinectOGLData(KinectId id);
  void reset();
  void setFrame(const KinectData &data);
  void draw();
};

class Frontend
{
public:
  Frontend(std::shared_ptr<FrameProcessorBase> frameProcessor);
  ~Frontend();

  void loop();
  void putData(PackOfFrames &framesPack);

private:
  GLuint screenVao;

  GLFWwindow *window;
  std::shared_ptr<FrameProcessorBase> frameProcessor;

  ViewType currentViewType;
  KinectId currentDeviceId;
  std::unordered_map<KinectId, KinectOGLData> oglData;

  void draw();

  static void initShaders();
  static bool shadersInitialized;
  static ShaderProgram previewRGB, previewGray;
};

#endif //INC_3DHUMANCAPTURE_FRONTEND_H
