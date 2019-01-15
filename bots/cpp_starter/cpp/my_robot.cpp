#include <vector>

#include "cpp_starter.h"
#include "fast_rand.h"
#include "UnitCounts.h"
#include "CastleTalker.h"

using std::to_string;
using std::unique_ptr;
using std::make_unique;
using std::pair;

namespace bc19 {

using specs::Unit;

namespace directions {
// TODO: make some constexpr functions that can compute lists of tile displacements at compiletime

constexpr std::array<pair<int, int>, 4> horiz_adjacent = {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}};
constexpr std::array<pair<int, int>, 8>
    adjacent_spiral = {{{-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}}};
}

/**
 * Short and long range navigation, for bots that can move
 */
//class NavRobot {
// private:
//  // Users must ensure that this pointer has a longer lifetime than this class
//  AbstractNativeRobot *const self_;
//
// public:
//  explicit NavRobot(AbstractNativeRobot *const this_robot) : self_(this_robot) {}
//};


/**
 * Rules for building new stuff, for bots that can build
 */
class BuildOrder {
 private:
  // Users must ensure that this pointer has a longer lifetime than this class
  AbstractNativeRobot *const self_;

  std::vector<Unit> const *const initial_build_order_;
  static const std::vector<Unit> PlCrCr_build_order;
  // 3 mage rush
  static const std::vector<Unit> PhPhPhPc_build_order;
  // all econ
  static const std::vector<Unit> PlPlPlPlPlPlPlPlPlPl_build_order;
  // 2 knight defence
  static const std::vector<Unit> CrCrPlPlPlPlPlPl_build_order;
  // 3 knight defence
  static const std::vector<Unit> CrCrCrPlPlPlPl_build_order;
  // 4 knight defence
  static const std::vector<Unit> CrCrCrCrPlPl_build_order;
  // 4 knight defence, workers first
  static const std::vector<Unit> PlPlCrCrCrCr_build_order;
  // 2 ranger defence?
  static const std::vector<Unit> PcPcPlPlPlPlPl_build_order;
  // 3 ranger defence?
  static const std::vector<Unit> PcPcPcPlPl_build_order;
  // 2 ranger + mage defence?
  static const std::vector<Unit> PcPcPhPlPl_build_order;

  /**
   * The choice of the first few units is very important. If the round num is low enough, check nextToBuild instead
   * of building adaptively.
   * @return true if nextToBuild() to be called
   */
  bool shouldFollowInitialBuildOrder() const {
    return self_->me().unit() == Unit::CASTLE && self_->me().turnCount() < 100;
  }

 public:
  explicit BuildOrder(AbstractNativeRobot *const this_robot)
      : self_(this_robot), initial_build_order_(&PlCrCr_build_order) {
  }

  /**
   * @return the next robot type to build, or UNDEFINED if it is better to not build
   */
  Unit nextToBuild(UnitCounts &unit_counts) const {
    if (shouldFollowInitialBuildOrder()) {
      int units_built = unit_counts.movableUnitsBuilt();
      if (units_built < initial_build_order_->size()) {
        return (*initial_build_order_)[units_built];
      }
    }
    // TODO: if the initial build is exhausted, switch to an adaptive strategy
    return Unit::CRUSADER;
  }
};

const std::vector<Unit>
    BuildOrder::PlCrCr_build_order = {Unit::PILGRIM, Unit::CRUSADER, Unit::CRUSADER};

const std::vector<Unit>
    BuildOrder::PhPhPhPc_build_order = {Unit::PROPHET, Unit::PROPHET, Unit::PROPHET, Unit::PILGRIM};
const std::vector<Unit>
    BuildOrder::PlPlPlPlPlPlPlPlPlPl_build_order = {
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM
};
const std::vector<Unit>BuildOrder::CrCrPlPlPlPlPlPl_build_order = {
    Unit::CRUSADER,
    Unit::CRUSADER,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM
};
const std::vector<Unit>BuildOrder::CrCrCrPlPlPlPl_build_order =
    {Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM};
const std::vector<Unit>BuildOrder::CrCrCrCrPlPl_build_order =
    {Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER, Unit::PILGRIM, Unit::PILGRIM};
const std::vector<Unit>BuildOrder::PlPlCrCrCrCr_build_order =
    {Unit::PILGRIM, Unit::PILGRIM, Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER};
const std::vector<Unit>BuildOrder::PcPcPlPlPlPlPl_build_order =
    {Unit::PREACHER, Unit::PREACHER, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM};
const std::vector<Unit>
    BuildOrder::PcPcPcPlPl_build_order = {Unit::PREACHER, Unit::PREACHER, Unit::PREACHER, Unit::PILGRIM, Unit::PILGRIM};
const std::vector<Unit>
    BuildOrder::PcPcPhPlPl_build_order = {Unit::PREACHER, Unit::PREACHER, Unit::PROPHET, Unit::PILGRIM, Unit::PILGRIM};

class CommonRobot : public AbstractNativeRobot {
 protected:
  CastleTalker castle_talker_;

 public:
  explicit CommonRobot(const emscripten::val &jsAbstractRobot)
      : AbstractNativeRobot(jsAbstractRobot), castle_talker_(this) {
  }

  virtual void preTurn() {
  };

  virtual emscripten::val onTurn() = 0;

  virtual void postTurn() {
  };

  emscripten::val turn() final {
    commonPreTurn();
    preTurn();
    auto action = onTurn();
    commonPostTurn();
    postTurn();
    return action;
  }

 private:

  void commonPreTurn() {
  }

  void commonPostTurn() {
    castle_talker_.sendStagedMessage();
  }

};

class NoOpRobot : public CommonRobot {
 public:
  explicit NoOpRobot(const emscripten::val &jsAbstractRobot) : CommonRobot(jsAbstractRobot) {
  }

  emscripten::val onTurn() override {
    return nullAction();
  }
};

class CastleRobot : public CommonRobot {
 private:
  AbstractNativeRobot *const self_;
  BuildOrder build_order_;
  UnitCounts unit_counts_;
 public:
  explicit CastleRobot(const emscripten::val &jsAbstractRobot)
      : CommonRobot(jsAbstractRobot), self_(this), build_order_(this) {
  }

  emscripten::val onTurn() override {
    castle_talker_.processCastleTalks(unit_counts_);
    self_->log("time left at start of turn: " + to_string(self_->me().time()));

    // TODO(daniel): instead of creating actions, consider creating action generators, which are functors that
    // return an action. that way we can consider multiple actions, but only instantiate (and call JS bindings) for
    // the one best action.
    // Also consider making a std::optional<Action>, to indicate return code.
    const auto unit_type = build_order_.nextToBuild(unit_counts_);
    emscripten::val action = tryBuilding(unit_type);
    if (!action.isNull()) {
      return action;
    }

    return nullAction();
  }

  emscripten::val tryBuilding(const Unit &unit_type) {
    // look for an adjacent tile to build
    // TODO: take into account imminent danger, and don't build there

    if (this->karbonite() < specs::units[static_cast<int>(unit_type)].construction_karbonite
        || this->fuel() < specs::units[static_cast<int>(unit_type)].construction_fuel) {
      return nullAction();
    }

    const auto visible_map = this->getVisibleRobotMap();
    const auto passable_map = this->getPassableMap();
    const int rows = visible_map.rows();
    const int cols = visible_map.cols();

    const int curx = this->me().x();
    const int cury = this->me().y();
    for (const auto &dir : directions::adjacent_spiral) {
      const auto &dx = dir.first;
      const auto &dy = dir.second;
      const auto targetx = curx + dx;
      const auto targety = cury + dy;
      if (!(targetx >= 0 && targety >= 0 && targetx < cols && targety < rows)) {
        continue;
      }
      if (visible_map.get(targety, targetx) == 0 && passable_map.get(targety, targetx)) {
        castle_talker_.stageBuiltUnit(CastleTalker::builtMessageFromUnit(unit_type));
        return this->buildUnit(unit_type, dx, dy);
      }
    }
    return nullAction();
  }
};

class PilgrimRobot : public CommonRobot {
 public:
  explicit PilgrimRobot(const emscripten::val &jsAbstractRobot) : CommonRobot(jsAbstractRobot) {
  }

  emscripten::val onTurn() override {
    return nullAction();
  }
};

class AttackerRobot : public CommonRobot {
 public:
  explicit AttackerRobot(const emscripten::val &jsAbstractRobot) : CommonRobot(jsAbstractRobot) {
  }

  emscripten::val onTurn() override {
    return nullAction();
  }
};

unique_ptr<AbstractNativeRobot> AbstractNativeRobot::createNativeRobotImpl(emscripten::val jsAbstractRobot) {
  // Can't use convenience me() method here, because the wrapper class has not been initialized
  Robot me = Robot::fromSelfRobot(jsAbstractRobot);
  switch (me.unit()) {
  case Unit::CASTLE:
    return make_unique<CastleRobot>(jsAbstractRobot);
  case Unit::CHURCH:
    return make_unique<NoOpRobot>(jsAbstractRobot);
  case Unit::PILGRIM:
    return make_unique<PilgrimRobot>(jsAbstractRobot);
  case Unit::CRUSADER:
    return make_unique<AttackerRobot>(jsAbstractRobot);
  case Unit::PROPHET:
    return make_unique<AttackerRobot>(jsAbstractRobot);
  case Unit::PREACHER:
    return make_unique<AttackerRobot>(jsAbstractRobot);
  case Unit::UNDEFINED:
    break;
  }
  return make_unique<NoOpRobot>(jsAbstractRobot);
}

}