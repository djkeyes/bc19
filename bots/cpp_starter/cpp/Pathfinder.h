

#ifndef CPP_STARTER_PATHFINDER_H
#define CPP_STARTER_PATHFINDER_H

#include "cpp_starter.h"
#include "Coordinate.h"
#include "DeterministicResourceClusterer.h"
#include "Grid.h"

namespace bc19 {
class Pathfinder {
 private:
  // Users must ensure that this pointer has a longer lifetime than this class
  AbstractNativeRobot *const self_;

  // FIXME until the starter API map is updated, keep a copy of the passable map
  static GridChar copyPassableMap(const AbstractNativeRobot::MapBool &map) {
    GridChar result(map.rows(), map.cols());
    for (int row = 0; row < result.rows_; ++row) {
      for (int cols = 0; cols < result.cols_; ++cols) {
        result.set(row, cols, static_cast<uint8_t>(map.get(row, cols)));
      }
    }
    return result;
  }

  // TODO: there's significant doubt in the world about whether vector<bool> is faster than vector<char>. We should
  // profile this.
  GridChar passable_map_;

  // Only does a single step of best-first-search. Avoid using this if feasible.
  emscripten::val singlePassPathFinding(const Coordinate &from,
                                        const Coordinate &to,
                                        Coordinate::DimSqType max_radius_sq) const;

 public:
  explicit Pathfinder(AbstractNativeRobot *const self)
      : self_(self), passable_map_(copyPassableMap(self->getPassableMap())) {
  }

  emscripten::val pathTowardCheaply(const Coordinate &coordinate);
  emscripten::val pathTowardQuickly(const Coordinate &coordinate);

  Coordinate getNearbyPassableTile(const Coordinate &coordinate) const;
};

}

#endif //CPP_STARTER_PATHFINDER_H
