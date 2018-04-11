#ifndef D3HUMAN_RPC_PACKAGE_MANAGER
#define D3HUMAN_RPC_PACKAGE_MANAGER

#include <future>
#include <list>
#include <mutex>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <rpc/client.h>
#include "rpc/msgpack.hpp"

#include "type_definitions.h"


class RpcPackageManager
{
public:
    RpcPackageManager(Mode mode, Endpoint server);
    ~RpcPackageManager();
    
    void call(libfreenect2::Frame *rgb, libfreenect2::Frame *depth, 
        libfreenect2::Frame *ir, KinectId kinectId, timeType captureTime);
    
private:
    using Future = std::future<clmdep_msgpack::object_handle>;
    
    Mode mode;
    rpc::client serverConn;
    std::list<Future> sentCalls;
    std::mutex sentCallsMutex;
};

#endif  // D3HUMAN_RPC_PACKAGE_MANAGER
