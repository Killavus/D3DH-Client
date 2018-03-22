//
// Created by shrum on 14.03.18.
//

#include <iostream>

#include "Camera.h"
#include "calibration/calibration_window.h"
#include "opencv2/opencv.hpp"
#include <vector>

using namespace std;
using namespace cv;

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
            corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));

}

// This will calibrate the camera. The result will be in cameraMatrix and distCoeffs.
static bool runCalibration( Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                            vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                            vector<float>& reprojErrs,  double& totalAvgErr)
{

    cameraMatrix = Mat::eye(3, 3, CV_64F);

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(Size(9,6), 50, objectPoints[0]);

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
void runExtrinsicCalibrationAndSave(vector< vector<Point2f> >& points_first,
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
    calcBoardCornerPositions(Size(9,6),50,boardPoints);
    vector<vector<Point3f> > boardPointsBig;
    for(int i = 0; i < points_first.size();i++)
	boardPointsBig.push_back(boardPoints);
    const cv::TermCriteria termCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, DBL_EPSILON);

    double error = cv::stereoCalibrate(boardPointsBig, points_first, points_second, matrix_first, distortion_first, matrix_second, distortion_second, imageSize,
                                       rotation, translation, essential, fundamental, cv::CALIB_FIX_INTRINSIC, termCriteria);
    cout <<  "Extrinsic calibration ended"
         << "error = "  << error << endl;

    saveExtrinsicCameraParams(filename, rotation, translation, essential, fundamental);
}

// This code does not yet calibrate the extrinsic parameters. It works only for 9 by 6 chessboard.
// The chessboard can be downloaded here: https://docs.opencv.org/3.0-beta/_downloads/pattern.png
// The guide used to achieve this code is also there.

// To use this one has to press c in order to capture the photo of the chessboard. When there are enough photos
// The calibration will start. Enough is 4 for now. The code needs to be rewritten (and fixed) as soon as we discuss
// the structure of the project.
int main()
{
    // Open some simple glfw window. This can be later substituted with nano (https://github.com/wjakob/nanogui)
    // and other cool guis.
    Window window(600,400,"Best kinect software ever");

    // Poll the events once before the main loop.
    window.poll_events();

    // Do the magic with kinect
    libfreenect2::Freenect2 freenect;
    int kinect_count = freenect.enumerateDevices();

    if (kinect_count == 0) {
        std::cerr << "No Kinect device connected. Exiting..." << std::endl;
        std::exit(-1);
    }

    Camera cam(freenect, 0);
    libfreenect2::FrameMap frame_map;
    libfreenect2::Frame *rgb, *ir, *depth;

    // Those will be the data from kinect in opencv form
    cv::Mat rgbmat, irmat, depthmat;

    // These two will indicate wether we found the patterns in respective cameras in this capture
    bool found_color = false;
    bool found_ir = false;


    // The captured points if the chessboard was found.
    std::vector<std::vector<cv::Point2f> > points_color, points_ir;

    // Those will be the matrices after the calibration is successful
    cv::Mat colorCameraMatrix, irCameraMatrix;
    cv::Mat colorDistCoefficients, irDistCoefficients;

    // This will be the rotation and the translation between the cameras
    cv::Mat rotation, translation, fundamental, essential;

    bool collecting_photo = false;
    do
    {
        // If c is pressed then capture stuff.
        if((glfwGetKey(window.getGLFWwindow(),GLFW_KEY_C) == GLFW_PRESS
           || glfwGetKey(window.getGLFWwindow(),GLFW_KEY_C) == GLFW_REPEAT) && !collecting_photo)
        {
            std::cout << "Pressed!" << std::endl;
            collecting_photo = true;
            if (!cam.getFrame(frame_map)) {
                std::cout << "Failed to get frame." << std::endl;
            }

            // Extract the frame
            rgb = frame_map[libfreenect2::Frame::Color];
            ir = frame_map[libfreenect2::Frame::Ir];
            depth = frame_map[libfreenect2::Frame::Depth];

            // Translate to OpenCV
            cv::Mat(rgb->height, rgb->width, CV_8UC4, rgb->data).copyTo(rgbmat);
            cv::Mat(ir->height, ir->width, CV_32FC1, ir->data).copyTo(irmat);
            cv::Mat(depth->height, depth->width, CV_32FC1, depth->data).copyTo(depthmat);


            // Gather the points
            vector<cv::Point2f> pointBufColor;
            vector<cv::Point2f> pointBufIr;

            found_color = cv::findChessboardCorners(rgbmat,cv::Size(9,6),pointBufColor, cv::CALIB_CB_FAST_CHECK);
            found_ir = cv::findChessboardCorners(rgbmat,cv::Size(9,6),pointBufIr, CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK);

            std::cout << "rgb: " << rgb->timestamp << " | ir: " << ir->timestamp << " | depth: " << depth->timestamp << std::endl;

            if(found_color && found_ir)
            {
                std::cout << "Both cameras found chessboard." << std::endl;
                std::cout << "It would be nice to draw the pictures :( but no picture showing :(" << std::endl;
                
                imwrite("rgbmat" + to_string(points_color.size())+".jpg",rgbmat);
		// This will show the minimum and maximum values 
		// because there are some problems with the images
		double min,max;
		minMaxLoc(irmat,&min,&max);
		std::cout << "Minmax in ir " << min << " " << max << std::endl;
		minMaxLoc(depthmat,&min, &max);
		std::cout << "Minmax in depth " << min << " " << max << std::endl;

		// This will convert the images to printable mode and save them
		Mat new_ir;
		irmat.convertTo(new_ir,CV_16U);

		Mat new_depth;
		depthmat.convertTo(new_depth,CV_16U);
		
                imwrite("irmat" + to_string(points_color.size())+".jpg",new_ir);
                imwrite("depthmat" + to_string(points_color.size())+".jpg",new_depth);

                Mat viewGray;
                cvtColor(rgbmat, viewGray, COLOR_BGR2GRAY);
                cornerSubPix( viewGray, pointBufColor, Size(11,11),
                              Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1 ));

                points_color.push_back(pointBufColor);
                points_ir.push_back(pointBufIr);

                if(points_color.size() > 3)
                {
                    bool success = runCalibrationAndSave(rgbmat.size(), colorCameraMatrix, colorDistCoefficients, points_color, "color_camera.yaml" );
                    success = success && runCalibrationAndSave(irmat.size(), irCameraMatrix, irDistCoefficients, points_ir, "ir_camera.yaml");
                    if(success)
                    {
                        runExtrinsicCalibrationAndSave(points_color, points_ir,
                                                       colorCameraMatrix, irCameraMatrix,
                                                        colorDistCoefficients, irDistCoefficients, rgbmat.size(),
                                                       rotation, translation, essential, fundamental, "extrinsic.yaml");
                    }
                }
            }

            cam.releaseFrame(frame_map);
        } else if(glfwGetKey(window.getGLFWwindow(),GLFW_KEY_C) == GLFW_RELEASE && collecting_photo)
        {
            collecting_photo = false;
            std::cout << "Released!" << std::endl;
        }

        window.clear_color();

        window.swap_buffers();
        window.poll_events();
    } while( glfwGetKey(window.getGLFWwindow(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
             glfwWindowShouldClose(window.getGLFWwindow()) == 0 );
    return 0;
}
