#ifndef CAMERA_H
#define CAMERA_H
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>

class Camera {
  public:
    Camera(libfreenect2::Freenect2 &freenect, int idx);
    ~Camera();

    std::string serialNumber() const;
    bool getFrame(libfreenect2::FrameMap &map);
    void releaseFrame(libfreenect2::FrameMap &map);

  private:
    libfreenect2::Freenect2Device *device;
    libfreenect2::SyncMultiFrameListener listener;
};

#endif //CAMERA_H
