#include "rpc_package_manager.h"
#include "utils.h"

RpcPackageManager::RpcPackageManager(Mode mode, Endpoint server)
    : mode(mode)
    , serverConn(server.first, server.second) {}

RpcPackageManager::~RpcPackageManager()
{
    std::lock_guard<std::mutex> guard(sentCallsMutex);
    auto it = sentCalls.begin();
    while (it != sentCalls.end())
    {
        it->wait();
        auto next = ++it;
        sentCalls.erase(it);
        it = next;
    }
}
    
void RpcPackageManager::call(libfreenect2::Frame *rgb, libfreenect2::Frame *depth, 
        libfreenect2::Frame *ir, KinectId kinectId, timeType captureTime)
{
    switch (mode)
    {
        case Mode::SEND_ALL:
            {
                auto rgbVec = copyToVector(rgb->data,
                    rgb->width * rgb->height * rgb->bytes_per_pixel);
                auto irVec = copyToVector(ir->data,
                    ir->width * ir->height * ir->bytes_per_pixel);
                auto depthVec = copyToVector(depth->data,
                    depth->width * depth->height * depth->bytes_per_pixel);
                
                auto future = serverConn.async_call("pushAllKinectData", 
                    kinectId, rgbVec, rgb->width, rgb->height,
                    depthVec, depth->width, depth->height,
                    irVec, ir->width, ir->height, captureTime);
                
                std::lock_guard<std::mutex> guard(sentCallsMutex);
                sentCalls.push_back(std::move(future));
            }
            break;
            
        case Mode::SEND_DEPTH:
            {
                auto depthVec = copyToVector(depth->data,
                    depth->width * depth->height * depth->bytes_per_pixel);
                auto future = serverConn.async_call("pushDepthKinectData", 
                    kinectId,  depthVec, depth->width, depth->height, 
                    captureTime);
                
                std::lock_guard<std::mutex> guard(sentCallsMutex);
                sentCalls.push_back(std::move(future));
            }
            break;
    }
}

