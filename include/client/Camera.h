#ifndef INC_3DHUMANCAPTURE_CAMERA_H
#define INC_3DHUMANCAPTURE_CAMERA_H
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>

class Camera {
  public:
    Camera(libfreenect2::Freenect2 &freenect, int idx);

    std::string serialNumber() const;
    bool getFrame(libfreenect2::FrameMap &map);
    void releaseFrame(libfreenect2::FrameMap &map);
    void close();

  private:
    libfreenect2::Freenect2Device *device;
    libfreenect2::SyncMultiFrameListener listener;
};

#endif //INC_3DHUMANCAPTURE_CAMERA_H
