#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "server/frontend.h"
#include "server/frame_processors.h"
#include "server/point_cloud_factory.h"

#include <Phelsuma/include/ShaderProgramLinker.h>
#include <Phelsuma/include/ShaderCompiler.h>
#include <Phelsuma/include/Shader.h>
#include <Phelsuma/include/examples/Utils.h>

bool Frontend::shadersInitialized = false;
ShaderProgram Frontend::previewRGB("");
ShaderProgram Frontend::previewGray("");

KinectOGLData::KinectOGLData(KinectId id) : id(id)
{
  glGenTextures(1, &rgbTex);
  glGenTextures(1, &irTex);
  glGenTextures(1, &depthTex);
  glGenVertexArrays(1, &pcVao);

  GLuint pcVbo;
  glGenBuffers(1, &pcVbo);
  glBindVertexArray(pcVao);
  glBindBuffer(GL_ARRAY_BUFFER, pcVbo);

  std::vector<float> pcloud(PointCloud::cloudSize() * 3, 0.0f);

  glBufferData(GL_ARRAY_BUFFER, PointCloud::cloudSize() * 3 * sizeof(float), pcloud.data(), GL_STATIC_DRAW);
	
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  glEnableVertexAttribArray(0);

  reset();
}

void KinectOGLData::setPointCloud(const PointCloud& cloud) {
  glBindVertexArray(pcVao);
  glBufferData(GL_ARRAY_BUFFER, clouds.points.size() * sizeof(float), cloud.points.data(), GL_STATIC_DRAW);
}

void KinectOGLData::reset()
{
  rgbReady = false;
  irReady = false;
  depthReady = false;
}

void KinectOGLData::draw(ViewType type)
{
  glActiveTexture(GL_TEXTURE0);
  switch (type)
  {
  case VIEW_DEPTH:
    if (!depthReady)
    {
      return;
    }
    glBindTexture(GL_TEXTURE_2D, depthTex);
    break;
  case VIEW_RGB:
    if (!rgbReady)
    {
      return;
    }
    glBindTexture(GL_TEXTURE_2D, rgbTex);
    break;
  case VIEW_IR:
    if (!irReady)
    {
      return;
    }
    glBindTexture(GL_TEXTURE_2D, irTex);
    break;
  }

  std::cout << depthTex << " " << rgbTex << " " << irTex << std::endl;
 
  std::cout << "glDrawElements" << std::endl;
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void KinectOGLData::setFrame(const KinectData &data)
{
  for (auto &frame : data.images)
  {
    GLint internalFormat, format;
    GLenum type;

    glActiveTexture(GL_TEXTURE0);

    switch (frame.first)
    {
    case ImageType::RGB:
      glBindTexture(GL_TEXTURE_2D, rgbTex);
      rgbReady = true;
      internalFormat = GL_RGBA;
      format = GL_BGRA;
      type = GL_UNSIGNED_BYTE;
      break;
    case ImageType::IR:
      glBindTexture(GL_TEXTURE_2D, irTex);
      irReady = true;
      internalFormat = GL_R32F;
      format = GL_RED;
      type = GL_FLOAT;
      break;
    case ImageType::DEPTH:
      std::cout << "[setFrame] Setting active texture to depth..." << std::endl;
      glBindTexture(GL_TEXTURE_2D, depthTex);
      depthReady = true;
      internalFormat = GL_R32F;
      format = GL_RED;
      type = GL_FLOAT;
      break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glTexImage2D(
      GL_TEXTURE_2D,
      0, internalFormat, 
      frame.second.width, frame.second.height, 
      0, format, type,
      frame.second.img.data()
    );
  }
}

Frontend::Frontend(std::shared_ptr<FrameProcessorBase> frameProcessor, const CameraCalibrationsMap& calibrations) : frameProcessor(frameProcessor), currentViewType(VIEW_UNKNOWN), calibrations(calibrations)
{
  std::srand(std::time(NULL));
  if (glfwInit() == GL_FALSE) {
    std::cerr << "Failed to initialize GLFW." << std::endl;
    terminate(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef PHELSUMA_DEBUG_OGL
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif // PHELSUMA_DEBUG_OGL

  window = glfwCreateWindow(1024, 768, "Viewer", NULL, NULL);

  if (window == nullptr) {
    std::cerr << "Failed to initialize GLFW window." << std::endl;
    terminate(EXIT_FAILURE);
  }
  
  std::cout << window << std::endl;
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize OpenGL context." << std::endl;
    terminate(EXIT_FAILURE);
  }
#ifdef PHELSUMA_DEBUG_OGL
  glDebugMessageCallback(glDebugOutput, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif // PHELSUMA_DEBUG_OGL

  glViewport(0, 0, 1024, 768);

  GLuint sqVbo, sqEbo;
  GLfloat sq[] = {
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f
  };

  GLuint sqElems[] = {0, 1, 2, 1, 2, 3};

  glGenVertexArrays(1, &screenVao);
  glGenBuffers(1, &sqVbo);
  glGenBuffers(1, &sqEbo);

  glBindVertexArray(screenVao);
  glBindBuffer(GL_ARRAY_BUFFER, sqVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(sq), sq, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sqEbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sqElems), sqElems, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  Frontend::initShaders();
}

void Frontend::loop()
{
  while (!glfwWindowShouldClose(window))
  {
    frameProcessor->processFramesStep();

    glClearColor(0.3, 0.2, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void Frontend::draw()
{
  switch (currentViewType)
  {
  case VIEW_RGB:
    previewRGB.use();
    break;
  case VIEW_IR:
    previewGray.use();
    break;
  case VIEW_DEPTH:
    previewGray.use();
    std::cout << "Frontend::draw: setting previewGray program." << std::endl;
    break;
  case VIEW_PCLOUD:
    // TODO: Point cloud retrieval.
    return;
    break;
  default:
    return;
  }

  if (currentViewType == VIEW_RGB || currentViewType == VIEW_IR || currentViewType == VIEW_DEPTH)
  {
    if (currentDeviceId != "")
    {
      glBindVertexArray(screenVao);
      auto it = oglData.find(currentDeviceId);
      it->second.draw(currentViewType);
    }
  }
  else if (currentViewType == VIEW_PCLOUD)
  {
    // TODO: Point cloud retrieval.
    return;
  }
}

void Frontend::putData(PackOfFrames &framesPack)
{

  PointCloudFactory pcFactory(calibrations);
  auto cloudSet = pcFactory.fromPack(framesPack);

  for (auto &pack : framesPack)
  {
    std::unordered_map<KinectId, KinectOGLData>::iterator kinectOgl = oglData.find(pack.first);
    if (kinectOgl == oglData.end()) {
      oglData.insert({ pack.first, KinectOGLData(pack.first) });
      kinectOgl = oglData.find(pack.first);
    }

    kinectOgl->second.setFrame(pack.second);

    if (cloudSet.find(pack.first) !== cloudSet.end()) {
      kinectOgl->second.setPointCloud(cloudSet[pack.first]);
    }
  }

  /* When we get the first data, we want to display _something on the screen_. */
  if (currentViewType == VIEW_UNKNOWN)
  {
    currentViewType = VIEW_DEPTH;
    currentDeviceId = oglData.begin()->first;
  }
}

void Frontend::initShaders()
{
  if (!Frontend::shadersInitialized)
  {
    Shader previewVertex = shaderFromFile("shaders/preview.vs", GL_VERTEX_SHADER);
    Shader previewRGBFrag = shaderFromFile("shaders/preview_rgb.fs", GL_FRAGMENT_SHADER);
    Shader previewGrayFrag = shaderFromFile("shaders/preview_gray.fs", GL_FRAGMENT_SHADER);

    std::vector<Shader> previewRGBShaders{previewVertex, previewRGBFrag};
    std::vector<Shader> previewGrayShaders{previewVertex, previewGrayFrag};

    Frontend::previewRGB = makeProgram(previewRGBShaders.begin(), previewRGBShaders.end());
    Frontend::previewGray = makeProgram(previewGrayShaders.begin(), previewGrayShaders.end());
    Frontend::shadersInitialized = true;
  }
}

Frontend::~Frontend()
{
  glfwTerminate();
}
