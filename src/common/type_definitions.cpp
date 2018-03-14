#include <cassert>

#include "type_definitions.h"

void ClientToFramesMapping::initData(std::vector<KinectId> kinectsIds)
{
    for (const auto &id : kinectsIds)
    {
        idToData[id];
        dataMutex[id];
    }
}

void ClientToFramesMapping::putFrame(const KinectId &kinId, KinectData data)
{
    dataAssert(kinId);
    
    std::lock_guard<std::mutex> guard(dataMutex[kinId]);
    idToData[kinId].push(std::move(data));
}

bool ClientToFramesMapping::empty(const KinectId &kinId)
{
    dataAssert(kinId);
    
    std::lock_guard<std::mutex> guard(dataMutex[kinId]);
    return idToData[kinId].empty();
}

KinectData& ClientToFramesMapping::peekFirstFrame(const KinectId &kinId)
{
    dataAssert(kinId);
    
    std::lock_guard<std::mutex> guard(dataMutex[kinId]);
    return idToData[kinId].front();
}

KinectData ClientToFramesMapping::removeFirstFrame(const KinectId &kinId)
{
    dataAssert(kinId);
    
    std::lock_guard<std::mutex> guard(dataMutex[kinId]);
    auto res = idToData[kinId].front();
    idToData[kinId].pop();
    return res;
}

void ClientToFramesMapping::dataAssert(const KinectId &kinId)
{
    assert(dataMutex.find(kinId) != dataMutex.end());
    assert(idToData.find(kinId) != idToData.end());
}

KinectData::KinectData(RawImage rgb, size_t rgbW, RawImage depth, size_t depthW,
    RawImage ir, size_t irW, time_t time)
    : rgb(std::move(rgb), rgbW)
    , depth(std::move(depth), depthW)
    , ir(std::move(ir), irW)
    , timestamp(time)
{
}

Image::Image(RawImage img, int w, int h) 
    : img(std::move(img)), width(w), height(h)
{
}

Image::Image(RawImage img, int w)
    : img(std::move(img)), width(w)
{
    height = this->img.size() / w;
}


PackOfFramesHandler::PackOfFramesHandler(std::uint64_t maxDistBetweenFramesInBatch)
    : maxDistBetweenFramesInBatch(maxDistBetweenFramesInBatch)
{
}

void PackOfFramesHandler::putFrame(const std::string& kinectId, KinectData&& data)
{
    auto frameID = getFrameId(data.timestamp);
    std::lock_guard<std::mutex> guard(packagesMutex);
    packages[frameID].insert(std::pair<std::string, KinectData>(kinectId, std::move(data)));

    auto currentIt = packages.find(frameID);
    if (currentIt->second.size() == 3)
    {
        for (auto it = packages.begin(); it != currentIt; )
        {
            if (it->second.size() >= 2)
            {
                std::lock_guard<std::mutex> guard(readyPackOfFramesMutex);
                readyPackOfFrames.push(it->second);
                packages.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }
}

boost::optional<PackOfFrames> PackOfFramesHandler::getNextPackOfFrames()
{
    std::lock_guard<std::mutex> guard(readyPackOfFramesMutex);
    if (!readyPackOfFrames.empty())
    {
        auto result = readyPackOfFrames.front();
        readyPackOfFrames.pop();
        return result;
    }
    return boost::none;
}

std::uint64_t PackOfFramesHandler::getFrameId(std::uint64_t timestamp)
{
    return timestamp / maxDistBetweenFramesInBatch;
}
