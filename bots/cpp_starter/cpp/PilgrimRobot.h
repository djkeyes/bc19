

#ifndef CPP_STARTER_PILGRIMROBOT_H
#define CPP_STARTER_PILGRIMROBOT_H

#include <unordered_map>
#include <optional>
#include "CommonRobot.h"
#include "DeterministicResourceClusterer.h"
#include "Pathfinder.h"

namespace bc19 {
class PilgrimRobot : public CommonRobot {
 private:
  DeterministicResourceClusterer clusterer_;
  Pathfinder pathfinder_;

  // state for mining behavior
  // TODO: if these states are too complicated, maybe we should switch to command pattern.
  bool gathering_;
  int current_cluster_;
  std::optional<Coordinate> resource_target_;
  Coordinate depot_target_;
  Coordinate start_loc_;

  bool hasCluster() const;
  void chooseNewCluster(int cluster_to_skip);
  void chooseNewCluster();
  void chooseCluster();
  bool farFromCluster() const;
  emscripten::val pathTowardCluster();
  std::optional<Coordinate> chooseResourceLocation() const;
  bool needKarboniteMoreThanFuel() const;
  bool reachedResource() const;
  bool cargoFull() const;
  emscripten::val pathTowardResource();
  bool adjacentToDepot() const;
  emscripten::val pathTowardDepot();
  emscripten::val dropOffResources();
  Coordinate chooseDepot() const;

 public:
  explicit PilgrimRobot(const emscripten::val &jsAbstractRobot)
      : CommonRobot(jsAbstractRobot),
      pathfinder_(this),
      gathering_(true),
      current_cluster_(-1),
      depot_target_(0, 0),
      start_loc_(static_cast<Coordinate::DimType>(me().y()), static_cast<Coordinate::DimType>(me().x())) {
    clusterer_.init(mineMapToVec(getKarboniteMap()), mineMapToVec(getFuelMap()));
  }

  emscripten::val onTurn() override;
};
}

#endif //CPP_STARTER_PILGRIMROBOT_H
