
#include <emscripten/bind.h>

using emscripten::val;
using emscripten::class_;

class NativeRobot {
 private:
  // TODO: I think this is reference counted. Does it ever get deallocated?
  val jsRobot_;

 public:
  explicit NativeRobot(val jsRobot) : jsRobot_(jsRobot) {}

  void turn() {

  }
};

// Binding code
EMSCRIPTEN_BINDINGS(my_robot_example) { // NOLINT
  class_<NativeRobot>("NativeRobot")
      .constructor<val>()
      .function("turn", &NativeRobot::turn);
}