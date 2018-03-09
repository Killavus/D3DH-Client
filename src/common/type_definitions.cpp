#include <cassert>

#include "type_definitions.h"

void ClientToFramesMapping::initData(std::vector<std::string> kinectsIds)
{
    for (const auto &id : kinectsIds)
    {
        idToData[id];
        dataMutex[id];
    }
}

void ClientToFramesMapping::putFrame(const std::string &kinId, KinectData data)
{
    dataAssert(kinId);
    
    std::lock_guard<std::mutex> guard(dataMutex[kinId]);
    idToData[kinId].push(std::move(data));
}

bool ClientToFramesMapping::empty(const std::string &kinId)
{
    dataAssert(kinId);
    
    std::lock_guard<std::mutex> guard(dataMutex[kinId]);
    return idToData[kinId].empty();
}

KinectData& ClientToFramesMapping::peekFirstFrame(const std::string &kinId)
{
    dataAssert(kinId);
    
    std::lock_guard<std::mutex> guard(dataMutex[kinId]);
    return idToData[kinId].front();
}

KinectData ClientToFramesMapping::removeFirstFrame(const std::string &kinId)
{
    dataAssert(kinId);
    
    std::lock_guard<std::mutex> guard(dataMutex[kinId]);
    auto res = idToData[kinId].front();
    idToData[kinId].pop();
    return res;
}

void ClientToFramesMapping::dataAssert(const std::string &kinId)
{
    assert(dataMutex.find(kinId) != dataMutex.end());
    assert(idToData.find(kinId) != idToData.end());
}
