#ifndef INC_3DHUMANCAPTURE_SERVER_TYPE_DEFINITIONS_H
#define INC_3DHUMANCAPTURE_SERVER_TYPE_DEFINITIONS_H

#include <mutex>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <map>
#include <vector>

#include <boost/optional.hpp>
#include <opencv2/core.hpp>

// <ip, port>
using Endpoint = std::pair<std::string, uint16_t>;
using KinectId = std::string;
using RawImage = std::vector<unsigned char>;
using timeType = std::int64_t;
using Times = std::vector<timeType>;

enum class ImageType
{
    RGB,
    DEPTH,
    IR,
};

enum class Mode
{
    SEND_ALL,
    SEND_DEPTH,
};

struct Image
{
    Image();
    Image(RawImage img, int w, int h);

    template <class Archive>
    void serialize(Archive &ar,
                   __attribute__((unused)) const unsigned int version)
    {
        ar &img;
        ar &width;
        ar &height;
    }

    RawImage img;
    int width, height;
};

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

struct KinectData
{
    KinectData();
    KinectData(RawImage rgb, size_t rgbW, size_t rgbH,
               RawImage depth, size_t depthW, size_t depthH,
               RawImage ir, size_t irW, size_t irH, timeType time);

    KinectData(RawImage depth, size_t depthW, size_t depthH,
               timeType time);

    void saveAsBinary(std::string fileName);
    static KinectData load(std::string fileName);

    template <class Archive>
    void serialize(Archive &ar,
                   __attribute__((unused)) const unsigned int version)
    {
        ar &images;
        ar &timestamp;
    }

    std::unordered_map<ImageType, Image, EnumClassHash> images;
    timeType timestamp;
};

class ClientToFramesMapping
{
    // !!Assumption!!
    // in order not to have mutex for idToData I want
    // initData to be called only once and before any other functions

  public:
    void initData(std::vector<KinectId> kinectsIds);
    void putFrame(const KinectId &kinId, KinectData data);
    bool empty(const KinectId &kinId);
    KinectData &peekFirstFrame(const KinectId &kinId);
    KinectData removeFirstFrame(const KinectId &kinId);

  private:
    void dataAssert(const KinectId &kinId);

    using FrameCollection = std::queue<KinectData>;
    std::unordered_map<KinectId, FrameCollection> idToData;
    std::unordered_map<KinectId, std::mutex> dataMutex;
};

using PackOfFrames = std::unordered_map<KinectId, KinectData>;
using PackOfTimeStamps = std::unordered_map<KinectId, timeType>;
using TimeLine = std::map<timeType, KinectData>;

class GenericPackOfFramesHandler {
public:
  virtual ~GenericPackOfFramesHandler() = default;
  virtual boost::optional<PackOfFrames> getNextPackOfFrames() = 0;
};

class PackOfFramesHandler : public GenericPackOfFramesHandler
{
  public:
    PackOfFramesHandler(std::uint64_t maxDistBetweenFramesInBatch,
                        std::size_t numberOfKinects,
                        std::size_t minNumberOfFramesInPackageToAccept,
                        timeType windowStartPos);

    void putFrame(const KinectId &kinectId, KinectData data);
    boost::optional<PackOfFrames> getNextPackOfFrames() override;

  private:
    std::uint64_t getFrameId(std::uint64_t frameID);

    std::mutex timeLinesMutex;
    std::mutex readyPackOfFramesMutex;

    std::unordered_map<KinectId, TimeLine> timeLines;
    std::unordered_map<KinectId, timeType> lastTimestamps;
    std::queue<PackOfFrames> readyPackOfFrames;
    std::uint64_t maxDistBetweenFramesInBatch;
    std::size_t numberOfKinects;
    std::size_t minNumberOfFramesInPackageToAccept;
    timeType windowStartPos;
};

struct CamerasExtrinsic
{
    /*
    Color camera is unrotated & positioned at (0, 0) camera space.
    Extrinsics below are about IR camera.
  */

    cv::Mat rotation;
    cv::Mat translation;

    cv::Mat fundamental;
    cv::Mat essential;
};

struct WorldExtrinsic
{
    cv::Mat rotation;
    cv::Mat translation;
};

struct CameraCalibration
{
    cv::Mat colorIntrinsic;
    cv::Mat irIntrinsic;
    cv::Mat irDistortion;
    cv::Mat colorDistortion;

    CamerasExtrinsic camerasExtrinsic;
    WorldExtrinsic worldExtrinsicColor;
    WorldExtrinsic worldExtrinsicIr;

    bool _irExtrinsicCalculated;

    CameraCalibration() : _irExtrinsicCalculated(false) {}

    void calculateIrExtrinsic()
    {
        if (!_irExtrinsicCalculated)
        {
            worldExtrinsicIr.rotation = camerasExtrinsic.rotation * worldExtrinsicColor.translation + camerasExtrinsic.translation;
            worldExtrinsicIr.translation = worldExtrinsicIr.rotation.t() * worldExtrinsicIr.translation + worldExtrinsicIr.rotation.t() * camerasExtrinsic.rotation.t() * camerasExtrinsic.translation;
            _irExtrinsicCalculated = true;
        }
    }
};

using CameraCalibrationsMap = std::unordered_map<KinectId, CameraCalibration>;

struct PointCloud
{
    static inline size_t cloudSize() { return 512 * 424; }
    std::vector<cv::Point3f> points;
};

#endif //INC_3DHUMANCAPTURE_SERVER_TYPE_DEFINITIONS_H
