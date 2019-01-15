

#include "AttackerRobot.h"
#include "cpp_starter.h"

namespace bc19 {

emscripten::val AttackerRobot::onTurn() {
  // TODO: cache old enemy positions
  // TODO: signal enemy positions using longer vision units
  nearby_enemies_ = getNearbyEnemies();
  if (!nearby_enemies_.empty()) {
    return drawValueMaps();
  }

  static const Coordinate
      start_tile = Coordinate(static_cast<Coordinate::DimType>(me().y()), static_cast<Coordinate::DimType>(me().x()));
  if (me().turnCount() < 150) {
    const auto dest = symmetry_.flipCoord(start_tile);
    if (me().unit() == specs::Unit::PREACHER) {
      // gotta go fast
      return pathfinder_.pathTowardQuickly(dest);
    } else {
      return pathfinder_.pathTowardCheaply(dest);
    }

  }

  // TODO: pathfind to clusters, to clear out hidden expos
  return pathfinder_.pathToRandomTile();
}

std::vector<Robot> AttackerRobot::getNearbyEnemies() const {
  std::vector<Robot> result;
  const auto &visible_robots = getVisibleRobots();
  const auto &length = visible_robots["length"].as<int>();
  const auto &us = me().team();
  for (int i = 0; i < length; ++i) {
    Robot robot(visible_robots[i]);
    if (!isVisible(robot)) {
      continue;
    }
    if (robot.team() != us) {
      result.emplace_back(robot);
    }
  }
  return result;
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
  attack_coord_.reset();

  {
    double damage_taken = 0;
    // we stand still, they shoot us
    for (const auto &robot : nearby_enemies_) {
      Coordinate their_loc(static_cast<Coordinate::DimType>(robot.y()), static_cast<Coordinate::DimType>(robot.x()));
      specs::Unit their_type(robot.unit());
      const auto &their_specs = specs::units[static_cast<int>(their_type)];
      const auto our_dist_sq = my_loc.distSq(their_loc);

      // if they can shoot us, the value of staying here is lower
      if (withinAttackRadius(their_type, our_dist_sq)) {
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
      if (withinAttackRadius(their_type, our_dist_sq)) {
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
    tile_values_.get(my_loc) += computeBestValueAttack(my_type, my_loc);
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
    if (attack_coord_) {
      const auto offset = *attack_coord_ - my_loc;
      //      log("attacking by " + std::to_string(offset));
      return attack(offset.col_, offset.row_);
    } else {
      //      log("staying still");
      return nullAction();
    }
  } else {
    const auto offset = best_tile - my_loc;
    //    log("moving by " + std::to_string(offset));
    return move(offset.col_, offset.row_);
  }
}

double AttackerRobot::computeBestValueAttack(const specs::Unit &my_type, const Coordinate my_loc) {
  double max_value = std::numeric_limits<double>::lowest();
  std::optional<Coordinate> best_attack_coord;
  const auto &my_specs = specs::units[static_cast<int>(my_type)];
  const auto &us = me().team();
  if (my_type == specs::Unit::PREACHER) {
    max_value = 0.f; // don't go shooting if we can't hit something good
    const auto &visible_robot_map = getVisibleRobotMap();
    for (const auto &offset : directions::preacher_effective_targets) {
      double value = 0.;
      const auto center = my_loc + offset;
      if (!pathfinder_.onMap(center)) {
        continue;
      }
      if (!pathfinder_.passable(center)) {
        continue;
      }
      for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
          const auto
              tile = center + Coordinate(static_cast<Coordinate::DimType>(dr), static_cast<Coordinate::DimType>(dc));
          if (my_loc.distSq(tile) > my_specs.vision_radius) {
            continue;
          }
          if (!pathfinder_.onMap(tile)) {
            continue;
          }
          int id = visible_robot_map.get(tile.row_, tile.col_);
          if (id == 0) {
            continue;
          }
          Robot them(getRobot(id));

          double weight;
          if (them.team() == us) {
            weight = -3.0;
          } else {
            const auto &their_type = them.unit();
            switch (their_type) {
            case specs::Unit::CASTLE:
              weight = 0.5;
              break;
            case specs::Unit::CHURCH:
              weight = 0.1;
              break;
            case specs::Unit::PILGRIM:
              weight = 0.25;
              break;
            case specs::Unit::CRUSADER:
              weight = 1.0;
              break;
            case specs::Unit::PROPHET:
              weight = 1.5;
              break;
            case specs::Unit::PREACHER:
              weight = 2.0;
              break;
            case specs::Unit::UNDEFINED:
              weight = 0.1;
              break;
            }
          }
          value += weight * my_specs.attack_damage;
        }
      }
      if (value > max_value) {
        max_value = value;
        best_attack_coord = center;
      }
    }
  } else {
    // attack this turn
    for (const auto &robot : nearby_enemies_) {
      Coordinate their_loc(static_cast<Coordinate::DimType>(robot.y()), static_cast<Coordinate::DimType>(robot.x()));
      specs::Unit their_type(robot.unit());
      const auto our_dist_sq = my_loc.distSq(their_loc);

      // if we can shoot them, the value of standing here is higher
      if (withinAttackRadius(my_type, our_dist_sq)) {
        double weight;
        switch (their_type) {
        case specs::Unit::CASTLE:
          weight = 0.5;
          break;
        case specs::Unit::CHURCH:
          weight = 0.1;
          break;
        case specs::Unit::PILGRIM:
          weight = 0.25;
          break;
        case specs::Unit::CRUSADER:
          weight = 1.0;
          break;
        case specs::Unit::PROPHET:
          weight = 1.5;
          break;
        case specs::Unit::PREACHER:
          weight = 2.0;
          break;
        case specs::Unit::UNDEFINED:
          weight = 0.1;
          break;
        }
        double value = weight * my_specs.attack_damage;
        if (value > max_value) {
          max_value = value;
          best_attack_coord = their_loc;
        }
      }
    }
  }

  attack_coord_ = best_attack_coord;
  return max_value;
}

double AttackerRobot::computeBestValueNextAttack(const specs::Unit &my_type, const Coordinate tile) {
  // TODO: attack next turn; assume opponent remove all but the strongest unit
  // for now, assume they just retreat and we can do nothing
  return 0.;
}

bool AttackerRobot::withinAttackRadius(const specs::Unit &unit, const Coordinate::DimSqType &sq) {
  const auto &range = specs::units[static_cast<int>(unit)].attack_radius;
  switch (unit) {
  case specs::Unit::CRUSADER:
  case specs::Unit::PROPHET:
    return range[0] <= sq && sq <= range[1];
  case specs::Unit::PREACHER:
    // extra cautious if we're a ranger and they're a preacher
    return range[0] <= sq
        && sq <= (me().unit() == specs::Unit::PROPHET ? 36 : directions::preacher_effective_max_radius);
  default:
    return false;
  }
}

}