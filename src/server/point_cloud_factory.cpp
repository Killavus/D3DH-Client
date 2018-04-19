#include "point_cloud_factory.h"
#include "type_definitions.h"

#include <opencv2/imgproc.hpp>
#include <vector>

std::unordered_map<KinectId, PointCloud> PointCloudFactory::fromPack(const PackOfFrames &framesPack) const
{
  std::unordered_map<KinectId, PointCloud> result;

  for (auto &frameEntry : framesPack)
  {
    auto calibrationIt = calibrations.find(frameEntry.first);
    if (calibrationIt == calibrations.end())
    {
      continue;
    }

    auto calibration = calibrationIt->second;

    auto colorImageIt = frameEntry.second.images.find(ImageType::RGB);
    bool colorDataPresent = colorImageIt != frameEntry.second.images.end();

    auto depthImageIt = frameEntry.second.images.find(ImageType::DEPTH);
    auto depthImage = depthImageIt->second;

    std::vector<cv::Point2f> distorted, undistorted;
    undistorted.reserve(depthImage.width * depthImage.height);
    distorted.reserve(depthImage.width * depthImage.height);

    for (int r = 0; r < depthImage.width; ++r)
    {
      for (int h = 0; h < depthImage.height; ++h)
      {
        distorted.push_back(cv::Point2f(float(r), float(h)));
      }
    }

    cv::undistortPoints(distorted, undistorted, calibration.irIntrinsic, calibration.irDistortion);

    float cx_d = calibration.irIntrinsic.at<float>(0, 2),
          cy_d = calibration.irIntrinsic.at<float>(1, 2),
          fx_d = calibration.irIntrinsic.at<float>(0, 0),
          fy_d = calibration.irIntrinsic.at<float>(1, 1);

    cv::Mat depthImageMat;
    cv::Mat(depthImage.width, depthImage.height, CV_32FC1, depthImage.img.data()).copyTo(depthImageMat);

    std::vector<cv::Point3f> points3d;
    points3d.reserve(depthImage.width * depthImage.height);

    for (int p = 0; p < depthImage.width * depthImage.height; ++p)
    {
      cv::Mat pix;
      cv::getRectSubPix(depthImageMat, cv::Size(1, 1), undistorted[p], pix);
      float pixVal = pix.at<float>(0, 0);
      points3d.push_back(cv::Point3f(
          ((undistorted[p].x - cx_d) * pixVal) / fx_d,
          ((undistorted[p].y - cy_d) * pixVal) / fy_d,
          pixVal));
    }

    PointCloud cloud;
    cloud.points = std::move(points3d);

    result[frameEntry.first] = cloud;
  }

  return result;
}
