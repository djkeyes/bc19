

#include "AttackerRobot.h"
#include "cpp_starter.h"
#include "fast_rand.h"

namespace bc19 {

emscripten::val AttackerRobot::onTurn() {
  // TODO: cache old enemy positions
  // TODO: signal enemy positions using longer vision units
  parseNearbyUnits();
  if (!nearby_enemies_.empty()) {
    return drawValueMaps();
  }

  static const Coordinate
      start_tile = Coordinate(static_cast<Coordinate::DimType>(me().y()), static_cast<Coordinate::DimType>(me().x()));
  if (me().turnCount() < 100) {
    const auto dest = symmetry_.flipCoord(start_tile);
    if (me().unit() == specs::Unit::PREACHER) {
      // gotta go fast
      return pathfinder_.pathTowardQuickly(dest);
    } else {
      return pathfinder_.pathTowardCheaply(dest);
    }
  }

  const auto tile = pathfinder_.getNearbyPassableTile(clusterer_.clusters()[cluster_index_].centroid());
  ++turns_pathing_;
  if ((turns_pathing_ % 50 == 0) || (Coordinate(static_cast<Coordinate::DimType>(me().y()),
                                                static_cast<Coordinate::DimType>(me().x())).distSq(tile) < 25)) {
    ++cluster_index_;
    turns_pathing_ = 0;
    cluster_index_ %= clusterer_.clusters().size();
  }

  return pathfinder_.pathTowardCheaply(tile);
}

void AttackerRobot::parseNearbyUnits() {
  nearby_castle_.reset();
  is_castle_under_attack_ = false;

  nearby_enemies_.clear();
  nearby_allies_.clear();
  const auto &visible_robots = getVisibleRobots();
  const auto &length = visible_robots["length"].as<int>();
  const auto &us = me().team();
  for (int i = 0; i < length; ++i) {
    Robot robot(visible_robots[i], this);

    if (!isVisible(robot)) {
      continue;
    }
    if (robot.team() != us) {
      nearby_enemies_.emplace_back(robot);
    } else {
      nearby_allies_.emplace_back(robot);
      if (robot.unit() == specs::Unit::CASTLE) {
        nearby_castle_ =
            Coordinate(static_cast<Coordinate::DimType>(robot.y()), static_cast<Coordinate::DimType>(robot.x()));
      }
    }
  }
  if (nearby_castle_) {
    for (const auto &enemy : nearby_enemies_) {
      if (nearby_castle_->distSq(Coordinate(static_cast<Coordinate::DimType>(enemy.y()),
                                            static_cast<Coordinate::DimType>(enemy.x()))))
        is_castle_under_attack_ = true;
      break;
    }
  }
}

// TODO: refactor this
emscripten::val AttackerRobot::drawValueMaps() {
  ++version_;

  const auto &m = me();
  const Coordinate my_loc(static_cast<Coordinate::DimType>(m.y()), static_cast<Coordinate::DimType>(m.x()));
  const auto my_type = m.unit();
  const auto &my_specs = specs::units[static_cast<int>(my_type)];
  const auto cur_fuel = fuel();
  bool can_afford_attacking = (cur_fuel >= my_specs.attack_fuel_cost);
  const auto &occupied_map = getVisibleRobotMap();
  tile_versions_.get(my_loc) = version_;
  tile_values_.get(my_loc) = 0.;
  Coordinate const *moves = nullptr;
  int num_moves = 0;

  switch (my_type) {
  case specs::Unit::CRUSADER:
    moves = directions::crusader_moves.data();
    num_moves = static_cast<int>(directions::crusader_moves.size());
    break;
  case specs::Unit::PROPHET:
    moves = directions::prophet_moves.data();
    num_moves = static_cast<int>(directions::prophet_moves.size());
    break;
  case specs::Unit::PREACHER:
    moves = directions::preacher_moves.data();
    num_moves = static_cast<int>(directions::preacher_moves.size());
    break;
  default:
    break;
  }

  // reset
  for (int i = 0; i < num_moves; ++i) {
    const auto &offset = moves[i];
    const auto tile = my_loc + offset;
    if (my_loc.distSq(tile) * my_specs.fuel_per_move > cur_fuel) {
      continue;
    }
    if (!pathfinder_.onMap(tile)) {
      continue;
    }
    if (!pathfinder_.passable(tile)) {
      continue;
    }
    if (occupied_map.get(tile.row_, tile.col_) != 0) {
      continue;
    }
    tile_versions_.get(tile) = version_;
    tile_values_.get(tile) = 0.;
  }

  {
    double damage_taken = 0;
    // we stand still, they shoot us
    for (const auto &robot : nearby_enemies_) {
      Coordinate their_loc(static_cast<Coordinate::DimType>(robot.y()), static_cast<Coordinate::DimType>(robot.x()));
      specs::Unit their_type(robot.unit());
      const auto &their_specs = specs::units[static_cast<int>(their_type)];
      const auto our_dist_sq = my_loc.distSq(their_loc);

      // if they can shoot us, the value of staying here is lower
      if (attack_.withinAttackRadius(their_type, our_dist_sq, is_castle_under_attack_)) {
        damage_taken += their_specs.attack_damage;
      }
    }
    // TODO: remove copy pasta
    if (damage_taken >= m.health()) {
      // don't want to die
      damage_taken *= 100;
    }
    tile_values_.get(my_loc) -= damage_taken;
  }

  // we move, they shoot us
  for (int i = 0; i < num_moves; ++i) {
    const auto &offset = moves[i];
    const auto tile = my_loc + offset;

    if (!pathfinder_.onMap(tile)) {
      continue;
    }
    if (tile_versions_.get(tile) != version_) {
      continue;
    }

    double damage_taken = 0;

    for (const auto &robot : nearby_enemies_) {
      Coordinate their_loc(static_cast<Coordinate::DimType>(robot.y()), static_cast<Coordinate::DimType>(robot.x()));
      specs::Unit their_type(robot.unit());
      const auto &their_specs = specs::units[static_cast<int>(their_type)];
      const auto our_dist_sq = tile.distSq(their_loc);
      // if they can shoot us, the value of staying here is lower
      if (attack_.withinAttackRadius(their_type, our_dist_sq, is_castle_under_attack_)) {
        damage_taken += their_specs.attack_damage;
      }

      if (damage_taken >= m.health()) {
        break;
      }
    }
    if (damage_taken >= m.health()) {
      // don't want to die
      damage_taken *= 100;
    }
    tile_values_.get(tile) -= damage_taken;
  }

  if (can_afford_attacking) {
    tile_values_.get(my_loc) +=
        attack_.computeBestValueAttack(my_type, my_loc, pathfinder_, nearby_enemies_, is_castle_under_attack_);
  }

  // TODO
  //  // we move, then we shoot
  //  for (int i = 0; i < num_moves; ++i) {
  //    const auto &offset = moves[i];
  //    const auto tile = my_loc + offset;
  //
  //    if (tile_versions_.get(tile) != version_) {
  //      continue;
  //    }
  //
  //    tile_values_.get(tile) += computeBestValueAttack(my_type, tile);
  //  }


  //  for (int row = std::max(my_loc.row_ - 6, 0); row <= std::min(my_loc.row_ + 6, tile_values_.cols_); ++row) {
  //    std::string foo = "";
  //    for (int col = std::max(my_loc.col_ - 6, 0); col <= std::min(my_loc.col_ + 6, tile_values_.cols_); ++col) {
  //      foo += "[" + (tile_versions_.get(row, col) == version_ ? std::to_string(tile_values_.get(row, col)) : "  ////  ")
  //          + "]";
  //    }
  //    log(foo);
  //  }

  // pragmatically speaking, we don't really want to stand next to allies if there are tanks nearby
  // ...and there are always tanks
  for (const auto &robot : nearby_allies_) {
    Coordinate their_loc(static_cast<Coordinate::DimType>(robot.y()), static_cast<Coordinate::DimType>(robot.x()));
    for (const auto &dir : directions::adjacent_spiral) {
      const auto &tile = their_loc + dir;
      const auto our_dist_sq = tile.distSq(their_loc);
      // encourage diagonal movement a little
      const auto distsq = dir.distSq(Coordinate());
      if (our_dist_sq <= my_specs.speed) {
        if (tile_versions_.get(tile) == version_) {
          tile_values_.get(tile) -= 20.0f / distsq;
        }
      }
    }
  }

  // choose the best
  double max_val = tile_values_.get(my_loc);
  Coordinate best_tile = my_loc;
  for (int i = 0; i < num_moves; ++i) {
    const auto &offset = moves[i];
    const auto tile = my_loc + offset;
    if (!pathfinder_.onMap(tile)) {
      continue;
    }
    if (tile_versions_.get(tile) != version_) {
      continue;
    }
    auto value = tile_values_.get(tile);
    if (value > max_val) {
      max_val = value;
      best_tile = tile;
    }
  }

  if (best_tile == my_loc) {
    return attack_.createAttackAction();
  } else {
    const auto offset = best_tile - my_loc;
    //    log("moving by " + std::to_string(offset));
    return move(offset.col_, offset.row_);
  }
}

}