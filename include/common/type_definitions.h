#ifndef TYPE_DEFINITIONS_H
#define TYPE_DEFINITIONS_H

#include <ctime>
#include <mutex>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <vector>

// <ip, port>
using Endpoint = std::pair<std::string, uint16_t>;
using KinectId = std::string;
using RawImage = std::vector<unsigned char>;
using Times = std::vector<time_t>;

struct Image
{
    Image(RawImage img, int w, int h);
    Image(RawImage img, int w);
    
    RawImage img;
    int width, height;
};

struct KinectData
{
    KinectData(RawImage rgb, size_t rgbW, RawImage depth, size_t depthW,
        RawImage ir, size_t irW, time_t time);
    
    Image rgb, depth, ir;
    time_t timestamp;
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

#endif
