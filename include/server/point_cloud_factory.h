#ifndef INC_3DHUMANCAPTURE_POINT_CLOUD_FACTORY_H
#define INC_3DHUMANCAPTURE_POINT_CLOUD_FACTORY_H

#include "type_definitions.h"

class PointCloudFactory
{
public:
  PointCloudFactory(const CameraCalibrationsMap &map) : calibrations(map) {}

  std::unordered_map<KinectId, PointCloud> fromPack(const PackOfFrames &framesPack) const;

private:
  const CameraCalibrationsMap &calibrations;
};

#endif // INC_3DHUMANCAPTURE_POINT_CLOUD_FACTORY_H
