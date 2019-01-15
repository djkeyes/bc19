

#include "PilgrimRobot.h"

namespace bc19 {

emscripten::val PilgrimRobot::onTurn() {
  // TODO: consider building a church if we go far away. Especially if we go >=45 units away.
  if (gathering_) {
    if (!hasCluster()) {
      chooseCluster();
    }
    if (farFromCluster()) {
      return pathTowardCluster();
    }

    if (!reachedResource()) {
      resource_target_ = chooseResourceLocation();
    }
    if (resource_target_) {
      if (reachedResource()) {
        if (cargoFull()) {
          gathering_ = false;
          resource_target_.reset();
        } else {
          return mine();
        }
      } else {
        return pathTowardResource();
      }
    } else {
      chooseNewCluster();
      return pathTowardCluster();
    }
  }
  if (!gathering_) {
    if (!adjacentToDepot()) {
      depot_target_ = chooseDepot();
    }
    if (!adjacentToDepot()) {
      return pathTowardDepot();
    } else {
      gathering_ = true;
      return dropOffResources();
    }
  }

  return nullAction();
}

bool PilgrimRobot::needKarboniteMoreThanFuel() const {
  // TODO: our fuel demand scales based on army size. castles can determine the right thing to gather.
  // most things cost 5x more karbonite than fuel
  // In a tie, get karbonite to bootstrap
  return karbonite() * 5 <= fuel();
}

bool PilgrimRobot::hasCluster() const {
  return current_cluster_ != -1;
}

void PilgrimRobot::chooseNewCluster(int cluster_to_skip) {
  // choose the closest cluster
  // TODO: use bfs distance here
  // TODO: create an all-pairs-shortest-path datastruction, and incrementally update it
  const auto &clusters = clusterer_.clusters();
  int closest_cluster_idx = -1;
  auto min_dist_sq = std::numeric_limits<Coordinate::DimSqType>::max();
  Coordinate my_loc(static_cast<Coordinate::DimType>(me().y()), static_cast<Coordinate::DimType>(me().x()));
  for (int cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
    if (cluster_idx == cluster_to_skip) {
      continue;
    }
    const auto dist_sq = my_loc.distSq(clusters[cluster_idx].centroid());
    if (dist_sq < min_dist_sq) {
      min_dist_sq = dist_sq;
      closest_cluster_idx = cluster_idx;
    }
  }
  current_cluster_ = closest_cluster_idx;
}

void PilgrimRobot::chooseCluster() {
  chooseNewCluster(-1);
}

void PilgrimRobot::chooseNewCluster() {
  chooseNewCluster(current_cluster_);
}

bool PilgrimRobot::farFromCluster() const {
  static constexpr Coordinate::DimSqType cluster_close_dist_sq = 7 * 7;
  Coordinate my_loc(static_cast<Coordinate::DimType>(me().y()), static_cast<Coordinate::DimType>(me().x()));
  return clusterer_.clusters()[current_cluster_].centroid().distSq(my_loc) > cluster_close_dist_sq;
}

emscripten::val PilgrimRobot::pathTowardCluster() {
  return pathfinder_.pathTowardCheaply(pathfinder_.getNearbyPassableTile(clusterer_.clusters()[current_cluster_].centroid()));
}

std::optional<Coordinate> PilgrimRobot::chooseResourceLocation() const {
  // check each resource location for occupancy
  // TODO: also sort by distance
  const auto &cluster = clusterer_.clusters()[current_cluster_];
  std::vector<Coordinate> const *betterResource;
  std::vector<Coordinate> const *worseResource;
  if (needKarboniteMoreThanFuel()) {
    betterResource = &cluster.karbonite_mines();
    worseResource = &cluster.fuel_mines();
  } else {
    betterResource = &cluster.fuel_mines();
    worseResource = &cluster.karbonite_mines();
  }
  const auto &map = getVisibleRobotMap();
  Coordinate my_loc(static_cast<Coordinate::DimType>(me().y()), static_cast<Coordinate::DimType>(me().x()));
  // depending on the exact cluster layout, this may be out of vision
  // if it's too far, be optimistic. We can re-adjust next turn.
  const auto &vision_range_sq = specs::units[static_cast<int>(me().unit())].vision_radius;
  for (const auto &loc : *betterResource) {
    if (my_loc.distSq(loc) > vision_range_sq) {
      return std::optional<Coordinate>(loc);
    }
    if (map.get(loc.row_, loc.col_) == 0) {
      return std::optional<Coordinate>(loc);
    }
  }
  for (const auto &loc : *worseResource) {
    if (my_loc.distSq(loc) > vision_range_sq) {
      return std::optional<Coordinate>(loc);
    }
    if (map.get(loc.row_, loc.col_) == 0) {
      return std::optional<Coordinate>(loc);
    }
  }
  return std::optional<Coordinate>();
}

bool PilgrimRobot::reachedResource() const {
  Coordinate my_loc(static_cast<Coordinate::DimType>(me().y()), static_cast<Coordinate::DimType>(me().x()));
  return my_loc == resource_target_;
}

bool PilgrimRobot::cargoFull() const {
  const auto &m = me();
  const auto &unit = m.unit();
  return (m.fuel() == specs::units[static_cast<int>(unit)].fuel_capacity)
      || (m.karbonite() == specs::units[static_cast<int>(unit)].karbonite_capacity);
}

emscripten::val PilgrimRobot::pathTowardResource() {
  return pathfinder_.pathTowardQuickly(*resource_target_);
}

bool PilgrimRobot::adjacentToDepot() const {
  Coordinate my_loc(static_cast<Coordinate::DimType>(me().y()), static_cast<Coordinate::DimType>(me().x()));
  return depot_target_.distSq(my_loc) <= 2;
}

emscripten::val PilgrimRobot::pathTowardDepot() {
  return pathfinder_.pathTowardQuickly(depot_target_);
}

emscripten::val PilgrimRobot::dropOffResources() {
  const auto &m = me();
  Coordinate my_loc(static_cast<Coordinate::DimType>(m.y()), static_cast<Coordinate::DimType>(m.x()));
  int dx = depot_target_.col_ - my_loc.col_;
  int dy = depot_target_.row_ - my_loc.row_;
  return give(dx, dy, m.karbonite(), m.fuel());
}

Coordinate PilgrimRobot::chooseDepot() const {
  // check visible robots
  const auto &m = me();
  const auto &us = m.team();
  Coordinate my_loc(static_cast<Coordinate::DimType>(m.y()), static_cast<Coordinate::DimType>(m.x()));
  // TODO: use bfs dist here
  const auto &visible_robots = getVisibleRobots();
  const auto &length = visible_robots["length"].as<int>();
  int min_dist_sq = -1;
  Coordinate closest_coord(0, 0);
  for (int i = 0; i < length; ++i) {
    const Robot robot(visible_robots[i]);
    if (robot.team() == us) {
      const auto &type = robot.unit();
      if (type == specs::Unit::CASTLE || type == specs::Unit::CHURCH) {
        Coordinate coord(static_cast<Coordinate::DimType>(robot.y()), static_cast<Coordinate::DimType>(robot.x()));
        const auto dist_sq = coord.distSq(my_loc);
        if (min_dist_sq == -1 || dist_sq < min_dist_sq) {
          min_dist_sq = dist_sq;
          closest_coord = coord;
        }
      }
    }
  }
  if (min_dist_sq != -1) {
    return closest_coord;
  }

  // no visible robots? try our start position (unless we're already pretty close)
  if (my_loc.distSq(start_loc_) >= specs::units[static_cast<int>(m.unit())].vision_radius - 21) {
    return start_loc_;
  }
  const auto &map = getKarboniteMap();
  const auto &rows = map.rows();
  const auto &cols = map.cols();
  // start position already pretty close? just chose anywhere outside of our vision range
  for (const auto dir :
      directions::horiz_adjacent) {
    Coordinate offset = my_loc + Coordinate(static_cast<Coordinate::DimType>(dir.first * 11),
                                            static_cast<Coordinate::DimType>(dir.second * 11));
    if (offset.row_ >= 0 && offset.col_ >= 0 && offset.row_ < rows && offset.col_ < cols) {
      return offset;
    }
  }
  // wat :S
  return Coordinate(0, 0);
}

}