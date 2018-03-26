#include <cassert>

#include "type_definitions.h"
#include "utils.h"

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

KinectData::KinectData(RawImage rgb, size_t rgbW, size_t rgbH,
    RawImage depth, size_t depthW, size_t depthH,
    RawImage ir, size_t irW, size_t irH, timeType time)
    : rgb(std::move(rgb), rgbW, rgbH)
    , depth(std::move(depth), depthW, depthH)
    , ir(std::move(ir), irW, irH)
    , timestamp(time)
{
}

Image::Image(RawImage img, int w, int h) 
    : img(std::move(img)), width(w), height(h)
{
}

PackOfFramesHandler::PackOfFramesHandler(std::uint64_t maxDistBetweenFramesInBatch,
    std::size_t numberOfKinects,
    std::size_t minNumberOfFramesInPackageToAccept)
    : maxDistBetweenFramesInBatch(maxDistBetweenFramesInBatch)
    , numberOfKinects(numberOfKinects)
    , minNumberOfFramesInPackageToAccept(minNumberOfFramesInPackageToAccept)
{
}

void PackOfFramesHandler::putFrame(const std::string& kinectId, KinectData&& data)
{
    auto frameID = getFrameId(data.timestamp);
    std::lock_guard<std::mutex> guard(packagesMutex);
    packages[frameID].insert(std::make_pair(kinectId, std::move(data)));

    auto currentIt = packages.find(frameID);
    if (currentIt->second.size() == numberOfKinects)
    {
        auto end = ++currentIt;
        for (auto it = packages.begin(); it != end; )
        {
            if (it->second.size() >= minNumberOfFramesInPackageToAccept)
            {
                IF_DEBUG(std::cerr << "[PackOfFramesHandler] Batch gathered, putting pack to queue" << std::endl);
                std::lock_guard<std::mutex> guard(readyPackOfFramesMutex);
                readyPackOfFrames.push(std::move(it->second));
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
        auto result = std::move(readyPackOfFrames.front());
        readyPackOfFrames.pop();
        return result;
    }
    return boost::none;
}

std::uint64_t PackOfFramesHandler::getFrameId(std::uint64_t timestamp)
{
    return timestamp / maxDistBetweenFramesInBatch;
}
