

#ifndef CPP_STARTER_COMMONROBOT_H
#define CPP_STARTER_COMMONROBOT_H

#include <utility>

#include "cpp_starter.h"
#include "CastleTalker.h"
#include "Coordinate.h"

namespace bc19 {

namespace directions {
// TODO: make some constexpr functions that can compute lists of tile displacements at compiletime

constexpr std::array<std::pair<int, int>, 4> horiz_adjacent = {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}};
constexpr std::array<std::pair<int, int>, 8>
    adjacent_spiral = {{{-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}}};
}

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

/**
 * Convert a karbonite or fuel map to a vector of sparse locations.
 * @return
 */
inline std::vector<Coordinate> mineMapToVec(const AbstractNativeRobot::MapBool &mine_map) {
  std::vector<Coordinate> result;
  for (Coordinate::DimType row = 0; row < mine_map.rows(); ++row) {
    for (Coordinate::DimType col = 0; col < mine_map.cols(); ++col) {
      if (mine_map.get(row, col)) {
        result.emplace_back(row, col);
      }
    }
  }
  return result;
}
}

#endif //CPP_STARTER_COMMONROBOT_H
