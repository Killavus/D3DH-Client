#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#include "server/pack_of_frames_to_disk_processor.h"

using namespace cv;

PackOfFramesToDiskProcessor::PackOfFramesToDiskProcessor(
    const std::string& directoryName) 
    : directory(directoryName)
{
    boost::filesystem::path dir(directoryName);
    boost::filesystem::create_directory(dir);
}

void PackOfFramesToDiskProcessor::onNewFrame(PackOfFrames& framePacks, int frameNo) 
{
    cv::Mat depthmat, rgbmat, irmat;
  
    for (auto &entry : framePacks) 
    { 
        std::string path = directory + "/" + std::to_string(frameNo) + 
            "_" + entry.first + "_depth_" + ".png";
        cv::Mat(entry.second.depth.height, entry.second.depth.width, CV_32FC1, 
        entry.second.depth.img.data()).copyTo(depthmat);
        cv::imwrite(path, depthmat);

        path = directory + "/" + std::to_string(frameNo) + 
            "_" + entry.first + "_rgb_" + ".png";
        cv::Mat(entry.second.rgb.height, entry.second.rgb.width, CV_8UC4, 
            entry.second.rgb.img.data()).copyTo(rgbmat);
        cv::imwrite(path, rgbmat);
        
        path = directory + "/" + std::to_string(frameNo) + 
            "_" + entry.first + "_ir_" + ".png";
        cv::Mat(entry.second.ir.height, entry.second.ir.width, CV_32FC1, 
            entry.second.ir.img.data()).copyTo(irmat);
        cv::imwrite(path, irmat);
    }
}
