#ifndef INC_3DHUMANCAPTURE_CAMERA_CALIBRATION_H
#define INC_3DHUMANCAPTURE_CAMERA_CALIBRATION_H
#include <opencv2/core.hpp>

struct CamerasExtrinsic {
  /*
    Color camera is unrotated & positioned at (0, 0) camera space.
    Extrinsics below are about IR camera.
  */

  cv::Mat rotation;
  cv::Mat translation;

  cv::Mat fundamental;
  cv::Mat essential;
};

struct WorldExtrinsic {
  cv::Mat rotation;
  cv::Mat translation;
};

struct CameraCalibration {
  cv::Mat colorIntrinsic;
  cv::Mat irIntrinsic;

  CamerasExtrinsic camerasExtrinsic;
  WorldExtrinsic worldExtrinsic;
};
#endif // INC_3DHUMANCAPTURE_CAMERA_CALIBRATION_LOADER_H
