//
// Created by shrum on 11.04.18.
//

#include <iostream>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include "Camera.h"
#include "calibration/aruco.h"

#include <boost/program_options.hpp>

using namespace cv;
using namespace std;
using namespace aruco;

namespace po = boost::program_options;

const int MIN_REQUIRED_TO_CALIBRATE = 4;

static const Scalar TEXT_COLOR = Scalar(125,255,255);

static constexpr char ESC_KEY = 27;

// Describes the kinect frame
struct KinectFrame
{
    Mat rgb;
    Mat depth;
    Mat ir;
    uint32_t rgbTimestamp;
    uint32_t irTimestamp;
    uint32_t depthTimestamp;

    int index;
};

// Returns next kinect frame from camera
KinectFrame getFrame(Camera& cam)
{
    static libfreenect2::FrameMap frame_map;
    static libfreenect2::Frame *rgb, *ir, *depth;
    static int counter = 0;
    if(!cam.getFrame(frame_map))
    {
        std::cerr << "Failed to get frame from kinect." << std::endl;
    }

    // Extract the frame
    rgb = frame_map[libfreenect2::Frame::Color];
    ir = frame_map[libfreenect2::Frame::Ir];
    depth = frame_map[libfreenect2::Frame::Depth];

    KinectFrame result = {};

    Mat(static_cast<int>(rgb->height), static_cast<int>(rgb->width), CV_8UC4, rgb->data).copyTo(result.rgb);
    Mat(static_cast<int>(ir->height), static_cast<int>(ir->width), CV_32FC1, ir->data).convertTo(result.ir,CV_16UC1);
    Mat(static_cast<int>(depth->height), static_cast<int>(depth->width), CV_32FC1, depth->data).copyTo(result.depth);

    normalize(result.ir,result.ir,0,255,NORM_MINMAX);
    result.ir.convertTo(result.ir,CV_8UC1);

    cvtColor(result.rgb,result.rgb,CV_BGRA2BGR);
    result.rgbTimestamp = rgb->timestamp;
    result.irTimestamp = ir->timestamp;
    result.depthTimestamp = depth->timestamp;
    
    cam.releaseFrame(frame_map);

    result.index = counter++;

    return result;
}

// Returns next kinect frame from camera
KinectFrame getFrameMock(VideoCapture* capture)
{
    static int counter = 0;
    KinectFrame result;
    *capture >> result.rgb;

    cvtColor(result.rgb, result.ir, COLOR_BGR2GRAY);
    cvtColor(result.rgb, result.depth, COLOR_BGR2GRAY);

    result.rgbTimestamp = 0;
    result.irTimestamp = 0;
    result.depthTimestamp = 0;

    result.index = counter++;
    return result;
}

// Puts the text on the kinect frame. (This puts the text on each of them.
void putText(KinectFrame* frame,const string& text, const Point &orig)
{
    putText(frame->rgb,text,orig,FONT_HERSHEY_SIMPLEX, 0.7f,TEXT_COLOR,1);
    putText(frame->ir,text,orig,FONT_HERSHEY_SIMPLEX, 0.7f,TEXT_COLOR,1);
    putText(frame->depth,text,orig,FONT_HERSHEY_SIMPLEX, 0.7f,TEXT_COLOR,1);
}

// Saves the kinect frame in given path adding .png and slightly modifying the filenames
void saveFrame(KinectFrame* frame, const string& path)
{
    imwrite(path + to_string(frame->index) + "rgbPoseMarker" + to_string(frame->rgbTimestamp) + ".png",frame->rgb);
    imwrite(path + to_string(frame->index) + "irPoseMarker" + to_string(frame->irTimestamp) + ".png",frame->ir);
    imwrite(path + to_string(frame->index) + "depthPoseMarker" + to_string(frame->depthTimestamp) + ".png",frame->depth);
}

// Shows the frame. This function only works if named windows were created with names:
// rgb
// ir
// depth
void showFrame(const KinectFrame& frame)
{
    imshow("rgb",frame.rgb);
    imshow("ir",frame.ir);
    imshow("depth",frame.depth);
}


// Saves the informations about detected markers.
void saveMarkerDetectionInfo(const std::string& path, const vector<Marker>& TheMarkers)
{
    FileStorage fs(path + "pose.yaml", FileStorage::WRITE);
    for(unsigned int i = 0; i < TheMarkers.size(); i++)
    {
        fs << "Marker_" + to_string(i)  +"_id" << TheMarkers[i].id;
        fs << "Marker_" + to_string(i)  +"_rvec" << TheMarkers[i].Rvec;
        fs << "Marker_" + to_string(i)  +"_tvec" << TheMarkers[i].Tvec;
    }
}


int main(int argc,const char* argv[])
{
    po::options_description description("Allowed options");
    description.add_options()
            ("help,h", "show options of program")
            ("path,p", po::value<std::string>(),
             "save calibration path, this string will be appended to all saved files at the beginning")
            ("mock,m", po::value<int>(),
             "mock the kinect input with given camera device, default camera device has index 0")
            ("camparams,c",po::value<std::string>(),"camera parameters in yaml. Can be created with calibrate program.")
            ("size,s",po::value<float>(),"the size of the markers");
          

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, description), vm);
        po::notify(vm);
    }
    catch (const boost::exception& e)
    {
        std::cout << description << std::endl;
        return 0;
    }

    if(vm.count("help"))
    {
        std::cout << description << std::endl;
        return 0;
    }

    std::string path;
    if(vm.count("path"))
    {
        path = vm["path"].as<std::string>();
    }

    bool mock = static_cast<bool>(vm.count("mock"));

    unique_ptr<Camera> cam;
    unique_ptr<VideoCapture> capture;
    
    unique_ptr<libfreenect2::Freenect2> freenect;
    
    if(!mock)
    {	
	freenect = unique_ptr<libfreenect2::Freenect2>(new libfreenect2::Freenect2());
        int kinect_count = freenect->enumerateDevices();

        if (kinect_count == 0) {
	    std::cerr << "No Kinect device connected. Exiting..." << std::endl;
	    std::exit(-1);
        }
	cam = unique_ptr<Camera>(new Camera(*freenect,0));
    }
    else
        capture = unique_ptr<VideoCapture>(new VideoCapture(vm["mock"].as<int>()));

    cv::namedWindow("rgb", cv::WINDOW_NORMAL);
    cv::resizeWindow("rgb",640,480);

    cv::namedWindow("ir", cv::WINDOW_NORMAL);
    cv::resizeWindow("ir",640,480);

    cv::namedWindow("depth", cv::WINDOW_NORMAL);
    cv::resizeWindow("depth",640,480);

    MarkerDetector MDetector;
    vector<Marker> TheMarkers;
    CameraParameters TheCameraParameters;

    float TheMarkerSize = 1;
    if(vm.count("size"))
      TheMarkerSize = vm["size"].as<float>();

    if(vm.count("camparams"))
      TheCameraParameters.readFromXMLFile(vm["camparams"].as<std::string>());

    char key;
    do
    {
        KinectFrame frame;

        if(!mock)
            frame = getFrame(*cam);
        else
            frame = getFrameMock(capture.get());

        
        key = static_cast<char>(cv::waitKey(10));

        TheMarkers = MDetector.detect(frame.rgb, TheCameraParameters, TheMarkerSize);

        if(key == 'a')
        {      
            if(TheMarkers.size() >= 1)
            {
                saveMarkerDetectionInfo(path, TheMarkers);
                saveFrame(&frame,path);
                cout << "Saved!" << endl;
            }
        }

        putText(&frame,"'a' to save the current frame and the pose of the markers for the current frame.",{10,40});
        for(unsigned int i = 0; i < TheMarkers.size(); i++)
        {
            TheMarkers[i].draw(frame.rgb, Scalar(0,0,255),2,true);
        }
        showFrame(frame);

    } while(key != ESC_KEY);
    if (!mock) cam->close();
}
