#include <iostream>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/config.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/frame_listener_impl.h>
#include <assert.h>

#include "Camera.h"

const int frame_types = libfreenect2::Frame::Color | 
  libfreenect2::Frame::Ir | 
  libfreenect2::Frame::Depth;

Camera::Camera(libfreenect2::Freenect2 &freenect, int idx) : listener(frame_types) {
  #ifdef LIBFREENECT2_WITH_CUDA_SUPPORT
    libfreenect2::PacketPipeline *pipeline = new libfreenect2::CudaPacketPipeline();
  #else
    libfreenect2::PacketPipeline *pipeline = new libfreenect2::OpenGLPacketPipeline();
  #endif

  device = freenect.openDevice(idx, pipeline);
  assert(device != NULL);

  device->setColorFrameListener(&listener);
  device->setIrAndDepthFrameListener(&listener);

  assert(device->start());
}

Camera::~Camera() {
  device->stop();
  device->close();
}

std::string Camera::serialNumber() const {
  return device->getSerialNumber();
}

bool Camera::getFrame(libfreenect2::FrameMap &map) {
  if (!listener.waitForNewFrame(map, 1000)) {
    return false;
  }

  return true;
}

void Camera::releaseFrame(libfreenect2::FrameMap &map) {
  listener.release(map);
}
