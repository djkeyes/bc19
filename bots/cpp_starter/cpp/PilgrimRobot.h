

#ifndef CPP_STARTER_PILGRIMROBOT_H
#define CPP_STARTER_PILGRIMROBOT_H

#include "CommonRobot.h"

namespace bc19 {
class PilgrimRobot : public CommonRobot {
 private:

 public:
  explicit PilgrimRobot(const emscripten::val &jsAbstractRobot) : CommonRobot(jsAbstractRobot) {
  }

  emscripten::val onTurn() override;
};
}

#endif //CPP_STARTER_PILGRIMROBOT_H
