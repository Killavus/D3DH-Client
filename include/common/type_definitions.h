#ifndef TYPE_DEFINITIONS_H
#define TYPE_DEFINITIONS_H

#include <mutex>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <map>
#include <vector>

#include <boost/optional.hpp>

// <ip, port>
using Endpoint = std::pair<std::string, uint16_t>;
using KinectId = std::string;
using RawImage = std::vector<unsigned char>;
using timeType = std::int64_t;
using Times = std::vector<timeType>;

struct Image
{
    Image(RawImage img, int w, int h);
    
    RawImage img;
    int width, height;
};

struct KinectData
{
    KinectData(RawImage rgb, size_t rgbW, size_t rgbH, 
	RawImage depth, size_t depthW, size_t depthH,
        RawImage ir, size_t irW, size_t irH, timeType time);
    
    Image rgb, depth, ir;
    timeType timestamp;
};

class ClientToFramesMapping
{
    // !!Assumption!!
    // in order not to have mutex for idToData I want 
    // initData to be called only once and before any other functions
    
public:
    void initData(std::vector<KinectId> kinectsIds);
    void putFrame(const KinectId &kinId, KinectData data);
    bool empty(const KinectId &kinId);
    KinectData& peekFirstFrame(const KinectId &kinId);
    KinectData removeFirstFrame(const KinectId &kinId);
    
private:
    void dataAssert(const KinectId &kinId);
    
    using FrameCollection = std::queue<KinectData>;
    std::unordered_map<KinectId, FrameCollection> idToData;
    std::unordered_map<KinectId, std::mutex> dataMutex;
};

using PackOfFrames = std::unordered_map<KinectId, KinectData>;

class PackOfFramesHandler {
public:
    PackOfFramesHandler(std::uint64_t maxDistBetweenFramesInBatch,
                        std::uint8_t numberOfKinects,
                        std::size_t minNumberOfFramesInPackageToAccept);

    void putFrame(const KinectId& kinectId, KinectData&& data);
    boost::optional<PackOfFrames> getNextPackOfFrames();

private:
    std::uint64_t getFrameId(std::uint64_t frameID);

    std::mutex packagesMutex;
    std::mutex readyPackOfFramesMutex;

    std::map<std::uint64_t, PackOfFrames> packages;
    std::queue<PackOfFrames> readyPackOfFrames;
    std::uint64_t maxDistBetweenFramesInBatch;
    std::uint8_t numberOfKinects;
    std::size_t minNumberOfFramesInPackageToAccept;
};



#endif
