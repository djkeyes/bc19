

#ifndef CPP_STARTER_CASTLETALKER_H
#define CPP_STARTER_CASTLETALKER_H

#include <cstdint>

#include "cpp_starter.h"
#include "UnitCounts.h"

namespace bc19 {
/**
 * Encapsulates CastleTalk message encoding and decoding
 */
class CastleTalker {
 public:

  // 2^8 = 512 isn't enough to encode important things (e.g. uncompressed specs::Unit ids, xy-coordinates, etc.), especially
  // if you want to send multiple messages at once.
  // So just use castletalk to encode a list of constants.


  // by default, messages have the value 0. Require all messages to be offset by 1.
  static constexpr uint8_t valid_message_offset = 1;

  enum class BUILT_UNIT : uint8_t {
    BUILT_NOTHING = 0, BUILT_PILGRIM, BUILT_CRUSADER, BUILT_PREACHER, BUILT_PROPHET, END_BUILT_UNIT
    // we could also add one for church, but that's more situational
  };

  static const BUILT_UNIT builtMessageFromUnit(const specs::Unit &unit) {
    switch (unit) {
    case specs::Unit::PILGRIM:
      return BUILT_UNIT::BUILT_PILGRIM;
    case specs::Unit::CRUSADER:
      return BUILT_UNIT::BUILT_CRUSADER;
    case specs::Unit::PROPHET:
      return BUILT_UNIT::BUILT_PROPHET;
    case specs::Unit::PREACHER:
      return BUILT_UNIT::BUILT_PREACHER;
    default:
      return BUILT_UNIT::BUILT_NOTHING;
    }
  }

  enum class ROLL_CALL : uint8_t {
    I_AM_CASTLE0 = 0, I_AM_CASTLE1, I_AM_CASTLE2, I_AM_A_PILGRIM, I_AM_A_CRUSADER, I_AM_A_PROPHET, I_AM_A_PREACHER,
    END_ROLL_CALL
  };

  static const ROLL_CALL rollCallMessageFromUnit(const specs::Unit &unit) {
    switch (unit) {
    case specs::Unit::CASTLE:
      // assume we're castle index 0. this will be updated later.
      return ROLL_CALL::I_AM_CASTLE0;
    case specs::Unit::PILGRIM:
      return ROLL_CALL::I_AM_A_PILGRIM;
    case specs::Unit::CRUSADER:
      return ROLL_CALL::I_AM_A_CRUSADER;
    case specs::Unit::PROPHET:
      return ROLL_CALL::I_AM_A_PROPHET;
    case specs::Unit::PREACHER:
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
      Robot robot(visible[i], self_);
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

    // Currently, this counts the number of specs::Units which we know have been built
    // TODO: periodically request a roll call from all specs::Units
    const auto &visible = self_->getVisibleRobots();
    const auto length = visible["length"].as<int>();
    // one message is from us, in the past. skip it.
    bool skipped_own_message = false;
    // reset
    unit_count = UnitCounts();
    for (int i = 0; i < length; ++i) {
      Robot robot(visible[i], self_);
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
        // these specs::Units were just built, but couldn't answer roll call yet
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
}

#endif //CPP_STARTER_CASTLETALKER_H
