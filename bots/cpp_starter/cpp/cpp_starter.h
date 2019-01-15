

#ifndef CPP_STARTER_CPP_STARTER_H
#define CPP_STARTER_CPP_STARTER_H

#include <emscripten/val.h>

// TODO: how fast is the js-native boundary layer? is it faster to copy values?

// TODO: this contains some hardcoded values. write a script to populate these constants from the game specs.

namespace bc19 {
namespace specs {

/**
 * The amount of bits that can be used when signaling (default: 16).
 */
constexpr int communication_bits = 16;

/**
 * The amount of bits that can be used when sending a message to the castles (default: 8).
 */
constexpr int castle_talk_bits = 8;

/**
 * The maximum amount of rounds this game can take (default: 1000).
 */
constexpr int max_rounds = 1000;

/**
 * The amount of fuel that is given per turn (default: 25).
 */
constexpr int trickle_fuel = 25;

/**
 * The initial amount of karbonite every player starts with (default: 100).
 */
constexpr int initial_karbonite = 100;

/**
 * The initial amount of fuel every player starts with (default: 500).
 */
constexpr int initial_fuel = 500;

/**
 * The amount of fuel it costs to mine once (default: 1).
 */
constexpr int mine_fuel_cost = 1;

/**
 * The amount of karbonite that can be mined from fields with karbonite (default: 2).
 */
constexpr int karbonite_yield = 2;

/**
 * The amount of karbonite that can be mined from fields with fuel (default: 10).
 */
constexpr int fuel_yield = 10;

/**
 * The maximum amount of goods that can be traded in a single turn (default: 1024).
 */
constexpr int max_trade = 1024;

/**
 * The maximum board size (default: 64).
 */
constexpr int max_board_size = 64;

/**
 * The maximum id of a unit (default: 4096).
 */
constexpr int max_id = 4096;

/**
 * The id of the red team (default: 0).
 */
constexpr int red = 1;

/**
 * The id of the blue team (default: 1).
 */
constexpr int blue = 1;

/**
 * The initial amount of milliseconds that is given to every robot (default: 100).
 */
constexpr int chess_initial = 100;

/**
 * The amount of extra milliseconds that a robot is given every turn (default: 20).
 */
constexpr int chess_extra = 20;

/**
 * The maximum amount of memory your robot can use in bytes (default: 50000000).
 *
 * At the time of writing, this limit is not enforced.
 * It is unclear whether this will change later in the competition.
 */
constexpr int max_memory = 50000000;

enum class Unit : int {
  CASTLE = 0, CHURCH, PILGRIM, CRUSADER, PROPHET, PREACHER, UNDEFINED
};

struct UnitSpecs {

  /**
   * The amount of karbonite it costs to construct this unit.
   */
  const int construction_karbonite;

  /**
   * The amount of fuel it costs to construct this unit.
   */
  const int construction_fuel;

  /**
   * The amount of karbonite this unit can carry.
   */
  const int karbonite_capacity;

  /**
   * The amount of fuel this unit can carry.
   */
  const int fuel_capacity;

  /**
   * The speed of this unit. 0 if this unit can't move.
   */
  const int speed;

  /**
   * The amount of fuel this unit needs per move.
   */
  const int fuel_per_move;

  /**
   * The amount of hp this unit starts with.
   */
  const int starting_hp;

  /**
   * The distance this unit can see.
   */
  const int vision_radius;

  /**
   * The amount of damage this unit does when attacking.
   */
  const int attack_damage;

  /**
   * An array specifying the minimum and maximum distance in which this unit can attack.
   */
  const std::array<int, 2> attack_radius;

  /**
   * The amount of fuel it takes for this unit to attack.
   */
  const int attack_fuel_cost;

  /**
   * How big the spread of this unit is when attacking.
   */
  const int damage_spread;
};

/**
 * An array of specs of all the different units.
 *
 * The index is the id of the unit as specified in the {@link SPECS}, like {@link PILGRIM} and {@link CRUSADER}.
 */
constexpr std::array<UnitSpecs, 6> units = {{{
    /*CONSTRUCTION_KARBONITE*/ -1,
    /*CONSTRUCTION_FUEL*/ -1,
    /*KARBONITE_CAPACITY*/ -1,
    /*FUEL_CAPACITY*/ -1,
    /*SPEED*/ 0,
    /*FUEL_PER_MOVE*/ -1,
    /*STARTING_HP*/ 100,
    /*VISION_RADIUS*/ 100,
    /*ATTACK_DAMAGE*/ -1,
    /*ATTACK_RADIUS*/ {{-1, -1}},
    /*ATTACK_FUEL_COST*/ -1,
    /*DAMAGE_SPREAD*/ -1
}, {
    /*CONSTRUCTION_KARBONITE*/ 50,
    /*CONSTRUCTION_FUEL*/ 200,
    /*KARBONITE_CAPACITY*/ -1,
    /*FUEL_CAPACITY*/ -1,
    /*SPEED*/ 0,
    /*FUEL_PER_MOVE*/ -1,
    /*STARTING_HP*/ 50,
    /*VISION_RADIUS*/ 100,
    /*ATTACK_DAMAGE*/ -1,
    /*ATTACK_RADIUS*/ {{-1, -1}},
    /*ATTACK_FUEL_COST*/ -1,
    /*DAMAGE_SPREAD*/ -1
}, {
    /*CONSTRUCTION_KARBONITE*/ 10,
    /*CONSTRUCTION_FUEL*/ 50,
    /*KARBONITE_CAPACITY*/ 20,
    /*FUEL_CAPACITY*/ 100,
    /*SPEED*/ 4,
    /*FUEL_PER_MOVE*/ 1,
    /*STARTING_HP*/ 10,
    /*VISION_RADIUS*/ 100,
    /*ATTACK_DAMAGE*/ -1,
    /*ATTACK_RADIUS*/ {{-1, -1}},
    /*ATTACK_FUEL_COST*/ -1,
    /*DAMAGE_SPREAD*/ -1
}, {
    /*CONSTRUCTION_KARBONITE*/ 20,
    /*CONSTRUCTION_FUEL*/ 50,
    /*KARBONITE_CAPACITY*/ 20,
    /*FUEL_CAPACITY*/ 100,
    /*SPEED*/ 9,
    /*FUEL_PER_MOVE*/ 1,
    /*STARTING_HP*/ 40,
    /*VISION_RADIUS*/ 36,
    /*ATTACK_DAMAGE*/ 10,
    /*ATTACK_RADIUS*/ {{1, 16}},
    /*ATTACK_FUEL_COST*/ 10,
    /*DAMAGE_SPREAD*/ 0
}, {
    /*CONSTRUCTION_KARBONITE*/ 25,
    /*CONSTRUCTION_FUEL*/ 50,
    /*KARBONITE_CAPACITY*/ 20,
    /*FUEL_CAPACITY*/ 100,
    /*SPEED*/ 4,
    /*FUEL_PER_MOVE*/ 2,
    /*STARTING_HP*/ 20,
    /*VISION_RADIUS*/ 64,
    /*ATTACK_DAMAGE*/ 10,
    /*ATTACK_RADIUS*/ {{16, 64}},
    /*ATTACK_FUEL_COST*/ 25,
    /*DAMAGE_SPREAD*/ 0
}, {
    /*CONSTRUCTION_KARBONITE*/ 30,
    /*CONSTRUCTION_FUEL*/ 50,
    /*KARBONITE_CAPACITY*/ 20,
    /*FUEL_CAPACITY*/ 100,
    /*SPEED*/ 4,
    /*FUEL_PER_MOVE*/ 3,
    /*STARTING_HP*/ 60,
    /*VISION_RADIUS*/ 16,
    /*ATTACK_DAMAGE*/ 20,
    /*ATTACK_RADIUS*/ {{1, 16}},
    /*ATTACK_FUEL_COST*/ 15,
    /*DAMAGE_SPREAD*/ 3
}}};
}

class Robot {
 private:
  // TODO: can this be a reference?
  emscripten::val jsRobot_;

 public:
  // TODO: make various factory methods for this
  Robot(emscripten::val jsRobot) : jsRobot_(jsRobot) {
  }

  static Robot fromSelfRobot(const emscripten::val &jsAbstractRobot) {
    return Robot(jsAbstractRobot["me"]);
  }

  auto &jsRobot() {
    return jsRobot_;
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
    // TODO: assert r = this.me
    return jsRobot_["health"].as<int>();
  }

  /**
   * The team of the robot, where {@link Specs.RED} stands for RED and {@link Specs.BLUE} stands for BLUE.
   *
   * Available if visible.
   */
  int team() const {
    return jsRobot_["team"].as<int>();
  }

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
  int fuel() const {
    // TODO: assert r = this.me
    return jsRobot_["fuel"].as<int>();
  }

  /**
   * The amount of karbonite that the robot carries.
   *
   * Only available if {@link BCAbstractRobot.me} equals this robot.
   */
  int karbonite() const {
    // TODO: assert r = this.me
    return jsRobot_["karbonite"].as<int>();
  }

  /**
   * The turn count of the robot (initialized to 0, and incremented just before `turn()`).
   *
   * In JS, this is an instance variable named 'turn'.
   *
   * Always available.
   */
  int turnCount() const {
    return jsRobot_["turn"].as<int>();
  }

  /**
   * The signal message of the robot.
   *
   * -1 if not radioable.
   */
  int signal() const {
    return jsRobot_["signal"].as<int>();
  }

  /**
   * The signal radius of the robot.
   *
   * -1 if not radioable.
   */
  int signal_radius() const {
    return jsRobot_["signal_radius"].as<int>();
  }

  /**
   * The castle talk message sent by the robot.
   *
   * Available if {@link BCAbstractRobot.me} is a Castle.
   */
  uint8_t castle_talk() const {
    // TODO: assert me.unit == CASTLE
    return jsRobot_["castle_talk"].as<uint8_t>();
  }

  /**
   * The amount of milliseconds this robot has left in it's chess clock.
   */
  int time() const {
    return jsRobot_["time"].as<int>();
  }
};

class AbstractNativeRobot {
 private:
  // TODO: I think this is reference counted. Does it ever get deallocated?
  emscripten::val jsAbstractRobot_;

 protected:
  explicit AbstractNativeRobot(emscripten::val jsAbstractRobot) : jsAbstractRobot_(jsAbstractRobot) {
  }

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
  Robot me() const {
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
  int fuel() const {
    return jsAbstractRobot_["fuel"].as<int>();
  }

  /**
   * The global amount of karbonite that the team possesses.
   */
  int karbonite() const {
    return jsAbstractRobot_["karbonite"].as<int>();
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
  void castleTalk(const uint8_t value) {
    // TODO: check preconditions, then assign to _bc_castle_talk directly
    jsAbstractRobot_.call<void>("castleTalk", value);
  }

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
  emscripten::val give(const int dx, const int dy, const int karbonite, const int fuel) const {
    // TODO: reimplement the JS logic here, to avoid unnecessary debug checks
    return jsAbstractRobot_.call<emscripten::val>("give", dx, dy, karbonite, fuel);
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
  bool isVisible(Robot &robot) {
    // TODO: we can implement this locally if we make a separate boolean for visibility in the robot class
    return jsAbstractRobot_.call<bool>("isVisible", robot.jsRobot());
  }

  /**
   * Returns `true` if the given robot object is currently sending radio (signal).
   *
   * @param robot - The robot to check
   */
  bool isRadioing(Robot &robot) {
    return robot.signal() > 0;
  }

  /**
   * Wrapper class for a 2d grid of numbers. This abstracts away the underlying implementation, which may change for
   * efficiency.
   * @tparam T The type of object stored.
   */
  template<typename T>
  class Map {
   private:
    emscripten::val internal_value_;

   public:
    // TODO: several of these maps don't change over time, but are likely to have frequent reads. Cache them on the first
    // turn to avoid stupid variable lookups. Profile to test the effect.
    // Even for the getVisibleRobotMap() (which changes every turn), we only need to update the visible tiles (at most
    // 357 for largest vision range), so it might be faster to store it locally.
    explicit Map(emscripten::val value) : internal_value_(value) {
    }

    /**
     * Get the element store at a particular column. Note that the row corresponds to the y-coordinate, and the
     * column corresponds to the x-coordinate.
     *
     * @param row the row
     * @param col the col
     * @return the value
     */
    T get(int row, int col) const {
      return internal_value_[row][col].template as<T>();
    }

    /**
     * Gets the number of rows (height) of the map
     * @return number of rows
     */
    int rows() const {
      return internal_value_["length"].template as<int>();
    }

    /**
     * Gets the number of cols (width) of the map
     * @return number of cols
     */
    int cols() const {
      return internal_value_[0]["length"].template as<int>();
    }
  };

  using MapBool = Map<bool>;
  using MapChar = Map<char>;
  using MapInt = Map<int>;

  /**
   * Returns {@link GameState.shadow}.
   */
  MapInt getVisibleRobotMap() const {
    return MapInt(jsAbstractRobot_.call<emscripten::val>("getVisibleRobotMap"));
  }

  /**
   * Returns {@link map}.
   */
  MapBool getPassableMap() const {
    return MapBool(jsAbstractRobot_.call<emscripten::val>("getPassableMap"));
  }

  /**
   * Returns {@link karbonite_map}.
   */
  MapBool getKarboniteMap() const {
    return MapBool(jsAbstractRobot_.call<emscripten::val>("getKarboniteMap"));
  }

  /**
   * Returns {@link fuel_map}.
   */
  MapBool getFuelMap() const {
    return MapBool(jsAbstractRobot_.call<emscripten::val>("getFuelMap"));
  }

  /**
   * Returns {@link GameState.visible}.
   */
  emscripten::val getVisibleRobots() const {
    // TODO: this may also be worth caching, or at least converting to typed instances
    return jsAbstractRobot_["_bc_game_state"]["visible"];
  }

};
}
#endif //CPP_STARTER_CPP_STARTER_H
