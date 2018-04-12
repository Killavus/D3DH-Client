#include <future>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#include "server/pack_of_frames_as_binary_processor.h"
#include "utils.h"


using namespace cv;

PackOfFramesAsBinaryProcessor::PackOfFramesAsBinaryProcessor(
    const std::string &directoryName)
    : directory(directoryName)
{
    boost::filesystem::path dir(directoryName);
    boost::filesystem::create_directory(dir);
}

void PackOfFramesAsBinaryProcessor::onNewFrame(PackOfFrames &framePacks, int frameNo)
{
    auto foo = [this, &framePacks, &frameNo](){
    auto frameNoStr = std::to_string(frameNo);

    for (auto &frameEntry : framePacks)
    {
        frameEntry.second.saveAsBinary(directory + "/" + 
                frameEntry.first + "_batch_"+ 
                frameNoStr);
    }};
    std::async(std::launch::async, foo);
}
