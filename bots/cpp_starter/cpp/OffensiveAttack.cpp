#include "OffensiveAttack.h"
#include "CommonRobot.h"
#include "Pathfinder.h"

namespace bc19 {

double OffensiveAttack::computeBestValueAttack(const specs::Unit &my_type,
                                               const Coordinate my_loc,
                                               const Pathfinder &pathfinder,
                                               const std::vector<Robot> &nearby_enemies,
                                               const bool is_castle_under_attack) {
  attack_coord_.reset();
  double max_value = std::numeric_limits<double>::lowest();
  std::optional<Coordinate> best_attack_coord;
  const auto &my_specs = specs::units[static_cast<int>(my_type)];
  const auto &us = self_->me().team();
  if (my_type == specs::Unit::PREACHER) {
    max_value = 0.f; // don't go shooting if we can't hit something good
    const auto &visible_robot_map = self_->getVisibleRobotMap();
    for (const auto &offset : directions::preacher_effective_targets) {
      double value = 0.;
      const auto center = my_loc + offset;
      if (!pathfinder.onMap(center)) {
        continue;
      }
      for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
          const auto
              tile = center + Coordinate(static_cast<Coordinate::DimType>(dr), static_cast<Coordinate::DimType>(dc));
          if (my_loc.distSq(tile) > my_specs.vision_radius) {
            continue;
          }
          if (!pathfinder.onMap(tile)) {
            continue;
          }
          int id = visible_robot_map.get(tile.row_, tile.col_);
          if (id == 0) {
            continue;
          }
          Robot them(self_->getRobot(id));

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
    for (const auto &robot : nearby_enemies) {
      Coordinate their_loc(static_cast<Coordinate::DimType>(robot.y()), static_cast<Coordinate::DimType>(robot.x()));
      specs::Unit their_type(robot.unit());
      const auto our_dist_sq = my_loc.distSq(their_loc);

      // if we can shoot them, the value of standing here is higher
      if (withinAttackRadius(my_type, our_dist_sq, is_castle_under_attack)) {
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

bool OffensiveAttack::withinAttackRadius(const specs::Unit &unit,
                                         const Coordinate::DimSqType &sq,
                                         const bool is_castle_under_attack) const {
  const auto &range = specs::units[static_cast<int>(unit)].attack_radius;
  switch (unit) {
  case specs::Unit::CRUSADER:
  case specs::Unit::PROPHET:
    return range[0] <= sq && sq <= range[1];
  case specs::Unit::PREACHER:
    // extra cautious if we're a ranger and they're a preacher
    if (self_->me().unit() == specs::Unit::PROPHET && !is_castle_under_attack) {
      return range[0] <= sq && sq <= 36;
    } else {
      return range[0] <= sq && sq <= directions::preacher_effective_max_radius;
    }
  default:
    return false;
  }
}

emscripten::val OffensiveAttack::createAttackAction() {
  if (attack_coord_) {
    const auto &m = self_->me();
    const Coordinate my_loc(static_cast<Coordinate::DimType>(m.y()), static_cast<Coordinate::DimType>(m.x()));
    const auto offset = *attack_coord_ - my_loc;
    return self_->attack(offset.col_, offset.row_);
  } else {
    return self_->nullAction();
  }
}
}