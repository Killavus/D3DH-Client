#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "type_definitions.h"
#include "server/binary_data_player.h"

typedef std::pair<std::string, int> PathPair;

static PathPair pathPair(const boost::filesystem::path& path) {
  std::string pathstr(path.filename().string());
  std::vector<std::string> pathparts;

  boost::iter_split(pathparts, pathstr, boost::first_finder("_batch_"));

  int seqno = boost::lexical_cast<int>(pathparts[1]);

  return std::make_pair(pathparts[0], seqno);
}

struct PathSort {
  bool operator()(
    const boost::filesystem::path &p1,
    const boost::filesystem::path &p2
  ) {
    PathPair p1pair(pathPair(p1));
    PathPair p2pair(pathPair(p2));

    return p1pair.second < p2pair.second ||
      (p1pair.second == p2pair.second &&
       boost::lexicographical_compare(p1pair.first, p2pair.first));
  }
};

BinaryDataPlayer::BinaryDataPlayer(const boost::filesystem::path& path) {
  for (auto &subpath : boost::filesystem::directory_iterator(path)) {
    filePaths.push_back(subpath);
  }

  std::sort(filePaths.begin(), filePaths.end(), PathSort());
  currentFile = filePaths.begin();
}

boost::optional<PackOfFrames> BinaryDataPlayer::getNextPackOfFrames() {
  if (currentFile == filePaths.end()) {
    return boost::none;
  }

  std::vector<boost::filesystem::path> toProcess;
  int seqno = pathPair(*currentFile).second;

  while(currentFile != filePaths.end() && pathPair(*currentFile).second == seqno) {
    toProcess.push_back(*currentFile);
    ++currentFile;
  }

  PackOfFrames result;

  for(auto &path : toProcess) {
    std::string hostname(pathPair(path).first);
    KinectData data = KinectData::load(path.string());

    result.insert({ hostname, data });
  }

  return result;
}
