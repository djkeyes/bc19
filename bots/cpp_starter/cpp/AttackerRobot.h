

#ifndef CPP_STARTER_ATTACKERROBOT_H
#define CPP_STARTER_ATTACKERROBOT_H

#include "CommonRobot.h"
#include "Pathfinder.h"

namespace bc19 {

class AttackerRobot : public CommonRobot {
 private:
  Pathfinder pathfinder_;

 public:
  explicit AttackerRobot(const emscripten::val &jsAbstractRobot) : CommonRobot(jsAbstractRobot), pathfinder_(this) {
  }

  emscripten::val onTurn() override {
    // TODO: use map symmetry to determine where (at least one) castle must be


    return pathfinder_.pathToRandomTile();
  }
};
}
#endif //CPP_STARTER_ATTACKERROBOT_H
