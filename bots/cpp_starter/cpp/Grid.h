

#ifndef CPP_STARTER_GRID_H
#define CPP_STARTER_GRID_H

#include <vector>
#include "Coordinate.h"

namespace bc19 {
template<typename T>
struct Grid {
  Grid() : Grid(0, 0) {
  }

  Grid(int rows, int cols)
      : rows_(rows), cols_(cols), data(static_cast<typename std::vector<T>::size_type>(cols * rows)) {
  }

  void resize(int rows, int cols) {
    rows_ = rows;
    cols_ = cols;
    data.resize(static_cast<typename std::vector<T>::size_type>(cols * rows));
  }

  void set(int row, int col, T val) {
    data[row * cols_ + col] = val;
  }

  void set(const Coordinate &coord, T val) {
    set(coord.row_, coord.col_, val);
  }

  const T &get(int row, int col) const {
    return data[row * cols_ + col];
  }

  const T &get(const Coordinate &coord) const {
    return get(coord.row_, coord.col_);
  }

  T &get(int row, int col) {
    return data[row * cols_ + col];
  }

  T &get(const Coordinate &coord) {
    return get(coord.row_, coord.col_);
  }

  int rows_;
  int cols_;
  std::vector<T> data;
};

using GridChar = Grid<uint8_t>;
using GridShort = Grid<uint16_t>;
}

#endif //CPP_STARTER_GRID_H
