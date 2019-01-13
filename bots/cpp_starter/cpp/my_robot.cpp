#include "cpp_starter.h"
#include "fast_rand.h"

using std::to_string;

emscripten::val NativeRobot::turn() {
  log("hello world!");
  auto unit = me().unit();
  if (unit == specs::Unit::CRUSADER) {
    log("Crusader health: " + std::to_string(me().health()));
    static const std::array<std::pair<int, int>, 8> choices =
        {{{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}}};
    const auto &choice = choices[fast_rand::small_uniform_int_rand(choices.size())];
    return move(choice.first, choice.second);
  } else if (unit == specs::Unit::CASTLE) {
    log("Castle health: " + std::to_string(me().health()));
    if (me().turn() % 10 == 0) {
      log("Building a crusader at " + to_string(me().x() + 1) + ", " + to_string(me().y() + 1));
      return buildUnit(specs::Unit::CRUSADER, 1, 1);
    } else {
      return nullAction();
    }
  }

  return nullAction();
}
