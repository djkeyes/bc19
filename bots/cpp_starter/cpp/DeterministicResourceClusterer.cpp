

#include "DeterministicResourceClusterer.h"

namespace bc19 {

// For a give cluster, if we pick two pairs and compair their centroids, they should be pretty close
// This is heuristic--in some extremely rare cases, it's possible that we might accidently split a cluster.
static constexpr Coordinate::DimSqType max_inter_cluster_dist_sq = 5 * 5;

class ClusterBuilder {
 private:
  int totalRows_;
  int totalCols_;
  int totalElements_;

  std::vector<Coordinate> karbonite_mines_;
  std::vector<Coordinate> fuel_mines_;

  static constexpr std::vector<Coordinate>::size_type max_resource_amount = 4;

  void countCoord(const Coordinate &coord) {
    ++totalElements_;
    totalCols_ += coord.col_;
    totalRows_ += coord.row_;
  }

 public:
  ClusterBuilder() : totalRows_(0), totalCols_(0), totalElements_(0) {
    karbonite_mines_.reserve(max_resource_amount);
    fuel_mines_.reserve(max_resource_amount);
  }

  void addKarbonite(Coordinate coord) {
    countCoord(coord);
    karbonite_mines_.emplace_back(coord);
  }

  void addFuel(Coordinate coord) {
    countCoord(coord);
    fuel_mines_.emplace_back(coord);
  }

  Coordinate centroid() const {
    return Coordinate(static_cast<Coordinate::DimType>(totalRows_ / totalElements_),
                      static_cast<Coordinate::DimType>(totalCols_ / totalElements_));
  }

  Cluster toCluster() {
    // note: this invalidates this cluster
    return Cluster(std::move(karbonite_mines_), std::move(fuel_mines_), centroid());
  }
};

void DeterministicResourceClusterer::init(const std::vector<Coordinate> &karbonite_coords,
                                          const std::vector<Coordinate> fuel_coords) {
  // this number seems to have a max value of 20. Add a little buffer in case my math is wrong.
  // based on https://github.com/battlecode/battlecode19/blob/master/coldbrew/game.js#L193
  static constexpr int max_expected_clusters_per_map = 22;
  std::vector<ClusterBuilder> clusters_so_far;
  clusters_so_far.reserve(max_expected_clusters_per_map);

  std::vector<int> fuel_assignments(fuel_coords.size(), -1);
  // each cluster seems to have at least 1 karbonite and 1 fuel, so match those first
  for (auto karb : karbonite_coords) {
    auto min_dist_sq = std::numeric_limits<Coordinate::DimSqType>::max();
    int closest_fuel = 0;
    for (int fuel_idx = 0; fuel_idx < fuel_coords.size(); ++fuel_idx) {
      const auto &fuel = fuel_coords[fuel_idx];
      const auto dist_sq = karb.distSq(fuel);
      if (dist_sq < min_dist_sq) {
        min_dist_sq = dist_sq;
        closest_fuel = fuel_idx;
      }
    }
    if (fuel_assignments[closest_fuel] == -1) {
      // the closest fuel isn't in a cluster yet--make a new cluster
      fuel_assignments[closest_fuel] = static_cast<const int>(clusters_so_far.size());
      ClusterBuilder cluster;
      cluster.addFuel(fuel_coords[closest_fuel]);
      cluster.addFuel(karb);

      // it's possible that there are 2 fuels and 2 karbonites near here. check if we should merge.
      ClusterBuilder *closest_cluster = nullptr;
      auto min_clust_dist_sq = max_inter_cluster_dist_sq + 1;
      const auto centroid = cluster.centroid();
      for (auto &prev_cluster: clusters_so_far) {
        const auto clust_dist_sq = centroid.distSq(prev_cluster.centroid());
        if (clust_dist_sq < min_clust_dist_sq) {
          min_clust_dist_sq = clust_dist_sq;
          closest_cluster = &prev_cluster;
        }
      }

      if (closest_cluster == nullptr) {
        // nope, nothing nearby
        clusters_so_far.emplace_back(cluster);
      } else {
        // already a similar cluster to ours
        closest_cluster->addKarbonite(karb);
        closest_cluster->addFuel(fuel_coords[closest_fuel]);
      }
    } else {
      // the closest fuel has a cluster--join it
      clusters_so_far[fuel_assignments[closest_fuel]].addKarbonite(karb);
    }
  }
  // check if we missed any fuels
  for (int fuel_idx = 0; fuel_idx < fuel_coords.size(); ++fuel_idx) {
    const auto &fuel = fuel_coords[fuel_idx];
    if (fuel_assignments[fuel_idx] == -1) {
      ClusterBuilder *closest_cluster = nullptr;
      auto min_dist_sq = max_inter_cluster_dist_sq + 1;
      for (auto &cluster: clusters_so_far) {
        const auto dist_sq = fuel.distSq(cluster.centroid());
        if (dist_sq < min_dist_sq) {
          min_dist_sq = dist_sq;
          closest_cluster = &cluster;
        }
      }
      if (closest_cluster == nullptr) {
        // this shouldn't happen, unless the generation routine has changed dramatically
        ClusterBuilder cb;
        cb.addFuel(fuel);
        clusters_so_far.emplace_back(cb);
      } else {
        closest_cluster->addFuel(fuel);
      }
    }
  }

  for (auto &cluster_builder: clusters_so_far) {
    clusters_.emplace_back(cluster_builder.toCluster());
  }
}
}