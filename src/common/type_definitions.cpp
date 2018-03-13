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

KinectData::KinectData(RawImage &&rgb, RawImage &&depth, RawImage &&ir, int width, 
    int height, time_t time)
    : rgb(std::move(rgb)), depth(std::move(depth)), ir(std::move(ir))
    , width(width), height(height), timestamp(time)
{
}

