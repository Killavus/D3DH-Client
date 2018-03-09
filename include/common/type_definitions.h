#ifndef TYPE_DEFINITIONS_H
#define TYPE_DEFINITIONS_H

#include <chrono>
#include <mutex>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <vector>

using Endpoint = std::pair<std::string, std::string>;
using Times = std::vector<std::chrono::steady_clock::time_point>;

struct KinectData
{
    
};

class ClientToFramesMapping
{
    // !!Assumption!!
    // in order not to have mutex for idToData I want 
    // initData to be called only once and before any other functions
    
public:
    void initData(std::vector<std::string> kinectsIds);
    void putFrame(const std::string &kinId, KinectData data);
    bool empty(const std::string &kinId);
    KinectData& peekFirstFrame(const std::string &kinId);
    KinectData removeFirstFrame(const std::string &kinId);
    
private:
    void dataAssert(const std::string &kinId);
    
    using FrameCollection = std::queue<KinectData>;
    std::unordered_map<std::string, FrameCollection> idToData;
    std::unordered_map<std::string, std::mutex> dataMutex;
};

#endif
