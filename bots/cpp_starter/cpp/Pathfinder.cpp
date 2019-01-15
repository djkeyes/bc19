

#include <algorithm>
#include "Pathfinder.h"

namespace bc19 {

emscripten::val Pathfinder::badPathFinding(const Coordinate &from,
                                           const Coordinate &to,
                                           Coordinate::DimSqType max_radius_sq) const {
  if (from == to) {
    return self_->nullAction();
  }

  // check if our radius is fuel-limited
  max_radius_sq = std::min(max_radius_sq,
                           static_cast<Coordinate::DimSqType>(self_->fuel()
                               / specs::units[static_cast<int>(self_->me().unit())].fuel_per_move));

  const auto &occupied_map = self_->getVisibleRobotMap();
  // TODO: search
  // bad pathfinding: just move in the direction, as long as we're getting closer
  auto min_dist = from.distSq(to);
  Coordinate best_delta(0, 0);
  for (Coordinate::DimSqType row = 0; row * row <= max_radius_sq; ++row) {
    for (Coordinate::DimSqType col = 0; col * col + row * row <= max_radius_sq; ++col) {
      for (Coordinate::DimType rsign = -1; rsign <= 1; rsign += 2) {
        for (Coordinate::DimType csign = -1; csign <= 1; csign += 2) {
          Coordinate delta =
              Coordinate(static_cast<Coordinate::DimType>(row) * rsign, static_cast<Coordinate::DimType>(col) * csign);
          Coordinate next = from + delta;
          if (next.row_ < 0 || next.col_ < 0 || next.row_ >= passable_map_.rows_ || next.col_ >= passable_map_.cols_) {
            continue;
          }
          if (!passable_map_.get(next)) {
            continue;
          }
          const auto next_dist = to.distSq(next);
          if (next_dist < min_dist) {
            if (occupied_map.get(next.row_, next.col_) == 0) {
              min_dist = next_dist;
              best_delta = delta;
            }
          }
        }
      }
    }
  }
  if (from + best_delta == from) {
    return self_->nullAction();
  }

  return self_->move(best_delta.col_, best_delta.row_);
}

emscripten::val Pathfinder::pathTowardCheaply(const Coordinate &coordinate) {
  Coordinate
      my_loc(static_cast<Coordinate::DimType>(self_->me().y()), static_cast<Coordinate::DimType>(self_->me().x()));
  return badPathFinding(my_loc, coordinate, 2);
}

emscripten::val Pathfinder::pathTowardQuickly(const Coordinate &coordinate) {
  const auto &m = self_->me();
  Coordinate my_loc(static_cast<Coordinate::DimType>(m.y()), static_cast<Coordinate::DimType>(m.x()));
  return badPathFinding(my_loc,
                        coordinate,
                        static_cast<const Coordinate::DimSqType>(specs::units[static_cast<int>(m.unit())].speed));
}

Coordinate Pathfinder::getNearbyPassableTile(const Coordinate &coordinate) const {
  // sometimes people ask for a ballpark destination.
  // in that case, we need to make sure they actually asked for a real location
  if (passable_map_.get(coordinate)) {
    return coordinate;
  }
  // just keep spiraling out in a manhattan circle. we'll finish eventually.
  for (Coordinate::DimType r = 1;; ++r) {
    for (Coordinate::DimType i = 0; i < r; ++i) {
      Coordinate d1 = coordinate + Coordinate(i, r - i);
      if (passable_map_.get(d1)) {
        return d1;
      }
      Coordinate d2 = coordinate + Coordinate(r - i, -i);
      if (passable_map_.get(d2)) {
        return d2;
      }
      Coordinate d3 = coordinate + Coordinate(-i, -r + i);
      if (passable_map_.get(d3)) {
        return d3;
      }
      Coordinate d4 = coordinate + Coordinate(-r + i, i);
      if (passable_map_.get(d4)) {
        return d4;
      }
    }
  }
}
}

