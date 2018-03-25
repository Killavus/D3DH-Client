#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "server/frontend.h"

Frontend::Frontend(std::shared_ptr<FrameProcessorBase> frameProcessor): frameProcessor(frameProcessor) {
  if (glfwInit() == GLFW_FALSE) {
    std::cerr << "Failed to initialize GLFW." << std::endl;
    std::exit(EXIT_FAILURE); 
  }
 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1024, 768, "Viewer", NULL, NULL);
  if (window == nullptr) {
    std::cerr << "Failed to initialize GLFW window." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cerr << "Failed to initialize OpenGL." << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

void Frontend::loop() {
  while (!glfwWindowShouldClose(window)) {
    frameProcessor->processFramesStep();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void Frontend::putData(PackOfFrames& framesPack) {}

Frontend::~Frontend() {
  glfwTerminate();
}
