

#ifndef CPP_STARTER_OFFENSIVEATTACK_H
#define CPP_STARTER_OFFENSIVEATTACK_H

#include <optional>
#include "Coordinate.h"
#include "cpp_starter.h"
#include "Pathfinder.h"

namespace bc19 {
/**
 * Utility that chooses an enemy target for us to attack
 */
class OffensiveAttack {
 private:
  AbstractNativeRobot const *const self_;

  std::optional<Coordinate> attack_coord_;

 public:
  explicit OffensiveAttack(AbstractNativeRobot const *const native_robot) : self_(native_robot) {
  }

  // TODO: refactor this to reduce the number of arguments. this function is gathering too many responsibilities.
  // notably: pathfinder is only necessary for bounds checks. nearby_enemies is arguably required, but
  // is_castle_under_attack should be packaged into some kind of context object if it's really necessary.
  double computeBestValueAttack(const specs::Unit &my_type,
                                Coordinate my_loc,
                                const Pathfinder &pathfinder,
                                const std::vector<Robot> &nearby_enemies,
                                bool is_castle_under_attack);

  bool withinAttackRadius(const specs::Unit &unit, const Coordinate::DimSqType &sq, bool is_castle_under_attack) const;

  emscripten::val createAttackAction();
};

}
#endif //CPP_STARTER_OFFENSIVEATTACK_H
