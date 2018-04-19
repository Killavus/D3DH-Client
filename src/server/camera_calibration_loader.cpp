#include <yaml-cpp/yaml.h>
#include <opencv2/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>

#include "camera_calibration_loader.h"
#include "type_definitions.h"

CameraCalibrationLoader::CameraCalibrationLoader(
    const std::string &calibrationDataDir) : path(calibrationDataDir) {}

CameraCalibration CameraCalibrationLoader::load() const
{
  CameraCalibration calibration;

  cv::FileStorage pose(path + "/pose.yaml", cv::FileStorage::READ);
  cv::FileStorage extrinsic(path + "/extrinsic.yaml", cv::FileStorage::READ);
  cv::FileStorage ir(path + "/ir_camera.yaml", cv::FileStorage::READ);
  cv::FileStorage color(path + "/color_camera.yaml", cv::FileStorage::READ);

  cv::Mat worldRotationVec;

  pose["Marker_0_rvec"] >> worldRotationVec;
  cv::Rodrigues(worldRotationVec, calibration.worldExtrinsicColor.rotation);

  pose["Marker_0_tvec"] >> calibration.worldExtrinsicColor.translation;

  extrinsic["Rotation"] >> calibration.camerasExtrinsic.rotation;
  extrinsic["Translation"] >> calibration.camerasExtrinsic.translation;

  ir["Camera_Matrix"] >> calibration.irIntrinsic;
  color["Camera_Matrix"] >> calibration.colorIntrinsic;
  ir["Distortion_Coefficients"] >> calibration.irDistortion;
  color["Distortion_Coefficients"] >> calibration.colorDistortion;

  return calibration;
}
