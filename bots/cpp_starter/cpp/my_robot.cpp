#include "cpp_starter.h"
#include "fast_rand.h"

using std::to_string;
using std::unique_ptr;
using std::make_unique;

namespace bc19 {

class NoOpRobot : public AbstractNativeRobot {
 public:
  explicit NoOpRobot(const emscripten::val &jsAbstractRobot) : AbstractNativeRobot(jsAbstractRobot) {}

  emscripten::val turn() override {
    return nullAction();
  }
};

class CastleRobot : public AbstractNativeRobot {
 public:
  explicit CastleRobot(const emscripten::val &jsAbstractRobot) : AbstractNativeRobot(jsAbstractRobot) {}

  emscripten::val turn() override {
    return nullAction();
  }
};

unique_ptr<AbstractNativeRobot> AbstractNativeRobot::createNativeRobotImpl(emscripten::val jsAbstractRobot) {
  // Can't use convenience me() method here, because the wrapper class has not been initialized
  Robot me = Robot::fromSelfRobot(jsAbstractRobot);
  switch (me.unit()) {
    case specs::Unit::CASTLE:return make_unique<CastleRobot>(jsAbstractRobot);
    case specs::Unit::CHURCH:return make_unique<NoOpRobot>(jsAbstractRobot);
    case specs::Unit::PILGRIM:return make_unique<NoOpRobot>(jsAbstractRobot);
    case specs::Unit::CRUSADER: return make_unique<NoOpRobot>(jsAbstractRobot);
    case specs::Unit::PROPHET:return make_unique<NoOpRobot>(jsAbstractRobot);
    case specs::Unit::PREACHER:return make_unique<NoOpRobot>(jsAbstractRobot);
    case specs::Unit::UNDEFINED:break;
  }
  return make_unique<NoOpRobot>(jsAbstractRobot);
}

}