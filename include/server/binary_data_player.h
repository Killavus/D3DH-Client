#ifndef INC_3DHUMANCAPTURE_BINARY_DATA_PLAYER
#define INC_3DHUMANCAPTURE_BINARY_DATA_PLAYER

#include <queue>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include "type_definitions.h"

class BinaryDataPlayer: public GenericPackOfFramesHandler {
public:
  BinaryDataPlayer(const boost::filesystem::path& path);

  boost::optional<PackOfFrames> getNextPackOfFrames() override;
private:
  std::vector<boost::filesystem::path> filePaths;
  std::vector<boost::filesystem::path>::iterator currentFile;
};
#endif // INC_3DHUMANCAPTURE_BINARY_DATA_PLAYER
