

#ifndef CPP_STARTER_COMMONROBOT_H
#define CPP_STARTER_COMMONROBOT_H

#include <utility>
#include <optional>

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
    static std::optional<int> prev_time;
    const auto &m = me();
    int cur_time = m.time();
    std::string label;
    switch (m.unit()) {
    case specs::Unit::CASTLE:
      label = "CA";
      break;
    case specs::Unit::CHURCH:
      label = "CH";
      break;
    case specs::Unit::PILGRIM:
      label = "PL";
      break;
    case specs::Unit::CRUSADER:
      label = "CR";
      break;
    case specs::Unit::PROPHET:
      label = "PH";
      break;
    case specs::Unit::PREACHER:
      label = "PC";
      break;
    case specs::Unit::UNDEFINED:
      label = "??";
      break;
    }
    std::string message = "[" + label + "] chess time: " + std::to_string(cur_time);
    if (prev_time) {
      message += " (prev delta=" + std::to_string(*prev_time - cur_time + specs::chess_extra) + ")";
    }
    prev_time = cur_time;
    log(message);
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
