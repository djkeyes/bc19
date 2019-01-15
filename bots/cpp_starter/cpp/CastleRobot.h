

#ifndef CPP_STARTER_CASTLEROBOT_H
#define CPP_STARTER_CASTLEROBOT_H

#include "cpp_starter.h"
#include "UnitCounts.h"
#include "CommonRobot.h"

namespace bc19 {

/**
 * Rules for building new stuff, for bots that can build
 */
class BuildOrder {
 private:
  // Users must ensure that this pointer has a longer lifetime than this class
  AbstractNativeRobot *const self_;

  std::vector<specs::Unit> const *const initial_build_order_;
  static const std::vector<specs::Unit> PlCrCr_build_order;
  // 3 mage rush
  static const std::vector<specs::Unit> PcPcPcPl_build_order;
  // all econ
  static const std::vector<specs::Unit> PlPlPlPlPlPlPlPlPlPl_build_order;
  // 2 knight defence
  static const std::vector<specs::Unit> CrCrPlPlPlPlPlPl_build_order;
  // 3 knight defence
  static const std::vector<specs::Unit> CrCrCrPlPlPlPl_build_order;
  // 4 knight defence
  static const std::vector<specs::Unit> CrCrCrCrPlPl_build_order;
  // 4 knight defence, workers first
  static const std::vector<specs::Unit> PlPlCrCrCrCr_build_order;
  // 2 ranger defence?
  static const std::vector<specs::Unit> PhPhPlPlPlPlPl_build_order;
  // 3 ranger defence?
  static const std::vector<specs::Unit> PhPhPhPlPl_build_order;
  // 2 ranger + mage defence?
  static const std::vector<specs::Unit> PhPhPcPlPl_build_order;

  /**
   * The choice of the first few units is very important. If the round num is low enough, check nextToBuild instead
   * of building adaptively.
   * @return true if nextToBuild() to be called
   */
  bool shouldFollowInitialBuildOrder() const {
    return self_->me().unit() == specs::Unit::CASTLE && self_->me().turnCount() < 100;
  }

 public:
  explicit BuildOrder(AbstractNativeRobot *const this_robot)
      : self_(this_robot),
      initial_build_order_(self_->me().team() == 0 ? &PcPcPcPl_build_order : &PhPhPhPlPl_build_order) {
  }

  /**
   * @return the next robot type to build, or UNDEFINED if it is better to not build
   */
  specs::Unit nextToBuild(UnitCounts &unit_counts) const {
    if (shouldFollowInitialBuildOrder()) {
      int units_built = unit_counts.movableUnitsBuilt();
      if (units_built < initial_build_order_->size()) {
        return (*initial_build_order_)[units_built];
      }
    }
    // TODO: if the initial build is exhausted, switch to an adaptive strategy
    static int count = 0;
    ++count;
    count %= 3;
    if (count == 0) {
      return specs::Unit::PREACHER;
    } else if (count == 1) {
      return specs::Unit::PILGRIM;
    } else {
      return specs::Unit::PROPHET;
    }
  }
};

class CastleRobot : public CommonRobot {
 private:
  AbstractNativeRobot *const self_;
  BuildOrder build_order_;
  UnitCounts unit_counts_;

  emscripten::val tryBuilding(const specs::Unit &unit_type);
 public:
  explicit CastleRobot(const emscripten::val &jsAbstractRobot)
      : CommonRobot(jsAbstractRobot), self_(this), build_order_(this) {
  }

  emscripten::val onTurn() override;
};

}
#endif //CPP_STARTER_CASTLEROBOT_H
