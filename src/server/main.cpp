#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "data_capture.h"
#include "utils.h"
#include "type_definitions.h"

#include "server/frame_processors.h"
#include "server/pack_of_frames_as_binary_processor.h"
#include "server/pack_of_frames_to_disk_processor.h"
#include "server/pack_of_frames_frontend_processor.h"

#include "server/camera_calibration_loader.h"
#include "server/point_cloud_factory.h"

void loadCalibrationData(const Config &config, CameraCalibrationsMap &calibrations)
{
    std::cout << "Loading calibration data..." << std::endl;
    for (auto it = config.clientCalibrationPaths.begin(); it != config.clientCalibrationPaths.end(); ++it)
    {

        CameraCalibrationLoader loader(it->second);
        calibrations[it->first] = loader.load();

#ifdef DEBUG_CALIB_LOADING
        std::cout << "Loaded calibration for " << it->first << "." << std::endl;
        std::cout << "Color intrinsic: " << std::endl
                  << cameraCalibrations[it->first].colorIntrinsic
                  << std::endl;

        std::cout << "IR intrinsic: " << std::endl
                  << cameraCalibrations[it->first].irIntrinsic
                  << std::endl;

        std::cout << "Camera rotation extrinsic: " << std::endl
                  << cameraCalibrations[it->first].camerasExtrinsic.rotation << std::endl;

        std::cout << "Camera translation extrinsic: " << std::endl
                  << cameraCalibrations[it->first].camerasExtrinsic.translation << std::endl;

        std::cout << "World rotation extrinsic: " << std::endl
                  << cameraCalibrations[it->first].worldExtrinsic.rotation << std::endl;

        std::cout << "World translation extrinsic: " << std::endl
                  << cameraCalibrations[it->first].worldExtrinsic.translation << std::endl;
#endif
    }
    std::cout << "Loaded " << config.clientCalibrationPaths.size() << " stored camera calibrations." << std::endl;
}

int main(int argc, char **argv)
{
    ArgsParser parser(argc, argv);
    Config config(parser.getOption("config_path"));
    CameraCalibrationsMap cameraCalibrations;

    loadCalibrationData(config, cameraCalibrations);

    PackOfFramesHandler frameSynchronizer(config.maxDistBetweenFramesInBatch,
                                          config.clientsEndpoints.size(),
                                          config.minNumberOfFramesInPackageToAccept,
                                          getTime());

    PointCloudFactory pointCloudFactory(cameraCalibrations);

    Server srv(config.serverEndpoint.second,
               config.clientsEndpoints, frameSynchronizer);

    srv.performSynchronization();

    auto frameProcessor =
        std::make_shared<ChainFrameProcessor>(frameSynchronizer);
    auto toDiskProcessor =
        std::make_shared<PackOfFramesToDiskProcessor>(config.outputDirectory);
    auto asBinaryProcessor =
        std::make_shared<PackOfFramesAsBinaryProcessor>(config.outputDirectory);

    if (config.withFrontend)
    {
        Frontend frontend(frameProcessor);

        auto guiUpdateProcessor =
            std::make_shared<PackOfFramesFrontendProcessor>(frontend);
        frameProcessor->addProcessor(guiUpdateProcessor);
        frameProcessor->addProcessor(toDiskProcessor);

        frontend.loop();
    }
    else
    {
        frameProcessor->addProcessor(toDiskProcessor);
        frameProcessor->addProcessor(asBinaryProcessor);
        frameProcessor->processFrames();
    }

    return 0;
}
