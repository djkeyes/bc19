

#ifndef CPP_STARTER_CPP_STARTER_H
#define CPP_STARTER_CPP_STARTER_H

#include <emscripten/val.h>

// TODO: how fast is the js-native boundary layer? is it faster to copy values?

namespace bc19 {
namespace specs {
enum class Unit : int {
  CASTLE = 0,
  CHURCH,
  PILGRIM,
  CRUSADER,
  PROPHET,
  PREACHER,
  UNDEFINED
};
}

class Robot {
 private:
  // TODO: can this be a reference?
  emscripten::val jsRobot_;

  // TODO: make various factory methods for this
  Robot(emscripten::val jsRobot) : jsRobot_(jsRobot) {
  }

 public:
  static Robot fromSelfRobot(const emscripten::val &jsAbstractRobot) {
    return Robot(jsAbstractRobot["me"]);
  }

  /**
   * The id of the robot, which is an integer between 1 and {@link Specs.MAX_ID}.
   *
   * Always available.
   */
  int id() const {
    // TODO
    return -1;
  }

  /**
   * The robot's unit type, where {@link Specs.CASTLE} stands for castle,
   * {@link Specs.CHURCH} stands for church, {@link Specs.PILGRIM} stands for pilgrim,
   * {@link Specs.CRUSADER} stands for crusader, {@link Specs.PROPHET} stands for prophet
   * and {@link Specs.PREACHER} stands for preacher.
   *
   * Available if visible.
   */
  specs::Unit unit() const {
    auto val = jsRobot_["unit"];
    return val.isUndefined() ? specs::Unit::UNDEFINED : static_cast<specs::Unit>(val.as<int>());
  }

  /**
   * The health of the robot.
   *
   * Only available for `r = this.me`.
   */
  int health() const {
    // TODO assert r = this.me
    return jsRobot_["health"].as<int>();
  }

  /**
   * The team of the robot, where {@link Specs.RED} stands for RED and {@link Specs.BLUE} stands for BLUE.
   *
   * Available if visible.
   */
//  team?:
//  number;

  /**
   * The x position of the robot.
   *
   * Available if visible.
   */
  int x() const {
    return jsRobot_["x"].as<int>();
  }
  /**
   * The y position of the robot.
   *
   * Available if visible.
   */
  int y() const {
    return jsRobot_["y"].as<int>();
  }

  /**
   * The amount of fuel that the robot carries.
   *
   * Only available if {@link BCAbstractRobot.me} equals this robot.
   */
  int fuel() {
    // TODO
    return -1;
  }

  /**
   * The amount of karbonite that the robot carries.
   *
   * Only available if {@link BCAbstractRobot.me} equals this robot.
   */
  int karbonite() {
    // TODO
    return -1;
  }

  /**
   * The turn count of the robot (initialized to 0, and incremented just before `turn()`).
   *
   * Always available.
   */
  int turn() const {
    return jsRobot_["turn"].as<int>();
  }

  /**
   * The signal message of the robot.
   *
   * -1 if not radioable.
   */
//  signal: number;

  /**
   * The signal radius of the robot.
   *
   * -1 if not radioable.
   */
//  signal_radius: number;

  /**
   * The castle talk message sent by the robot.
   *
   * Available if {@link BCAbstractRobot.me} is a Castle.
   */
//  castle_talk?:
//  number;

  /**
   * The amount of milliseconds this robot has left in it's chess clock.
   */
//  time: number;
};

class AbstractNativeRobot {
 private:
  // TODO: I think this is reference counted. Does it ever get deallocated?
  emscripten::val jsAbstractRobot_;

 protected:
  explicit AbstractNativeRobot(emscripten::val jsAbstractRobot) : jsAbstractRobot_(jsAbstractRobot) {}

 public:
  AbstractNativeRobot(const AbstractNativeRobot &) = delete;
  AbstractNativeRobot(AbstractNativeRobot &&) = delete;
  AbstractNativeRobot &operator=(const AbstractNativeRobot &) = delete;
  AbstractNativeRobot &operator=(AbstractNativeRobot &&) = delete;
  virtual ~AbstractNativeRobot() = default;

  /**
   * Competitors must subclass AbstractNativeRobot, and they must implement this function to provide a concrete
   * implementation of the class. The parameter `jsAbstractRobot` is meant to be passed to the AbstractNativeRobot
   * constructor. The parameter points to the javascript instance of BCAbstractRobot, and so players may use
   * this to reason about the game state during the robot's first turn. For example, they might check what kind of
   * unit is currently being controlled, and provide different concrete subclasses based on the type.
   *
   * @param jsAbstractRobot the JS BCAbstractRobot instance
   * @return An instance of a subclass of AbstractNativeRobot, containing the team's code
   */
  static std::unique_ptr<AbstractNativeRobot> createNativeRobotImpl(emscripten::val jsAbstractRobot);

  /**
   * This is called once every turn.
   *
   * @return A valid javascript action object.
   */
  virtual emscripten::val turn() = 0;

  /**
   * The robot object of the playing robot.
   */
  Robot me() {
    // TODO: for some frequently used methods, it's probably better to cache
    return Robot::fromSelfRobot(jsAbstractRobot_);
  }

  /**
   * The id of the playing robot.
   */
  int id() {
    // TODO
    return -1;
  }

  /**
   * The global amount of fuel that the team possesses.
   */
  int fuel() {
    // TODO
    return -1;
  }

  /**
   * The global amount of karbonite that the team possesses.
   */
  int karbonite() {
    // TODO
    return -1;
  }

  /**
   * A 2 by 2 grid containing the last trade offers by both teams.
   *
   * last_offer[{@link Specs.RED}] is the last offer made by RED and contains an array of two integers.
   * Similarly, last_offer[{@link Specs.BLUE}] is the last offer made by BLUE.
   *
   * The first value in the array of integers is the amount of karbonite and the second one is the amount of fuel.
   * For both offers, a positive amount signifies that the resource goes from RED to BLUE.
   *
   * Available for castles (always `null` for other units).
   */
//  last_offer: number
//  [][] |
//  null;

  /**
   * The full map represented as a boolean grid where `true` indicates passable and `false` indicates impassable.
   */
//  map: boolean
//  [][];

  /**
   * The karbonite map represented as a boolean grid where `true` indicates that karbonite is present and `false` indicates that it is not.
   */
//  karbonite_map: boolean
//  [][];

  /**
   * The fuel map represented as a boolean grid where `true` indicates that fuel is present and `false` indicates that it is not.
   */
//  fuel_map: boolean
//  [][];


  /**
   * Print a message to the command line. You cannot use ordinary `console.log` in Battlecode for security reasons.
   *
   * The message is converted to a string using `JSON.stringify`.
   *
   * @param message - The message to log
   */
  void log(const std::string &message) const {
    // TODO only enable printing if this is a debug build
    jsAbstractRobot_.call<void>("log", message);
  }

  /**
   * Broadcast `value` to all robots within the squared radius `sq_radius`. Uses `sq_radius` Fuel.
   * Can be called multiple times in one `turn()`; however, only the most recent signal will be used, while each signal will cost fuel.
   *
   * @param value - The value to signal, which should be between 0 and 2^{@link Specs.COMMUNICATION_BITS}-1 (inclusive)
   * @param radius - The radius to signal in
   */
//  signal(value: number, radius: number
//  ): void;

  /**
   * Broadcast `value` to all castles of the same team. Does not use fuel.
   * Can be called multiple times in one `turn()`; however, only the most recent castle talk will be used.
   *
   * @param value - The number to broadcast, which should be between 0 and 2^{@link Specs.CASTLE_TALK_BITS}-1 (inclusive)
   */
//  castleTalk(value: number): void;

  /**
   * Propose a trade with the other team. `karbonite` and `fuel` need to be integers.
   *
   * For example, for RED to make the offer "I give you 10 Karbonite if you give me 10 Fuel", the parameters
   * would be `karbonite = 10` and `fuel = -10` (for BLUE, the signs are reversed).
   *
   * If the proposed trade is the same as the other team's `last_offer`, a trade is performed, after which the `last_offer` of both teams will be nullified.
   *
   * Only available for castles.
   *
   * @param karbonite - The amount of karbonite to propose
   * @param fuel - The amount of fuel to propose
   */
  emscripten::val proposeTrade(const int karbonite, const int fuel) const {
    // TODO: reimplement the JS logic here, to avoid unnecessary debug checks
    return jsAbstractRobot_.call<emscripten::val>("proposeTrade", karbonite, fuel);
  }

  /**
   * Build a unit of the type `unit` (integer, see `r.unit`) in the tile that is `dx` steps in the x direction and `dy` steps in the y direction from `this.me`.
   * Can only build in adjacent, empty and passable tiles.
   *
   * Uses {@link UnitSpecs.CONSTRUCTION_FUEL} fuel and {@link UnitSpecs.CONSTRUCTION_KARBONITE} karbonite (depending on the constructed unit).
   *
   * Available for pilgrims, castles and churches.
   *
   * Pilgrims can only build churches.
   * Castles and churches can only build pilgrims, crusaders, prophets and preachers.
   *
   * @param unit - The type of the unit to build
   * @param dx - The amount of steps away in the x direction to build
   * @param dy - The amount of steps away in the y direction to build
   */
  emscripten::val buildUnit(const specs::Unit &unit, const int dx, const int dy) const {
    // TODO: reimplement the JS logic here, to avoid unnecessary debug checks
    // TODO: assert unit != Unit::UNDEFINED
    return jsAbstractRobot_.call<emscripten::val>("buildUnit", static_cast<int>(unit), dx, dy);
  }

  /**
   * Move `dx` steps in the x direction, and `dy` steps in the y direction.
   *
   * Uses fuel (depending on unit and distance).
   *
   * Available for pilgrims, crusaders, prophets, preachers.
   *
   * @param dx - The amount of steps to move in the x direction
   * @param dy - The amount of steps to move in the y direction
   */
  emscripten::val move(const int dx, const int dy) const {
    // TODO: reimplement the JS logic here, to avoid unnecessary debug checks
    return jsAbstractRobot_.call<emscripten::val>("move", dx, dy);
  }

  /**
   * Mine {@link Specs.KARBONITE_YIELD} karbonite or {@link Specs.FUEL_YIELD} fuel, if on a corresponding resource tile.
   *
   * Uses {@link Specs.MINE_FUEL_COST} fuel. Available for pilgrims.
   */
  emscripten::val mine() const {
    // TODO: reimplement the JS logic here, to avoid unnecessary debug checks
    return jsAbstractRobot_.call<emscripten::val>("mine");
  }

  /**
   * Give `karbonite` karbonite and `fuel` fuel to the robot in the tile that is `dx` steps in the x direction and `dy` steps in the y direction from `this.me`.
   * A robot can only give to another robot that is in one of its 8 adjacent tiles, and cannot give more than it has.
   *
   * Uses 0 Fuel.
   *
   * Available for all robots.
   *
   * If a unit tries to give a robot more than its capacity, the excess is loss to the void.
   *
   * @param dx - The amount of steps away the receiving robot is in the x direction
   * @param dy - The amount of steps away the receiving robot is in the y direction
   * @param karbonite - The amount of karbonite to give to the receiving robot
   * @param fuel - The amount of fuel to give to the receiving robot
   */
//  give(
//      dx: number,
//      dy: number,
//      karbonite: number,
//      fuel: number,
//  ):
//  GiveAction;
  emscripten::val give(const int dx, const int dy, const int karbonite, const int number) const {
    // TODO: reimplement the JS logic here, to avoid unnecessary debug checks
    return jsAbstractRobot_.call<emscripten::val>("give", dy, dy, karbonite, number);
  }

  /**
   * Attack the robot in the tile that is `dx` steps in the x direction and `dy` steps in the y direction from `this.me`.
   * A robot can only attack another robot that is within its attack radius (depending on unit).
   *
   * Uses fuel (depending on unit).
   *
   * Available for crusaders, prophets and preachers.
   *
   * @param dx - The amount of steps away the attacked robot is in the x direction
   * @param dy - The amount of steps away the attacked robot is in the y direction
   */
  emscripten::val attack(const int dx, const int dy) const {
    // TODO: reimplement the JS logic here, to avoid unnecessary debug checks
    return jsAbstractRobot_.call<emscripten::val>("attack", dx, dy);
  }

  static emscripten::val nullAction() {
    return emscripten::val::null();;
  }

  /**
   * Returns a robot object with the given integer `id`.
   *
   * Returns `null` if such a robot is not in your vision (for castles, it also
   * returns a robot object for all robots on `this.me`'s team that are not in
   * the robot's vision, to access `castle_talk`).
   *
   * @param id - The id of the robot to retrieve
   */
//   Robot getRobot(int id) {
//    // TODO
//    return -1;
//  }

  /**
   * Returns `true` if the given robot object is visible.
   *
   * @param robot - The robot to check
   */
//
//  isVisible(robot: Robot): boolean;

  /**
   * Returns `true` if the given robot object is currently sending radio (signal).
   *
   * @param robot - The robot to check
   */
//
//  isRadioing(robot: Robot): boolean;

  /**
   * Returns {@link GameState.shadow}.
   */
//
//  getVisibleRobotMap() : number
//  [][];

  /**
   * Returns {@link map}.
   */
//
//  getPassableMap() : boolean
//  [][];

  /**
   * Returns {@link karbonite_map}.
   */
//
//  getKarboniteMap() : boolean
//  [][];

  /**
   * Returns {@link fuel_map}.
   */
//
//  getFuelMap() : boolean
//  [][];

  /**
   * Returns {@link GameState.visible}.
   */
//
//  getVisibleRobots() : Robot
//  [];

};
}
#endif //CPP_STARTER_CPP_STARTER_H
