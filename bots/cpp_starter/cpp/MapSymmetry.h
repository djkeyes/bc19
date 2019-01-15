

#ifndef CPP_STARTER_MAPSYMMETRY_H
#define CPP_STARTER_MAPSYMMETRY_H

#include "Pathfinder.h"

namespace bc19 {
class MapSymmetry {
 private:
  bool is_horizontal_;
  Coordinate::DimType rows_;
  Coordinate::DimType cols_;

  Coordinate flipHorizontally(const Coordinate &coord) const {
    return Coordinate(coord.row_, static_cast<Coordinate::DimType>(cols_ - coord.col_ - 1));
  }

  Coordinate flipVertically(const Coordinate &coord) const {
    return Coordinate(static_cast<Coordinate::DimType>(rows_ - coord.row_ - 1), coord.col_);
  }

 public:
  explicit MapSymmetry(const Pathfinder &pathfinder) {
    const auto &map = pathfinder.passableMap();
    rows_ = static_cast<Coordinate::DimType>(map.rows_);
    cols_ = static_cast<Coordinate::DimType>(map.cols_);
    bool deteminedSymmetry = false;
    for (int row = 0; row < map.rows_; ++row) {
      for (int col = 0; col < map.cols_; ++col) {
        Coordinate coord(row, col);
        Coordinate hori = flipHorizontally(coord);
        Coordinate vert = flipVertically(coord);
        const auto &passable = map.get(coord);
        const auto &hpassable = map.get(hori);
        const auto vpassable = map.get(vert);
        if ((passable == hpassable) && (passable != vpassable)) {
          is_horizontal_ = true;
          deteminedSymmetry = true;
          break;
        } else if ((passable == vpassable) && (passable != hpassable)) {
          is_horizontal_ = false;
          deteminedSymmetry = true;
          break;
        }
      }
      if (deteminedSymmetry) {
        break;
      }
    }
  }

  bool hasHorizontalSymmetry() const {
    return is_horizontal_;
  }

  Coordinate flipCoord(const Coordinate &coord) const {
    if (hasHorizontalSymmetry()) {
      return flipHorizontally(coord);
    } else {
      return flipVertically(coord);
    }
  }
};
}
#endif //CPP_STARTER_MAPSYMMETRY_H
