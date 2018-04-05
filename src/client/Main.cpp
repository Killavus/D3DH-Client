#include <iostream>
#include <unistd.h>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <rpc/client.h>

#include "Camera.h"
#include "timeService.h"
#include "type_definitions.h"
#include "utils.h"

#include "opencv2/opencv.hpp"
using namespace cv;

void mainLoop(rpc::client &client)
{
    int counter = 0;
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

    while (true)
    {
        if (!cam.getFrame(frame_map))
        {
            std::cout << "Failed to get frame." << std::endl;
        }
        timeType captureTime = getTime();
	
        rgb = frame_map[libfreenect2::Frame::Color];
        ir = frame_map[libfreenect2::Frame::Ir];
        depth = frame_map[libfreenect2::Frame::Depth];

        auto rgbVec = copyToVector(rgb->data,
                                   rgb->width * rgb->height * rgb->bytes_per_pixel);
        auto irVec = copyToVector(ir->data,
                                  ir->width * ir->height * ir->bytes_per_pixel);
        auto depthVec = copyToVector(depth->data,
                                     depth->width * depth->height * depth->bytes_per_pixel);

        IF_DEBUG(std::cerr << "Sending frame" << std::endl);
	//if (counter++ % 10 == 0)
        client.async_call("pushAllKinectData", kinectId,
                          rgbVec, rgb->width, rgb->height,
                          depthVec, depth->width, depth->height,
                          irVec, ir->width, ir->height, captureTime);

        cam.releaseFrame(frame_map);
    }
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

    rpc::client client(config.serverEndpoint.first, config.serverEndpoint.second);

    mainLoop(client);

    return 0;
}
