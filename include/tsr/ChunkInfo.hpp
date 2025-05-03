#pragma once

#include <boost/container_hash/hash_fwd.hpp>
#include <boost/functional/hash.hpp>
#include <cstddef>
#include <functional>

namespace tsr {

struct ChunkInfo {
  double minLat;
  double minLng;
  double maxLat;
  double maxLng;

  bool operator==(const ChunkInfo &other) const {
    return (other.minLat == this->minLat && other.minLng == this->minLng &&
            other.maxLat == this->maxLat && other.maxLng == this->maxLng);
  }
};

} // namespace tsr

namespace boost {
template <> struct hash<tsr::ChunkInfo> {
  std::size_t operator()(const tsr::ChunkInfo &chunk) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, chunk.minLat);
    boost::hash_combine(seed, chunk.minLng);
    boost::hash_combine(seed, chunk.maxLat);
    boost::hash_combine(seed, chunk.maxLng);
    return seed;
  }
};
} // namespace boost

namespace std {
template <> struct hash<tsr::ChunkInfo> {
  std::size_t operator()(const tsr::ChunkInfo &chunk) const {
    return boost::hash<tsr::ChunkInfo>()(chunk);
  }
};
} // namespace std
