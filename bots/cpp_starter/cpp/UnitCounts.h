

#ifndef CPP_STARTER_UNITCOUNTS_H
#define CPP_STARTER_UNITCOUNTS_H

#include <string>

namespace bc19 {

/**
 * Container for the count of each unit, to be approximated by some means.
 */
struct UnitCounts {
  int num_unknown_ = 0;
  int num_castles_ = 0;
  int num_churches_ = 0;
  int num_pilgrims_ = 0;
  int num_crusaders_ = 0;
  int num_preachers_ = 0;
  int num_prophets_ = 0;

  int movableUnitsBuilt() const {
    return num_pilgrims_ + num_crusaders_ + num_preachers_ + num_prophets_;
  }
};

}

namespace std {

// TODO: only enable this in debug mode
std::string to_string(const bc19::UnitCounts &unit_counts) {
  return "UnitCounts{" + ("num_unknown_: " + std::to_string(unit_counts.num_unknown_)) + ", "
      + ("num_castles_: " + std::to_string(unit_counts.num_castles_)) + ", "
      + ("num_churches_: " + std::to_string(unit_counts.num_churches_)) + ", "
      + ("num_pilgrims_: " + std::to_string(unit_counts.num_pilgrims_)) + ", "
      + ("num_crusaders_: " + std::to_string(unit_counts.num_crusaders_)) + ", "
      + ("num_preachers_: " + std::to_string(unit_counts.num_preachers_)) + ", "
      + ("num_prophets_: " + std::to_string(unit_counts.num_prophets_)) + "}";
}
}
#endif //CPP_STARTER_UNITCOUNTS_H
