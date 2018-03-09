#include <iostream>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>

#include "Camera.h"

#define CONFIG_FNAME "config.ini"


int main(int argc, char *argv[]) {
  libfreenect2::Freenect2 freenect;
  int kinect_count = freenect.enumerateDevices();

  if (kinect_count == 0) {
    std::cerr << "No Kinect device connected. Exiting..." << std::endl;
    std::exit(-1);
  }

  Camera cam(freenect, 0);
  libfreenect2::FrameMap frame_map;
  libfreenect2::Frame *rgb, *ir, *depth;

  while (true) {
    if (!cam.getFrame(frame_map)) {
      std::cout << "Failed to get frame." << std::endl;
    }

    rgb = frame_map[libfreenect2::Frame::Color];
    ir = frame_map[libfreenect2::Frame::Ir];
    depth = frame_map[libfreenect2::Frame::Depth];

    std::cout << "rgb: " << rgb->timestamp << " | ir: " << ir->timestamp << " | depth: " << depth->timestamp << std::endl;

    cam.releaseFrame(frame_map);
  }

  return 0; 
}
