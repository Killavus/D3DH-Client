#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#include "server/pack_of_frames_to_disk_processor.h"
#include "utils.h"
#include <future>

using namespace cv;

PackOfFramesToDiskProcessor::PackOfFramesToDiskProcessor(
    const std::string &directoryName)
    : directory(directoryName)
{
    boost::filesystem::path dir(directoryName);
    boost::filesystem::create_directory(dir);
}

auto getEncoding(ImageType type)
{
    switch (type)
    {
    case ImageType::RGB:
        return CV_8UC4;
    default:
        return CV_32FC1;
    };
}

void PackOfFramesToDiskProcessor::onNewFrame(PackOfFrames &framePacks, int frameNo)
{
    auto foo = [this, &framePacks, &frameNo](){
    auto frameNoStr = std::to_string(frameNo);
    //boost::filesystem::path dir(directory + "/" + frameNoStr);
    //boost::filesystem::create_directory(dir);

    for (auto &frameEntry : framePacks)
    {
 //       for (auto &imgEntry : frameEntry.second.images)
        {
	    auto &imgEntry =  *frameEntry.second.images.find(ImageType::DEPTH);
            cv::Mat cvMat;
            std::string path = directory + "/" + frameEntry.first + "_" + imgTypeToStr(imgEntry.first) + "_" + frameNoStr + ".png";
            IF_DEBUG(std::cerr << "Saving to " << path << std::endl);
            cv::Mat(imgEntry.second.height, imgEntry.second.width, getEncoding(imgEntry.first),
                    imgEntry.second.img.data())
                .copyTo(cvMat);
            cv::imwrite(path, cvMat);	

        }
    }};
std::async(std::launch::async, foo);
}
