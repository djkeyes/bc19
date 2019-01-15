

#ifndef CPP_STARTER_ATTACKERROBOT_H
#define CPP_STARTER_ATTACKERROBOT_H

#include "CommonRobot.h"
#include "Pathfinder.h"

namespace bc19 {

class AttackerRobot : public CommonRobot {
 private:
  Pathfinder pathfinder_;

  std::vector<Robot> nearby_enemies_;
  Grid<double> tile_values_;
  Grid<uint8_t> tile_versions_;
  uint8_t version_;
  std::optional<Coordinate> attack_coord_;

  std::vector<Robot> getNearbyEnemies() const;
  emscripten::val drawValueMaps();

 public:
  explicit AttackerRobot(const emscripten::val &jsAbstractRobot)
      : CommonRobot(jsAbstractRobot), pathfinder_(this), version_(1) {
    const auto &map = getPassableMap();
    tile_values_.resize(map.rows(), map.cols());
    tile_versions_.resize(map.rows(), map.cols());
  }

  emscripten::val onTurn() override;
  static constexpr bool withinAttackRadius(const specs::Unit &unit, const Coordinate::DimSqType &sq);
  double computeBestValueAttack(const specs::Unit &my_type, Coordinate my_loc);
  double computeBestValueNextAttack(const specs::Unit &my_type, Coordinate tile);
};
}
#endif //CPP_STARTER_ATTACKERROBOT_H
