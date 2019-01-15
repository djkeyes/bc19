

#include <algorithm>
#include "Pathfinder.h"
#include "CommonRobot.h"
#include "fast_rand.h"

namespace bc19 {

emscripten::val Pathfinder::singlePassPathfind(const Coordinate &from,
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

template<typename T>
class CircularQueue {
 private:
  std::vector<T> data_;
  int head_;
  int tail_;
  int size_;

 public:
  explicit CircularQueue(int max_size)
      : data_(static_cast<typename std::vector<T>::size_type>(max_size)), head_(0), tail_(0), size_(0) {
  }

  void push(T element) {
    data_[tail_++] = std::move(element);
    tail_ %= data_.size();
    size_++;
  }

  T pop() {
    const T result(std::move(data_[head_++]));
    head_ %= data_.size();
    size_--;
    return result;
  }

  bool empty() {
    return size_ == 0;
  }
};

emscripten::val Pathfinder::bfsPathfind(const Coordinate &from,
                                        const Coordinate &to,
                                        const bool allow_diagonals) const {
  static Grid<uint16_t> distances(passable_map_.rows_, passable_map_.cols_);
  for (unsigned short &iter : distances.data) {
    iter = std::numeric_limits<uint16_t>::max();
  }

  const auto &occupied_map = self_->getVisibleRobotMap();

  CircularQueue<Coordinate> queue(2 * (passable_map_.rows_ + passable_map_.cols_ + 2));
  // search backwards from the destination
  // if we're doing more than 1 search, it would make more sense to start at the origin, then back trace the path
  queue.push(to);
  distances.set(to, 0);
  static const int radius_sq_to_ignore_occupied = 5 * 5;
  while (!queue.empty()) {
    const auto cur = queue.pop();

    if (cur == from) {
      // find which direction came here
      Coordinate cheapest_dir(1, 0);
      auto cheapest_cost = std::numeric_limits<uint16_t>::max();
      if (allow_diagonals) {
        for (const auto &dir : directions::adjacent_spiral) {
          const auto prev = cur + dir;
          if (prev.row_ >= 0 && prev.col_ >= 0 && prev.row_ < passable_map_.rows_ && prev.col_ < passable_map_.cols_) {
            const auto dist = distances.get(prev);
            if (dist < cheapest_cost) {
              cheapest_cost = dist;
              cheapest_dir = dir;
            }
          }
        }
      } else {
        for (const auto &dir : directions::horiz_adjacent) {
          const auto prev = cur + dir;
          if (prev.row_ >= 0 && prev.col_ >= 0 && prev.row_ < passable_map_.rows_ && prev.col_ < passable_map_.cols_) {
            const auto dist = distances.get(prev);
            if (dist < cheapest_cost) {
              cheapest_cost = dist;
              cheapest_dir = dir;
            }
          }
        }
      }
      return self_->move(cheapest_dir.col_, cheapest_dir.row_);
    }

    const uint16_t next_dist = distances.get(cur) + static_cast<const uint16_t>( 1);
    for (const auto &dir : directions::horiz_adjacent) {
      const auto next = cur + dir;
      if (next.row_ >= 0 && next.col_ >= 0 && next.row_ < passable_map_.rows_ && next.col_ < passable_map_.cols_
          && passable_map_.get(next)) {
        if (distances.get(next) == std::numeric_limits<uint16_t>::max()) {
          // TODO: until we improve the JS api, it's probably fastest to call this last
          // TODO: we should probably check and remember where buildings are
          if (from.distSq(next) >= radius_sq_to_ignore_occupied || occupied_map.get(next.row_, next.col_) == 0
              || from == next) {
            distances.set(next, next_dist);
            queue.push(next);
          }
        }
      }
    }
  }

  // couldn't reach it. Are we trapped by units?
  return self_->nullAction();
}

emscripten::val Pathfinder::dijkstraPathfind(const Coordinate &from,
                                             const Coordinate &to,
                                             Coordinate::DimSqType max_radius_sq) const {
  if (self_->fuel() < specs::units[static_cast<int>(self_->me().unit())].fuel_per_move) {
    // can't do anything =/
    return self_->nullAction();
  }

  if (max_radius_sq <= 2) {
    return bfsPathfind(from, to, max_radius_sq == 2);
  }

  // this naming is bad. This isn't dijkstra's, but the issue is that we have to different cost functions (time vs
  // fuel).
  // TODO: clean up

  static Grid<uint16_t> distances(passable_map_.rows_, passable_map_.cols_);
  for (unsigned short &iter : distances.data) {
    iter = std::numeric_limits<uint16_t>::max();
  }

  const auto &occupied_map = self_->getVisibleRobotMap();

  CircularQueue<Coordinate> queue(2 * max_radius_sq * (passable_map_.rows_ + passable_map_.cols_ + 2));
  // search backwards from the destination
  // if we're doing more than 1 search, it would make more sense to start at the origin, then back trace the path
  queue.push(to);
  distances.set(to, 0);
  static const int visibility_radius = specs::units[static_cast<int>(self_->me().unit())].vision_radius;
  while (!queue.empty()) {
    const auto cur = queue.pop();

    if (cur == from) {
      // find which direction came here
      Coordinate cheapest_dir(1, 0);
      auto cheapest_cost = std::numeric_limits<uint16_t>::max();

      for (Coordinate::DimSqType row = 0; row * row <= max_radius_sq; ++row) {
        for (Coordinate::DimSqType col = 0; col * col + row * row <= max_radius_sq; ++col) {
          if (row == 0 && col == 0) {
            continue;
          }
          for (Coordinate::DimType rsign = -1; rsign <= 1; rsign += 2) {
            for (Coordinate::DimType csign = -1; csign <= 1; csign += 2) {
              Coordinate delta = Coordinate(static_cast<Coordinate::DimType>(row) * rsign,
                                            static_cast<Coordinate::DimType>(col) * csign);
              const Coordinate prev = cur + delta;
              if (prev.row_ >= 0 && prev.col_ >= 0 && prev.row_ < passable_map_.rows_
                  && prev.col_ < passable_map_.cols_) {
                const auto dist = distances.get(prev);
                if (dist < cheapest_cost) {
                  cheapest_cost = dist;
                  cheapest_dir = delta;
                }
              }
            }
          }
        }
      }

//      self_->log("moving to " + std::to_string(to) + " from " + std::to_string(from));
      //      for (int row = 0; row < distances.rows_; ++row) {
      //        std::string foo = "";
      //        for (int col = 0; col < distances.cols_; ++col) {
      //          foo += "[" + std::to_string(distances.get(row, col)) + "]";
      //        }
      //        self_->log(foo);
      //      }
      //
      //      self_->log("cheapest dir: " + std::to_string(cheapest_dir) + ", cost: " + std::to_string(cheapest_cost));
      return self_->move(cheapest_dir.col_, cheapest_dir.row_);
    }

    const uint16_t next_dist = distances.get(cur) + static_cast<const uint16_t>(1);
    for (Coordinate::DimSqType row = 0; row * row <= max_radius_sq; ++row) {
      for (Coordinate::DimSqType col = 0; col * col + row * row <= max_radius_sq; ++col) {
        for (Coordinate::DimType rsign = -1; rsign <= 1; rsign += 2) {
          for (Coordinate::DimType csign = -1; csign <= 1; csign += 2) {
            Coordinate delta = Coordinate(static_cast<Coordinate::DimType>(row) * rsign,
                                          static_cast<Coordinate::DimType>(col) * csign);
            Coordinate next = cur + delta;
            if (next.row_ >= 0 && next.col_ >= 0 && next.row_ < passable_map_.rows_ && next.col_ < passable_map_.cols_
                && passable_map_.get(next)) {
              if (distances.get(next) == std::numeric_limits<uint16_t>::max()) {
                // TODO: until we improve the JS api, it's probably fastest to call this last
                // TODO: we should probably check and remember where buildings are
                if (from.distSq(next) > visibility_radius || occupied_map.get(next.row_, next.col_) == 0
                    || from == next) {
                  distances.set(next, next_dist);
                  queue.push(next);
                }
              }
            }
          }
        }
      }
    }
  }


  // couldn't reach it. Are we trapped by units?
  return self_->nullAction();
}

emscripten::val Pathfinder::pathTowardCheaply(const Coordinate &coordinate) {
  Coordinate
      my_loc(static_cast<Coordinate::DimType>(self_->me().y()), static_cast<Coordinate::DimType>(self_->me().x()));
  return dijkstraPathfind(my_loc, coordinate, 2);
}

emscripten::val Pathfinder::pathTowardQuickly(const Coordinate &coordinate) {
  const auto &m = self_->me();
  Coordinate my_loc(static_cast<Coordinate::DimType>(m.y()), static_cast<Coordinate::DimType>(m.x()));
  return dijkstraPathfind(my_loc,
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

emscripten::val Pathfinder::pathToRandomTile() {
  int row = fast_rand::small_uniform_int_rand(static_cast<const uint32_t>(passable_map_.rows_));
  int col = fast_rand::small_uniform_int_rand(static_cast<const uint32_t>(passable_map_.cols_));
  Coordinate target =
      getNearbyPassableTile(Coordinate(static_cast<Coordinate::DimType>(row), static_cast<Coordinate::DimType>(col)));
  return pathTowardCheaply(target);
}

}

