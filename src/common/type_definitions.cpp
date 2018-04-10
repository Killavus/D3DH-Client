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

KinectData &ClientToFramesMapping::peekFirstFrame(const KinectId &kinId)
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
    : timestamp(time)
{
    images.emplace(ImageType::RGB, Image(std::move(rgb), rgbW, rgbH));
    images.emplace(ImageType::DEPTH, Image(std::move(depth), depthW, depthH));
    images.emplace(ImageType::IR, Image(std::move(ir), irW, irH));
}

KinectData::KinectData(RawImage depth, size_t depthW, size_t depthH, timeType time)
    : timestamp(time)
{
    images.emplace(ImageType::DEPTH, Image(std::move(depth), depthW, depthH));
}

Image::Image(RawImage img, int w, int h)
    : img(std::move(img)), width(w), height(h)
{
}

PackOfFramesHandler::PackOfFramesHandler(std::uint64_t maxDistBetweenFramesInBatch,
                                         std::size_t numberOfKinects,
                                         std::size_t minNumberOfFramesInPackageToAccept,
                                         timeType windowStartPos)
    : maxDistBetweenFramesInBatch(maxDistBetweenFramesInBatch)
    , numberOfKinects(numberOfKinects)
    , minNumberOfFramesInPackageToAccept(minNumberOfFramesInPackageToAccept)
    , windowStartPos(windowStartPos)
{
}

typedef std::pair<KinectId, timeType> PairType;
struct CompareSecond
{
    bool operator()(const PairType& left, const PairType& right) const
    {
        return left.second < right.second;
    }
};

void PackOfFramesHandler::putFrame(const std::string &kinectId, KinectData data)
{
    std::lock_guard<std::mutex> guard(timeLinesMutex);
    
    lastTimestamps[kinectId] = data.timestamp;
    timeLines[kinectId].insert(std::make_pair(data.timestamp, std::move(data)));
    
    timeType minOldestTimestamp = (*std::min_element(lastTimestamps.begin(), lastTimestamps.end(), CompareSecond())).second;

    while (windowStartPos < minOldestTimestamp)
    {
        PackOfTimeStamps currentPack;
        for (auto it = timeLines.begin(); it != timeLines.end(); ++it)
        {
            KinectId kinectId = (*it).first;
            for (auto kinDataIt = (*it).second.begin(); kinDataIt != (*it).second.end(); ++kinDataIt)
            {
                auto currentTime = (*kinDataIt).first;
                if (currentTime < windowStartPos) {
                    it->second.erase(kinDataIt);
                }
                else if (currentTime < windowStartPos + maxDistBetweenFramesInBatch) {
                    currentPack[kinectId] = currentTime;
                    break;
                }
            }
        }
        if (currentPack.size() >= minNumberOfFramesInPackageToAccept) {
            PackOfFrames packOfFrames;
            timeType minTimestamp = std::numeric_limits<timeType>::max();
            KinectId minKinectId{};
            
            for (auto it = currentPack.begin(); it != currentPack.end(); ++it) {
                KinectId kinectId = (*it).first;
                timeType timestamp = (*it).second;
                auto kinDataIt = timeLines[kinectId].find(timestamp);
                packOfFrames.insert(std::make_pair(kinectId, kinDataIt->second));
                if (timestamp < minTimestamp) {
                    minTimestamp = timestamp;
                    minKinectId = kinectId;
                }
            }
            readyPackOfFrames.push(std::move(packOfFrames));
            IF_DEBUG(std::cerr << "[PackOfFramesHandler] Batch gathered, putting pack to queue" << std::endl);
            windowStartPos = minTimestamp + 1;
	}
	else {
             ++windowStartPos;
        }
    }
    
    ++windowStartPos;
    /*
    if (currentIt->second.size() == numberOfKinects)
    {
        auto end = ++currentIt;
        for (auto it = packages.begin(); it != end;)
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
    */
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
