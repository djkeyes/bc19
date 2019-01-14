#include <emscripten/bind.h>

#include "cpp_starter.h"

using namespace bc19;
using emscripten::class_;
using emscripten::val;

// Binding code
EMSCRIPTEN_BINDINGS(my_robot_example) { // NOLINT
  class_<AbstractNativeRobot>("AbstractNativeRobot").class_function<std::unique_ptr<AbstractNativeRobot> >(
      "createNativeRobotImpl",
      &AbstractNativeRobot::createNativeRobotImpl).function("turn", &AbstractNativeRobot::turn);
}
