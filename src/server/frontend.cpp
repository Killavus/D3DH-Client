#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "server/frontend.h"
#include "server/frame_processors.h"

#include <Phelsuma/include/ShaderProgramLinker.h>
#include <Phelsuma/include/ShaderCompiler.h>
#include <Phelsuma/include/Shader.h>

bool Frontend::shadersInitialized = false;
ShaderProgram Frontend::previewRGB("");
ShaderProgram Frontend::previewGray("");

static Shader shaderFromFile(const std::string &path, GLenum type)
{
  ShaderCompiler compiler = ShaderCompiler::fromFile(path, type);
  Shader shader = compiler.compile();

  GLint success;
  
  if (shader.invalid())
  {
    std::cerr << path << " -- "
              << "Shader compilation error!\n\n"
              << shader.errorMessage() << std::endl;
    std::exit(EXIT_FAILURE);
  }

  return shader;
}

static ShaderProgram makeProgram(
    std::vector<Shader>::iterator it,
    std::vector<Shader>::iterator end)
{
  ShaderProgramLinker linker;
  for (; it != end; ++it)
  {
    linker.attachShader(*it);
  }

  ShaderProgram program = linker.link();

  if (program.invalid())
  {
    std::cerr << " -- "
              << "Shader program link error!\n\n"
              << program.errorMessage() << std::endl;
    std::exit(EXIT_FAILURE);
  }

  return program;
}

KinectOGLData::KinectOGLData(KinectId id) : id(id)
{
  glGenTextures(1, &rgbTex);
  glGenTextures(1, &irTex);
  glGenTextures(1, &depthTex);

  reset();
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
  switch(type) {
    case VIEW_DEPTH:
      if(!depthReady) { return; }
      glBindTexture(GL_TEXTURE_RECTANGLE, depthTex);
      break;
    case VIEW_RGB:
      if(!rgbReady) { return; }
      glBindTexture(GL_TEXTURE_RECTANGLE, rgbTex);
      break;
    case VIEW_IR:
      if(!irReady) { return; }
      glBindTexture(GL_TEXTURE_RECTANGLE, irTex);
      break;
  }

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
      glBindTexture(GL_TEXTURE_RECTANGLE, rgbTex);
      rgbReady = true;
      internalFormat = GL_RGBA;
      format = GL_BGRA;
      type = GL_UNSIGNED_BYTE;
      break;
    case ImageType::IR:
      glBindTexture(GL_TEXTURE_RECTANGLE, irTex);
      irReady = true;
      internalFormat = GL_R32F;
      format = GL_RED;
      type = GL_FLOAT;
      break;
    case ImageType::DEPTH:
      glBindTexture(GL_TEXTURE_RECTANGLE, depthTex);
      depthReady = true;
      internalFormat = GL_R32F;
      format = GL_RED;
      type = GL_FLOAT;
      break;
    }

    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormat, frame.second.width, frame.second.height, 0, format, type, frame.second.img.data());
  }
}

Frontend::Frontend(std::shared_ptr<FrameProcessorBase> frameProcessor) : frameProcessor(frameProcessor), currentViewType(VIEW_UNKNOWN)
{
  if (!glfwInit())
  {
    std::cerr << "Failed to initialize GLFW." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1024, 768, "Viewer", NULL, NULL);
  if (window == nullptr)
  {
    std::cerr << "Failed to initialize GLFW window." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cerr << "Failed to initialize OpenGL." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  glViewport(0, 0, 1024, 768);

  GLuint sqVbo, sqEbo;
  GLfloat sq[] = {
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f};

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
    glClearColor(0.0, 0.0, 0.0, 1.0);
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
    break;
  case VIEW_PCLOUD:
    // TODO: Point cloud retrieval.
    return;
    break;
  }

  if (currentViewType == VIEW_RGB || currentViewType == VIEW_IR || currentViewType == VIEW_DEPTH) {
    if (currentDeviceId != "") {
      glBindVertexArray(screenVao);
      auto it = oglData.find(currentDeviceId);
      it->second.draw(currentViewType);
    }
  } else if(currentViewType == VIEW_PCLOUD) {
    // TODO: Point cloud retrieval.
    return;
  }
}

void Frontend::putData(PackOfFrames &framesPack)
{
  for (auto &pack : framesPack)
  {
    std::unordered_map<KinectId, KinectOGLData>::iterator kinectOgl = oglData.emplace(
                                                                                 std::make_pair(pack.first, KinectOGLData(pack.first)))
                                                                          .first;

    kinectOgl->second.setFrame(pack.second);
  }

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
