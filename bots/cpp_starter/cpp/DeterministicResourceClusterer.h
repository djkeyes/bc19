

#ifndef CPP_STARTER_DETERMINISTICRESOURCECLUSTERER_H
#define CPP_STARTER_DETERMINISTICRESOURCECLUSTERER_H

#include <utility>

#include "cpp_starter.h"
#include "Coordinate.h"

namespace bc19 {

class Cluster {
 private:
  std::vector<Coordinate> karbonite_mines_;
  std::vector<Coordinate> fuel_mines_;
  Coordinate centroid_;

 public:
  Cluster(std::vector<Coordinate> karbonite_mines, std::vector<Coordinate> fuel_mines, Coordinate centroid)
      : karbonite_mines_(std::move(karbonite_mines)), fuel_mines_(std::move(fuel_mines)), centroid_(centroid) {
  }

  const Coordinate &centroid() const {
    return centroid_;
  }

  const std::vector<Coordinate> &karbonite_mines() const {
    return karbonite_mines_;
  }

  const std::vector<Coordinate> &fuel_mines() const {
    return fuel_mines_;
  }
};

/**
 * In Battlecode 2019, resources are not distributed uniformly at random. Instead, they are generated in clusters,
 * with each cluster containing 1-4 karbonite and 1-4 fuel. (maybe 1-3?)
 *
 * This utility summarizes resources into clusters. It acts in a deterministic way, so the cluster labels are uniform
 * across all robots.
 */
class DeterministicResourceClusterer {
 private:
  std::vector<Cluster> clusters_;
 public:
  DeterministicResourceClusterer() = default;

  void init(const std::vector<Coordinate> &karbonite_coords, const std::vector<Coordinate> fuel_coords);

  const std::vector<Cluster> &clusters() const {
    return clusters_;
  }
};

}
#endif //CPP_STARTER_DETERMINISTICRESOURCECLUSTERER_H
