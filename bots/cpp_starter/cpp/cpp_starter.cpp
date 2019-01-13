
#include <emscripten/bind.h>

#include "cpp_starter.h"

using emscripten::class_;
using emscripten::val;

// Binding code
EMSCRIPTEN_BINDINGS(my_robot_example) { // NOLINT
  class_<NativeRobot>("NativeRobot")
      .constructor<val>()
      .function("turn", &NativeRobot::turn);
}