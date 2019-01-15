#include <vector>

#include "cpp_starter.h"
#include "fast_rand.h"
#include "UnitCounts.h"

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
 * Encapsulates CastleTalk message encoding and decoding
 */
class CastleTalker {
 public:

  // 2^8 = 512 isn't enough to encode important things (e.g. uncompressed unit ids, xy-coordinates, etc.), especially
  // if you want to send multiple messages at once.
  // So just use castletalk to encode a list of constants.


  // by default, messages have the value 0. Require all messages to be offset by 1.
  static constexpr uint8_t valid_message_offset = 1;

  enum class BUILT_UNIT : uint8_t {
    BUILT_NOTHING = 0, BUILT_PILGRIM, BUILT_CRUSADER, BUILT_PREACHER, BUILT_PROPHET, END_BUILT_UNIT
    // we could also add one for church, but that's more situational
  };

  static const BUILT_UNIT builtMessageFromUnit(const Unit &unit) {
    switch (unit) {
    case Unit::PILGRIM:
      return BUILT_UNIT::BUILT_PILGRIM;
    case Unit::CRUSADER:
      return BUILT_UNIT::BUILT_CRUSADER;
    case Unit::PROPHET:
      return BUILT_UNIT::BUILT_PROPHET;
    case Unit::PREACHER:
      return BUILT_UNIT::BUILT_PREACHER;
    default:
      return BUILT_UNIT::BUILT_NOTHING;
    }
  }

  enum class ROLL_CALL : uint8_t {
    I_AM_CASTLE0 = 0, I_AM_CASTLE1, I_AM_CASTLE2, I_AM_A_PILGRIM, I_AM_A_CRUSADER, I_AM_A_PROPHET, I_AM_A_PREACHER,
    END_ROLL_CALL
  };

  static const ROLL_CALL rollCallMessageFromUnit(const Unit &unit) {
    switch (unit) {
    case Unit::CASTLE:
      // assume we're castle index 0. this will be updated later.
      return ROLL_CALL::I_AM_CASTLE0;
    case Unit::PILGRIM:
      return ROLL_CALL::I_AM_A_PILGRIM;
    case Unit::CRUSADER:
      return ROLL_CALL::I_AM_A_CRUSADER;
    case Unit::PROPHET:
      return ROLL_CALL::I_AM_A_PROPHET;
    case Unit::PREACHER:
      return ROLL_CALL::I_AM_A_PREACHER;
    default:
      return ROLL_CALL::I_AM_A_PREACHER;
    }
  }

 private:
  // Users must ensure that this pointer has a longer lifetime than this class
  AbstractNativeRobot *const self_;

  int last_turn_message_sent_;
  uint8_t last_message_sent_;
  BUILT_UNIT staged_build_message_;
  ROLL_CALL staged_roll_call_message_;

  static constexpr uint8_t multAdd(const uint8_t addend, const uint8_t multicand1, const uint8_t multicand2) {
    return addend + multicand1 * multicand2;
  }

  static constexpr uint8_t divMod(uint8_t &dividend, const uint8_t divisor) {
    // TODO(daniel): does it help to use std::div? half-decent compilers should know to optimize this.
    const uint8_t modulus = dividend % divisor;
    dividend = dividend / divisor;
    return modulus;
  }

  static constexpr uint8_t encodeMessage(const BUILT_UNIT built_unit_part, const ROLL_CALL roll_call_part) {
    // sanity check that the encoding isn't too large
    // not that division is necessary here; multiplication could overflow.
    static_assert(
        (0xFF - valid_message_offset) / static_cast<uint8_t>(BUILT_UNIT::END_BUILT_UNIT ) / static_cast<uint8_t>
        (ROLL_CALL::END_ROLL_CALL) > 0);

    return valid_message_offset + multAdd(static_cast<const uint8_t>(built_unit_part),
                                          static_cast<const uint8_t>(BUILT_UNIT::END_BUILT_UNIT),
                                          multAdd(static_cast<const uint8_t>(roll_call_part), static_cast<const uint8_t>
                                          (ROLL_CALL::END_ROLL_CALL),
                                              /* more terms go here */ 0));
  }

  void determineCastleIndex() {
    // TODO: this fails abysmally if any of the castles time out on the first turn
    const auto &visible = self_->getVisibleRobots();
    const auto length = visible["length"].as<int>();
    int castle_index = 0;
    for (int i = 0; i < length; ++i) {
      Robot robot(visible[i]);
      // check for team -- enemies show up if visible
      if (self_->isVisible(robot) && robot.team() != self_->me().team()) {
        continue;
      }

      const auto message = robot.castle_talk();

      CastleTalker::BUILT_UNIT built_unit_part;
      CastleTalker::ROLL_CALL roll_call_part;
      if (!decodeMessage(message, built_unit_part, roll_call_part)) {
        continue;
      }
      // check roll call
      switch (roll_call_part) {
      case ROLL_CALL::I_AM_CASTLE0:
      case ROLL_CALL::I_AM_CASTLE1:
      case ROLL_CALL::I_AM_CASTLE2:
        ++castle_index;

      default:
        break;
      }
    }

    staged_roll_call_message_ = static_cast<ROLL_CALL>(castle_index);
  }

 public:

  explicit CastleTalker(AbstractNativeRobot *const this_robot)
      : self_(this_robot),
      last_turn_message_sent_(-1),
      last_message_sent_(0),
      staged_build_message_(BUILT_UNIT::BUILT_NOTHING),
      staged_roll_call_message_(rollCallMessageFromUnit(this_robot->me().unit())) {
  }

  static constexpr bool decodeMessage(uint8_t message, BUILT_UNIT &built_unit_part, ROLL_CALL &roll_call_part) {
    if (message < valid_message_offset) {
      return false;
    }
    message -= valid_message_offset;
    built_unit_part = static_cast<BUILT_UNIT>(divMod(message, static_cast<const uint8_t>(BUILT_UNIT::END_BUILT_UNIT)));
    roll_call_part = static_cast<ROLL_CALL>(divMod(message, static_cast<const uint8_t>(ROLL_CALL::END_ROLL_CALL)));
    /* more terms go here */

    return true;
  }

  void stageBuiltUnit(const BUILT_UNIT &built_unit_part) {
    staged_build_message_ = built_unit_part;
  }

  void sendStagedMessage() {
    // TODO(daniel): assert that multiple messages aren't being enqueued each turn
    last_message_sent_ = encodeMessage(staged_build_message_, staged_roll_call_message_);
    self_->castleTalk(last_message_sent_);
    last_turn_message_sent_ = self_->me().turnCount();

    staged_build_message_ = BUILT_UNIT::BUILT_NOTHING;
  }

  void processCastleTalks(UnitCounts &unit_count) {
    // TODO: assert the me() is a castle. can we use the friend keyword?
    if (self_->me().turnCount() == 1) {
      determineCastleIndex();
    }

    // Currently, this counts the number of units which we know have been built
    // TODO: periodically request a roll call from all units
    const auto &visible = self_->getVisibleRobots();
    const auto length = visible["length"].as<int>();
    // one message is from us, in the past. skip it.
    bool skipped_own_message = false;
    // reset
    unit_count = UnitCounts();
    for (int i = 0; i < length; ++i) {
      Robot robot(visible[i]);
      // check for team -- enemies show up if visible
      if (self_->isVisible(robot) && robot.team() != self_->me().team()) {
        continue;
      }
      const auto message = robot.castle_talk();

      // TODO: can we check robot.id here? this only checks the message contents, which is okay for now.
      if (!skipped_own_message && message == last_message_sent_) {
        ++unit_count.num_castles_;
        skipped_own_message = true;
        continue;
      }

      CastleTalker::BUILT_UNIT built_unit_part;
      CastleTalker::ROLL_CALL roll_call_part;
      if (!decodeMessage(message, built_unit_part, roll_call_part)) {
        ++unit_count.num_unknown_;
        continue;
      }
      // check roll call
      switch (roll_call_part) {
      case ROLL_CALL::I_AM_CASTLE0:
      case ROLL_CALL::I_AM_CASTLE1:
      case ROLL_CALL::I_AM_CASTLE2:
        ++unit_count.num_castles_;
        break;
      case ROLL_CALL::I_AM_A_PILGRIM:
        ++unit_count.num_pilgrims_;
        break;
      case ROLL_CALL::I_AM_A_CRUSADER:
        ++unit_count.num_crusaders_;
        break;
      case ROLL_CALL::I_AM_A_PROPHET:
        ++unit_count.num_prophets_;
        break;
      case ROLL_CALL::I_AM_A_PREACHER:
        ++unit_count.num_preachers_;
        break;
      default:
        break;
      }

      // only check built messages if we're a later castle
      if ((staged_roll_call_message_ == ROLL_CALL::I_AM_CASTLE2
          && (roll_call_part == ROLL_CALL::I_AM_CASTLE0 || roll_call_part == ROLL_CALL::I_AM_CASTLE1))
          || (staged_roll_call_message_ == ROLL_CALL::I_AM_CASTLE1 && roll_call_part == ROLL_CALL::I_AM_CASTLE0)) {
        // these units were just built, but couldn't answer roll call yet
        if (built_unit_part != BUILT_UNIT::BUILT_NOTHING) {
          --unit_count.num_unknown_;
        }
        switch (built_unit_part) {
        case BUILT_UNIT::BUILT_PILGRIM:
          ++unit_count.num_pilgrims_;
          break;
        case BUILT_UNIT::BUILT_CRUSADER:
          ++unit_count.num_crusaders_;
          break;
        case BUILT_UNIT::BUILT_PREACHER:
          ++unit_count.num_preachers_;
          break;
        case BUILT_UNIT::BUILT_PROPHET:
          ++unit_count.num_prophets_;
          break;
        default:
          break;
        }
      }

    }
  }

};

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