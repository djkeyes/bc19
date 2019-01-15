

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

constexpr std::array<Coordinate, 4> horiz_adjacent = {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}};
constexpr std::array<Coordinate, 8>
    adjacent_spiral = {{{-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}}};

static constexpr int countDiskOffsets(const int radius_sq) {
  int count = 0;
  for (int row = 0; row * row <= radius_sq; ++row) {
    for (int col = 1; col * col + row * row <= radius_sq; ++col) {
      count += 4;
    }
  }
  return count;
}

template<int radius_sq>
static constexpr std::array<Coordinate, countDiskOffsets(radius_sq)> computeDiskOffsets() {
  std::array<Coordinate, countDiskOffsets(radius_sq)> result;

  int idx = 0;
  for (int row = 0; row * row <= radius_sq; ++row) {
    for (int col = 1; col * col + row * row <= radius_sq; ++col) {
      result[idx++] = Coordinate(static_cast<Coordinate::DimType>(row), static_cast<Coordinate::DimType>(col));
      result[idx++] = Coordinate(static_cast<Coordinate::DimType>(col), static_cast<Coordinate::DimType>(-row));
      result[idx++] = Coordinate(static_cast<Coordinate::DimType>(-row), static_cast<Coordinate::DimType>(-col));
      result[idx++] = Coordinate(static_cast<Coordinate::DimType>(-col), static_cast<Coordinate::DimType>(row));
    }
  }
  return result;
}

static constexpr int countHollowDiskOffsets(const int min_radius_sq, const int max_radius_sq) {
  int count = 0;
  for (int row = 0; row * row <= max_radius_sq; ++row) {
    for (int col = 1; col * col + row * row <= max_radius_sq; ++col) {
      if (row * row + col * col >= min_radius_sq) {
        count += 4;
      }
    }
  }
  return count;
}

template<int min_radius_sq, int max_radius_sq>
static constexpr std::array<Coordinate, countHollowDiskOffsets(min_radius_sq, max_radius_sq)> computeDiskOffsets() {
  std::array<Coordinate, countHollowDiskOffsets(min_radius_sq, max_radius_sq)> result;

  int idx = 0;
  for (int row = 0; row * row <= max_radius_sq; ++row) {
    for (int col = 1; col * col + row * row <= max_radius_sq; ++col) {
      if (row * row + col * col >= min_radius_sq) {
        result[idx++] = Coordinate(static_cast<Coordinate::DimType>(row), static_cast<Coordinate::DimType>(col));
        result[idx++] = Coordinate(static_cast<Coordinate::DimType>(col), static_cast<Coordinate::DimType>(-row));
        result[idx++] = Coordinate(static_cast<Coordinate::DimType>(-row), static_cast<Coordinate::DimType>(-col));
        result[idx++] = Coordinate(static_cast<Coordinate::DimType>(-col), static_cast<Coordinate::DimType>(row));
      }
    }
  }
  return result;
}

static constexpr auto pilgrim_moves = computeDiskOffsets<specs::units[static_cast<int>(specs::Unit::PILGRIM)].speed>();
static constexpr auto
    crusader_moves = computeDiskOffsets<specs::units[static_cast<int>(specs::Unit::CRUSADER)].speed>();
static constexpr auto prophet_moves = computeDiskOffsets<specs::units[static_cast<int>(specs::Unit::PROPHET)].speed>();
static constexpr auto
    preacher_moves = computeDiskOffsets<specs::units[static_cast<int>(specs::Unit::PREACHER)].speed>();

static constexpr auto crusader_attacks =
    computeDiskOffsets<specs::units[static_cast<int>(specs::Unit::CRUSADER)].attack_radius[0],
                       specs::units[static_cast<int>(specs::Unit::CRUSADER)].attack_radius[1]>();
static constexpr auto prophet_attacks =
    computeDiskOffsets<specs::units[static_cast<int>(specs::Unit::PROPHET)].attack_radius[0],
                       specs::units[static_cast<int>(specs::Unit::PROPHET)].attack_radius[1]>();
static constexpr auto preacher_attacks =
    computeDiskOffsets<specs::units[static_cast<int>(specs::Unit::PREACHER)].attack_radius[0],
                       specs::units[static_cast<int>(specs::Unit::PREACHER)].attack_radius[1]>();
// same as preacher_attacks, but won't damage self
static constexpr auto preacher_effective_targets =
    computeDiskOffsets<4, specs::units[static_cast<int>(specs::Unit::PREACHER)].attack_radius[1]>();
static constexpr auto preacher_effective_max_radius = 26;
// same as preacher_attacks, but also includes extended AoE area
static constexpr auto preacher_effective_area =
    computeDiskOffsets<specs::units[static_cast<int>(specs::Unit::PREACHER)].attack_radius[0],
                       preacher_effective_max_radius>();

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
//    static std::optional<int> prev_time;
    //    const auto &m = me();
    //    int cur_time = m.time();
    //    std::string label;
    //    switch (m.unit()) {
    //    case specs::Unit::CASTLE:
    //      label = "CA";
    //      break;
    //    case specs::Unit::CHURCH:
    //      label = "CH";
    //      break;
    //    case specs::Unit::PILGRIM:
    //      label = "PL";
    //      break;
    //    case specs::Unit::CRUSADER:
    //      label = "CR";
    //      break;
    //    case specs::Unit::PROPHET:
    //      label = "PH";
    //      break;
    //    case specs::Unit::PREACHER:
    //      label = "PC";
    //      break;
    //    case specs::Unit::UNDEFINED:
    //      label = "??";
    //      break;
    //    }
    //    std::string message = "[" + label + "] chess time: " + std::to_string(cur_time);
    //    if (prev_time) {
    //      message += " (prev delta=" + std::to_string(*prev_time - cur_time + specs::chess_extra) + ")";
    //    }
    //    prev_time = cur_time;
    //    log(message);
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
