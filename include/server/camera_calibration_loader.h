#ifndef INC_3DHUMANCAPTURE_CAMERA_CALIBRATION_LOADER_H
#define INC_3DHUMANCAPTURE_CAMERA_CALIBRATION_LOADER_H
#include <string>

#include <yaml-cpp/yaml.h>
#include <opencv2/core.hpp>

#include "type_definitions.h"

class CameraCalibrationLoader
{
public:
  CameraCalibrationLoader(const std::string &calibrationDataDir);
  CameraCalibration load() const;

private:
  const std::string path;
};

#endif // INC_3DHUMANCAPTURE_CAMERA_CALIBRATION_LOADER_H
