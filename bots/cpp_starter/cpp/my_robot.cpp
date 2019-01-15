#include <vector>

#include "cpp_starter.h"
#include "fast_rand.h"
#include "UnitCounts.h"
#include "CastleTalker.h"
#include "CommonRobot.h"
#include "CastleRobot.h"

using std::unique_ptr;
using std::make_unique;

namespace bc19 {

using specs::Unit;

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


class NoOpRobot : public CommonRobot {
 public:
  explicit NoOpRobot(const emscripten::val &jsAbstractRobot) : CommonRobot(jsAbstractRobot) {
  }

  emscripten::val onTurn() override {
    return nullAction();
  }
};

class PilgrimRobot : public CommonRobot {
 private:

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