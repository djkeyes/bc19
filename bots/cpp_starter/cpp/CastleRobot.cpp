#include <utility>

#include "CastleRobot.h"
#include "CastleTalker.h"

using bc19::specs::Unit;
using std::to_string;
using std::pair;

namespace bc19 {

const std::vector<specs::Unit>
    BuildOrder::PlCrCr_build_order = {Unit::PILGRIM, Unit::CRUSADER, Unit::CRUSADER};

const std::vector<specs::Unit>
    BuildOrder::PhPhPhPc_build_order = {Unit::PROPHET, Unit::PROPHET, Unit::PROPHET, Unit::PILGRIM};
const std::vector<specs::Unit>
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
const std::vector<specs::Unit>BuildOrder::CrCrPlPlPlPlPlPl_build_order = {
    Unit::CRUSADER,
    Unit::CRUSADER,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM,
    Unit::PILGRIM
};
const std::vector<specs::Unit>BuildOrder::CrCrCrPlPlPlPl_build_order =
    {Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM};
const std::vector<specs::Unit>BuildOrder::CrCrCrCrPlPl_build_order =
    {Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER, Unit::PILGRIM, Unit::PILGRIM};
const std::vector<specs::Unit>BuildOrder::PlPlCrCrCrCr_build_order =
    {Unit::PILGRIM, Unit::PILGRIM, Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER, Unit::CRUSADER};
const std::vector<specs::Unit>BuildOrder::PcPcPlPlPlPlPl_build_order =
    {Unit::PREACHER, Unit::PREACHER, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM, Unit::PILGRIM};
const std::vector<specs::Unit>
    BuildOrder::PcPcPcPlPl_build_order = {Unit::PREACHER, Unit::PREACHER, Unit::PREACHER, Unit::PILGRIM, Unit::PILGRIM};
const std::vector<specs::Unit>
    BuildOrder::PcPcPhPlPl_build_order = {Unit::PREACHER, Unit::PREACHER, Unit::PROPHET, Unit::PILGRIM, Unit::PILGRIM};

emscripten::val CastleRobot::onTurn() {
  castle_talker_.processCastleTalks(unit_counts_);

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

emscripten::val CastleRobot::tryBuilding(const Unit &unit_type) {
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
}