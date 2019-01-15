

#ifndef CPP_STARTER_ATTACKERROBOT_H
#define CPP_STARTER_ATTACKERROBOT_H

#include "CommonRobot.h"
#include "Pathfinder.h"
#include "MapSymmetry.h"

namespace bc19 {

class AttackerRobot : public CommonRobot {
 private:
  Pathfinder pathfinder_;
  MapSymmetry symmetry_;
  DeterministicResourceClusterer clusterer_;

  std::vector<Robot> nearby_enemies_;
  std::optional<Coordinate> nearby_castle_;
  bool is_castle_under_attack_;
  Grid<double> tile_values_;
  Grid<uint8_t> tile_versions_;
  uint8_t version_;
  std::optional<Coordinate> attack_coord_;

  int cluster_index_ = 0;
  int turns_pathing_ = 0;

  void parseNearbyUnits();
  emscripten::val drawValueMaps();

 public:
  explicit AttackerRobot(const emscripten::val &jsAbstractRobot)
      : CommonRobot(jsAbstractRobot),
      pathfinder_(this),
      symmetry_(pathfinder_),
      is_castle_under_attack_(false),
      version_(1) {
    const auto &map = getPassableMap();
    tile_values_.resize(map.rows(), map.cols());
    tile_versions_.resize(map.rows(), map.cols());

    clusterer_.init(mineMapToVec(getKarboniteMap()), mineMapToVec(getFuelMap()));
  }

  emscripten::val onTurn() override;
  bool withinAttackRadius(const specs::Unit &unit, const Coordinate::DimSqType &sq);
  double computeBestValueAttack(const specs::Unit &my_type, Coordinate my_loc);
  double computeBestValueNextAttack(const specs::Unit &my_type, Coordinate tile);
};
}
#endif //CPP_STARTER_ATTACKERROBOT_H
