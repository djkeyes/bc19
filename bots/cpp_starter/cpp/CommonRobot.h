

#ifndef CPP_STARTER_COMMONROBOT_H
#define CPP_STARTER_COMMONROBOT_H

#include "CastleTalker.h"

namespace bc19 {

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

}

#endif //CPP_STARTER_COMMONROBOT_H
