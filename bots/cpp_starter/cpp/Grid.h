

#ifndef CPP_STARTER_GRID_H
#define CPP_STARTER_GRID_H

#include <vector>
#include "Coordinate.h"

namespace bc19 {
template<typename T>
struct Grid {
  Grid(int cols, int rows)
      : cols_(cols), rows_(rows), data(static_cast<typename std::vector<T>::size_type>(cols * rows)) {
  }

  void set(int row, int col, T val) {
    data[row * cols_ + col] = val;
  }

  const T &get(int row, int col) const {
    return data[row * cols_ + col];
  }

  const T &get(const Coordinate &coord) const {
    return get(coord.row_, coord.col_);
  }

  int cols_;
  int rows_;
  std::vector<T> data;
};

using GridChar = Grid<uint8_t>;
}

#endif //CPP_STARTER_GRID_H
