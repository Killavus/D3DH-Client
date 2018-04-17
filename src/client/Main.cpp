#include <iostream>
#include <unistd.h>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>

#include "Camera.h"
#include "rpc_package_manager.h"
#include "timeService.h"
#include "type_definitions.h"
#include "utils.h"

#include "opencv2/opencv.hpp"
using namespace cv;

void mainLoop(RpcPackageManager &rpcManager, int maxNumFramesToBeSent)
{
    libfreenect2::Freenect2 freenect;
    int kinect_count = freenect.enumerateDevices();

    if (kinect_count == 0)
    {
        std::cerr << "No Kinect device connected. Exiting..." << std::endl;
        std::exit(-1);
    }

    std::string kinectId = getHostname();
    Camera cam(freenect, 0);
    libfreenect2::FrameMap frame_map;
    libfreenect2::Frame *rgb, *ir, *depth;

    int frameCounter = 0;
    while (frameCounter++ < maxNumFramesToBeSent)
    {
        if (!cam.getFrame(frame_map))
        {
            std::cout << "Failed to get frame." << std::endl;
        }
        timeType captureTime = getTime();

        rgb = frame_map[libfreenect2::Frame::Color];
        ir = frame_map[libfreenect2::Frame::Ir];
        depth = frame_map[libfreenect2::Frame::Depth];

        IF_DEBUG(std::cerr << "Sending frame" << std::endl);
        rpcManager.call(rgb, depth, ir, kinectId, captureTime);

        cam.releaseFrame(frame_map);
    }

    cam.close();
}

int main(int argc, char *argv[])
{
    ArgsParser parser(argc, argv);

    Config config(parser.getOption("config_path"));
    TimeService timeService(config.clientsEndpoints[getHostname()].second);

    while (!timeService.isSynchronized())
    {
        IF_DEBUG(std::cerr << "Waiting for sync" << std::endl);
        sleep(1);
    }

    RpcPackageManager manager(config.mode, config.serverEndpoint);
    rpc::client client(config.serverEndpoint.first, config.serverEndpoint.second);

    mainLoop(manager, config.maxNumFramesToBeSent);

    return 0;
}
