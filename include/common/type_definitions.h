#ifndef TYPE_DEFINITIONS_H
#define TYPE_DEFINITIONS_H

#include <ctime>
#include <mutex>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <vector>

// <ip, port>
using Endpoint = std::pair<std::string, int>;
using KinectId = std::string;
using RawImage = std::vector<unsigned char>;
using Times = std::vector<time_t>;


struct KinectData
{
    KinectData(RawImage &&rgb, RawImage &&depth, RawImage &&ir, int width, 
               int height, time_t time);
    
    RawImage rgb;
    RawImage depth;
    RawImage ir;
    
    int width;
    int height;
    
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
