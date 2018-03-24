#include <unistd.h>

#include <boost/filesystem.hpp>
#include "opencv2/opencv.hpp"

#include "frame_processors.h"

using namespace cv;

FrameProcessorBase::FrameProcessorBase(PackOfFramesHandler &frameHandler)
	: frameHandler(frameHandler)
{}

ToFileWriter::ToFileWriter(std::string directory, PackOfFramesHandler &frameHandler)
	: FrameProcessorBase(frameHandler)
	, directory(directory)
{
	boost::filesystem::path dir(directory);
	boost::filesystem::create_directory(dir);
}

void ToFileWriter::processFrames()
{
	int frameCounter = 0;
	cv::Mat rgbmat, irmat, depthmat;

	while (true)
	{
		auto nextFrameMaybe = frameHandler.getNextPackOfFrames();
		if (!nextFrameMaybe)
			sleep(1);
		else
		{
			auto &frameData = *nextFrameMaybe;
			for (auto &entry : frameData)
			{
				std::string path = entry.first + "_depth_" + std::to_string(frameCounter) + ".png";
				cv::Mat(entry.second.depth.height, entry.second.depth.width, CV_32FC1, 
					entry.second.depth.img.data()).copyTo(depthmat);
				cv::imwrite(path, depthmat);

				path = entry.first + "_rgb_" + std::to_string(frameCounter) + ".png";
                cv::Mat(entry.second.rgb.height, entry.second.rgb.width, CV_8UC4, 
                    entry.second.rgb.img.data()).copyTo(rgbmat);
                cv::imwrite(path, rgbmat);
				
				path = entry.first + "_ir_" + std::to_string(frameCounter) + ".png";
                cv::Mat(entry.second.ir.height, entry.second.ir.width, CV_32FC1, 
                    entry.second.ir.img.data()).copyTo(irmat);
                cv::imwrite(path, irmat);
			}

			++frameCounter;
		}
	}
}

