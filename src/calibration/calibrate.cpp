//
// Created by shrum on 09.04.18.
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

#include <boost/program_options.hpp>

using namespace cv;
using namespace std;
namespace po = boost::program_options;

const Size CHESSBOARD_SIZE = Size(9,6);
constexpr float CHESSBOARD_SQUARE_SIZE = 50;
const int MIN_REQUIRED_TO_CALIBRATE = 4;

static const Scalar TEXT_COLOR = Scalar(125,255,255);

static constexpr char ESC_KEY = 27;

// Returns the camera object of the kinect that can be used to get frames
unique_ptr<Camera> initKinect()
{
}

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

// Represents result of detection of chessboard pattern on one kinect frame
struct ChessboardPoints
{
    bool success;
    vector<Point2f> rgbPoints;
    vector<Point2f> irPoints;
};

// Searches for chessboard points on given kinect frame
ChessboardPoints findChessboardPoints(const KinectFrame& frame)
{
    ChessboardPoints result;

    bool foundColor = cv::findChessboardCorners(frame.rgb,CHESSBOARD_SIZE,result.rgbPoints, cv::CALIB_CB_FAST_CHECK);
    bool foundIr = cv::findChessboardCorners(frame.ir,CHESSBOARD_SIZE,result.irPoints, CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK);

    result.success = foundColor && foundIr;
    return result;
}

// Draws chessboard corners on kinect frame
void drawChessboardCorners(KinectFrame* frame, const ChessboardPoints& points)
{
    drawChessboardCorners( frame->rgb, CHESSBOARD_SIZE, Mat(points.rgbPoints), points.success);
    drawChessboardCorners( frame->ir, CHESSBOARD_SIZE, Mat(points.irPoints), points.success);
    drawChessboardCorners( frame->depth, CHESSBOARD_SIZE, Mat(points.irPoints), points.success);
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
    imwrite(path + to_string(frame->index) + "rgb" + to_string(frame->rgbTimestamp) + ".png",frame->rgb);
    imwrite(path + to_string(frame->index) + "ir" + to_string(frame->irTimestamp) + ".png",frame->ir);
    imwrite(path + to_string(frame->index) + "depth" + to_string(frame->depthTimestamp) + ".png",frame->depth);
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

// This will compute the reprojection error for given matrix and points.
static double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                         const vector<vector<Point2f> >& imagePoints,
                                         const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                         const Mat& cameraMatrix , const Mat& distCoeffs,
                                         vector<float>& perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i )
    {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), NORM_L2);

        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

// This gives chessboard corner positions for given board size and square size.
// Example:
//     vector<Point3f> pts;
//     calcBoardCornerPositions(Size(9,6),50,pts);
static void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners)
{
    corners.clear();
    for( int i = 0; i < boardSize.height; ++i )
        for( int j = 0; j < boardSize.width; ++j )
            corners.push_back(Point3f(j * squareSize, i * squareSize, 0));

}

// This will calibrate the camera. The result will be in cameraMatrix and distCoeffs.
static bool runCalibration( Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                            vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                            vector<float>& reprojErrs,  double& totalAvgErr)
{

    cameraMatrix = Mat::eye(3, 3, CV_64F);

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(CHESSBOARD_SIZE, CHESSBOARD_SQUARE_SIZE, objectPoints[0]);

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters.
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
                                 distCoeffs, rvecs, tvecs, CALIB_FIX_K4|CALIB_FIX_K5);

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                            rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}

// Print camera parameters to the output file.
static void saveCameraParams( std::string filename, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                              const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                              const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
                              double totalAvgErr )
{
    FileStorage fs( filename, FileStorage::WRITE );

    time_t tm;
    time( &tm );
    struct tm *t2 = localtime( &tm );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_Time" << buf;

    if( !rvecs.empty() || !reprojErrs.empty() )
        fs << "nrOfFrames" << (int)std::max(rvecs.size(), reprojErrs.size());
    fs << "image_Width" << imageSize.width;
    fs << "image_Height" << imageSize.height;
    fs << "board_Width" << 9;
    fs << "board_Height" << 6;
    fs << "square_Size" << 50;

    fs << "Camera_Matrix" << cameraMatrix;
    fs << "Distortion_Coefficients" << distCoeffs;

    fs << "Avg_Reprojection_Error" << totalAvgErr;
    if( !reprojErrs.empty() )
        fs << "Per_View_Reprojection_Errors" << Mat(reprojErrs);

    if( !rvecs.empty() && !tvecs.empty() )
    {
        CV_Assert(rvecs[0].type() == tvecs[0].type());
        Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
        for( int i = 0; i < (int)rvecs.size(); i++ )
        {
            Mat r = bigmat(Range(i, i+1), Range(0,3));
            Mat t = bigmat(Range(i, i+1), Range(3,6));

            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
            //*.t() is MatExpr (not Mat) so we can use assignment operator
            r = rvecs[i].t();
            t = tvecs[i].t();
        }
        //cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
        fs << "Extrinsic_Parameters" << bigmat;
    }

    if( !imagePoints.empty() )
    {
        Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
        for( int i = 0; i < (int)imagePoints.size(); i++ )
        {
            Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
            Mat imgpti(imagePoints[i]);
            imgpti.copyTo(r);
        }
        fs << "Image_points" << imagePtMat;
    }
}

// This will run calibration process and save the results if it succeeds in given filename.
bool runCalibrationAndSave(Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,vector<vector<Point2f> > imagePoints, std::string filename )
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;

    bool ok = runCalibration(imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs,
                             reprojErrs, totalAvgErr);
    cout << (ok ? "Calibration succeeded" : "Calibration failed")
         << ". avg re projection error = "  << totalAvgErr << endl;

    if( ok )   // save only if the calibration was done with success
        saveCameraParams(filename, imageSize, cameraMatrix, distCoeffs, rvecs ,tvecs, reprojErrs,
                         imagePoints, totalAvgErr);
    return ok;
}

// Print extrinsic calibration parameters to the output file
static void saveExtrinsicCameraParams( std::string filename, Mat& rotation,
                                       Mat& translation,
                                       Mat& essential,
                                       Mat& fundamental )
{
    FileStorage fs( filename, FileStorage::WRITE );

    time_t tm;
    time( &tm );
    struct tm *t2 = localtime( &tm );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_Time" << buf;

    fs << "board_Width" << 9;
    fs << "board_Height" << 6;
    fs << "square_Size" << 50;

    fs << "Rotation" << rotation;
    fs << "Translation" << translation;

    fs << "Essential" << essential;
    fs << "Fundamental" << fundamental;
}

// This will try to calibrate extrinsic parameters of the two cameras. (the translation and rotation beetwen them.) and
// will save it to the filename.
double runExtrinsicCalibrationAndSave(vector< vector<Point2f> >& points_first,
                                    vector< vector<Point2f> >& points_second,
                                    Mat& matrix_first,
                                    Mat& matrix_second,
                                    Mat& distortion_first,
                                    Mat& distortion_second,
                                    const Size& imageSize,
                                    Mat& rotation,
                                    Mat& translation,
                                    Mat& essential,
                                    Mat& fundamental, std::string filename)
{

    vector<Point3f> boardPoints;
    calcBoardCornerPositions(CHESSBOARD_SIZE, CHESSBOARD_SQUARE_SIZE ,boardPoints);
    vector<vector<Point3f> > boardPointsBig;
    for(int i = 0; i < points_first.size();i++)
        boardPointsBig.push_back(boardPoints);
    const cv::TermCriteria termCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, DBL_EPSILON);

    double error = cv::stereoCalibrate(boardPointsBig, points_first, points_second, matrix_first, distortion_first, matrix_second, distortion_second, imageSize,
                                       rotation, translation, essential, fundamental, cv::CALIB_FIX_INTRINSIC, termCriteria);
    cout <<  "Extrinsic calibration ended"
         << "error = "  << error << endl;

    saveExtrinsicCameraParams(filename, rotation, translation, essential, fundamental);
    return error;
}

int main(int argc,const char* argv[])
{
    po::options_description description("Allowed options");
    description.add_options()
            ("help,h", "show options of program")
            ("path,p", po::value<std::string>(),
             "save calibration path, this string will be appended to all saved files at the beginning")
            ("mock,m", po::value<int>(),
             "mock the kinect input with given camera device, default camera device has index 0");;

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
    cv::resizeWindow("rgb",800,600);

    cv::namedWindow("ir", cv::WINDOW_NORMAL);
    cv::resizeWindow("ir",800,600);

    cv::namedWindow("depth", cv::WINDOW_NORMAL);
    cv::resizeWindow("depth",800,600);


    std::vector<std::vector<Point2f> > pointsColor, pointsIr;
    double error = 0;

    char key;
    do
    {
        KinectFrame frame;

        if(!mock)
            frame = getFrame(*cam);
        else
            frame = getFrameMock(capture.get());

        
        key = static_cast<char>(cv::waitKey(10));

        ChessboardPoints points = findChessboardPoints(frame);

        if(key == 'a')
        {            if(points.success)
            {
		
                // Refine the points in rgb
                Mat viewGray;
                cvtColor(frame.rgb, viewGray, COLOR_BGR2GRAY);
                cornerSubPix( viewGray, points.rgbPoints, Size(11,11),
                              Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1 ));

                pointsColor.push_back(points.rgbPoints);
                pointsIr.push_back(points.irPoints);

                if(pointsColor.size() >= MIN_REQUIRED_TO_CALIBRATE)
                {
                    // Those will be the matrices after the calibration is successful
                    cv::Mat colorCameraMatrix, irCameraMatrix;
                    cv::Mat colorDistCoefficients, irDistCoefficients;

                    // This will be the rotation and the translation between the cameras
                    cv::Mat rotation, translation, fundamental, essential;

                    bool success = runCalibrationAndSave(frame.rgb.size(), colorCameraMatrix, colorDistCoefficients, pointsColor,path+ "color_camera.yaml" );
                    success = success && runCalibrationAndSave(frame.ir.size(), irCameraMatrix, irDistCoefficients, pointsIr,path+ "ir_camera.yaml");
                    if(success)
                    {
                        // The size does not matter since both cameras are calibrated
                        error = runExtrinsicCalibrationAndSave(pointsColor, pointsIr,
                                                       colorCameraMatrix, irCameraMatrix,
                                                       colorDistCoefficients, irDistCoefficients, frame.rgb.size(),
                                                       rotation, translation, essential, fundamental,path+ "extrinsic.yaml");
                    }
                }
                saveFrame(&frame,path);
            }
        }

        putText(&frame,"'a' add current image for calibration",{10,20});

        if(pointsColor.size() >= MIN_REQUIRED_TO_CALIBRATE)
        {
            putText(&frame,"Current reprojection error is " + to_string(error),{10,40});
        }
        else
        {
            auto required = static_cast<int>(MIN_REQUIRED_TO_CALIBRATE - pointsColor.size());
            putText(&frame,"Images required to calibrate: " + to_string(required),{10,40});
        }
        drawChessboardCorners(&frame, points);
        showFrame(frame);

    } while(key != ESC_KEY);

}
