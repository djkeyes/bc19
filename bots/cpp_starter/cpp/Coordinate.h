

#ifndef CPP_STARTER_COORDINATE_H
#define CPP_STARTER_COORDINATE_H

#include <cstdint>
#include <string>

namespace bc19 {
struct Coordinate {
  // maps can be at most 2^6 x 2^6, so 7-bits is sufficient
  // we allow negative in order to make subtraction easier
  using DimType = int8_t;
  // max distance is 2^6 * 2^6 + 2^6 * 2^6 = 2^13
  using DimSqType = int16_t;

  DimType row_; // aka "y"
  DimType col_; // aka "x"

  constexpr Coordinate() : Coordinate(0, 0) {
  }

  constexpr Coordinate(DimType row, DimType col) : row_(row), col_(col) {
  }

  constexpr Coordinate &operator+=(const Coordinate &that) {
    row_ += that.row_;
    col_ += that.col_;
    return *this;
  }

  constexpr Coordinate operator+(const Coordinate &that) const {
    Coordinate result = *this;
    return result += that;
  }

  constexpr Coordinate &operator-=(const Coordinate &that) {
    row_ -= that.row_;
    col_ -= that.col_;
    return *this;
  }

  constexpr Coordinate operator-(const Coordinate &that) const {
    Coordinate result = *this;
    return result -= that;
  }

  constexpr DimSqType distSq(const Coordinate &that) const {
    Coordinate diff = *this - that;
    DimSqType dc = diff.col_;
    DimSqType dr = diff.row_;
    return dc * dc + dr * dr;
  }

  constexpr bool operator==(const Coordinate &that) const {
    return (row_ == that.row_) && (col_ == that.col_);
  }

};
}

namespace std {

// TODO: only enable this in debug mode
inline std::string to_string(const bc19::Coordinate &coordinate) {
  return "Coordinate{" + ("row_: " + std::to_string(coordinate.row_)) + ", "
      + ("col_: " + std::to_string(coordinate.col_)) + "}";
}
}
#endif //CPP_STARTER_COORDINATE_H
