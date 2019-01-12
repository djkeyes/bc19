import {BCAbstractRobot, SPECS} from 'battlecode';

class MyRobot extends BCAbstractRobot {
    
    constructor() {
        super();
        this.impl = null;
    }
    
    turn() {
        if (this.impl == null) {
            this.impl = this.createImpl();
        }
        return this.impl.turn();
    }
    
    createImpl() {
        switch (this.me.unit) {
            case SPECS.CASTLE:
                return new CastleRobot(this);
            case SPECS.PILGRIM:
                return new PilgrimRobot(this);
            case SPECS.CRUSADER:
                return new CrusaderRobot(this);
        }
        return new NoopRobot();
    }
}

class CommonRobot {
    constructor(robotPlayer) {
        this.rob = robotPlayer;
        this.isFirstTurn = true;
    }
    
    turn() {
        if (this.isFirstTurn) {
            this.isFirstTurn = false;
            this.init();
        }
        return this.turnImpl();
    }
    
    turnImpl() {
        return null;
    }
    init() {
    }
    
    moveToward(destinationX, destinationY) {
        // bad path planning impl
        if (this.rob.fuel < SPECS.UNITS[this.rob.me.unit].FUEL_PER_MOVE) {
            // can't even make one move
            return null;
        }
        
        // for now, only do adjacent moves, because that's cheapest
        // chose the movement that gets us closest, even if it means going backwards
        let closestDir = -1;
        let closestDistSq = -1;
        const choices = [[0,-1], [1, -1], [1, 0], [1, 1], [0, 1], [-1, 1], [-1, 0], [-1, -1]];
        for (let dir = 0; dir < choices.length; ++dir) {
            const dx = choices[dir][1];
            const dy = choices[dir][0];
            const curx = this.rob.me.x;
            const cury = this.rob.me.y;
            const targetx = curx + dx;
            const targety = cury + dy;

            if (!this.rob._bc_check_on_map(targetx, targety)) {
                continue;
            }
            if (this.rob.getVisibleRobotMap()[targety][targetx] !== 0) {
                continue;
            }
            if (!this.rob.map[targety][targetx]) {
                continue;
            }
            const rSq = dx * dx + dy * dy;
            if (this.rob.fuel < rSq*SPECS.UNITS[this.rob.me.unit].FUEL_PER_MOVE) {
                continue;
            }

            const remainingx = destinationX - targetx;
            const remainingy = destinationY - targety;
            const distanceSq = remainingx * remainingx + remainingy * remainingy;
            if (closestDistSq === -1 || distanceSq < closestDistSq) {
                closestDistSq = distanceSq;
                closestDir = dir;
            }
            
        }
        if (closestDistSq !== -1) {
            const dx = choices[closestDir][1];
            const dy = choices[closestDir][0];
            return this.rob.move(dx, dy);
        }
        return null;
    }
    
}
class CastleRobot extends CommonRobot {
    turnImpl() {
        if (this.numPilgrimsBuilt < 3) {
            // try building a pilgrim
            const action = this.tryBuilding(SPECS.PILGRIM);
            if (action !== null) {
                this.numPilgrimsBuilt++;
                return action;
            }
        } else {
            // try building a crusader
            return this.tryBuilding(SPECS.CRUSADER);
        }
    }
    
    init() {
        this.numPilgrimsBuilt = 0;
    }
    
    tryBuilding(unitType) {
        if (this.rob.karbonite < SPECS.UNITS[unitType].CONSTRUCTION_KARBONITE
                || this.rob.fuel < SPECS.UNITS[unitType].CONSTRUCTION_FUEL) {
            return null;
        }

        const choices = [[0,-1], [1, -1], [1, 0], [1, 1], [0, 1], [-1, 1], [-1, 0], [-1, -1]];
        for (let dir = 0; dir < choices.length; ++dir) {
            const dx = choices[dir][1];
            const dy = choices[dir][0];
            const curx = this.rob.me.x;
            const cury = this.rob.me.y;
            const targetx = curx + dx;
            const targety = cury + dy;
            if (!this.rob._bc_check_on_map(targetx, targety)) {
                continue;
            }
            if (this.rob.getVisibleRobotMap()[targety][targetx] === 0
                    && this.rob.getPassableMap()[targety][targetx]) {
                return this.rob.buildUnit(unitType, dx, dy);
            }
        }
        return null;
    }
}

class PilgrimRobot extends CommonRobot {
    init() {
        this.currentTarget = null;
        this.startingLoc = [this.rob.me.x, this.rob.me.y];
        this.karboniteList = this.makeMineList(this.rob.getKarboniteMap())
        this.fuelList = this.makeMineList(this.rob.getFuelMap())
        this.gathering = true;
    }
    turnImpl() {
        if (this.currentTarget === null) {    
            // if we have no goal, choose a goal
            if (this.rob.me.karbonite > 0 || this.rob.me.fuel > 0) {
                // if we are carrying resources, return to base
                this.currentTarget = this.startingLoc;
                this.gathering = false;
            } else {
                // if we are empty, find a resource deposit
                if (this.preferKarboniteOverFuel()) {
                    this.rob.log("seeking karbonite");
                    this.currentTarget = this.findNearestMine(this.karboniteList);
                } else {
                    this.rob.log("seeking fuel");
                    this.currentTarget = this.findNearestMine(this.fuelList);
                }
                this.rob.log("nearest mine is at " + this.currentTarget);
                this.gathering = true;
            }
        }
        
        if (this.gathering) {
            const isFull = this.rob.me.fuel >= SPECS.UNITS[SPECS.PILGRIM].FUEL_CAPACITY
                || this.rob.me.karbonite >= SPECS.UNITS[SPECS.PILGRIM].KARBONITE_CAPACITY;
            // already at a mine?
            if (this.currentTarget[0] === this.rob.me.x && this.currentTarget[1] == this.rob.me.y) {
                if (isFull) {
                    // full? change directions
                    this.currentTarget = this.startingLoc;
                    this.gathering = false;
                    this.rob.log("full, returning to " + this.currentTarget);
                } else {
                    if (this.rob.fuel < SPECS.MINE_FUEL_COST) {
                        return null;
                    }
                    this.rob.log("reached the mine at " + this.currentTarget + ", now mining");
                    return this.rob.mine();
                }
            }
        }
        if (!this.gathering) {
            // check for visible depots to return to
            const rp = this.rob;
            const depots = this.rob.getVisibleRobots().filter(function (r) {
                return r.team === rp.me.team && (r.unit === SPECS.CASTLE || r.unit === SPECS.CHURCH);
            });
            let closestDepotL1 = null;
            let closestL1Dist = -1;
            let closestDepotL2 = null;
            let closestSqDist = -1;
            for (let i=0; i < depots.length; ++i) {
                const dx = depots[i].x - this.rob.me.x;
                const dy = depots[i].y - this.rob.me.y;
                const l1Dist = Math.abs(dx) + Math.abs(dy);
                if (closestL1Dist == -1 || l1Dist < closestL1Dist) {
                    closestL1Dist = l1Dist;
                    closestDepotL1 = [depots[i].x, depots[i].y];
                }
                const distSq = dx * dx + dy * dy;
                if (closestSqDist == -1 || distSq < closestSqDist) {
                    closestSqDist = distSq;
                    closestDepotL2 = [depots[i].x, depots[i].y];
                }
            }
        
            if (closestSqDist <= 2) {
                // arrive? deposit
                const dx = closestDepotL2[0] - this.rob.me.x;
                const dy = closestDepotL2[1] - this.rob.me.y;
                this.currentTarget = null;
                this.gathering = true;
                this.rob.log("currently at (" + this.rob.me.x + ", " + this.rob.me.y + "), which is adjacent to " + closestDepotL2 + ", so depositing.");
                return this.rob.give(dx, dy, this.rob.me.karbonite, this.rob.me.fuel);
            } else {
                this.rob.log("Closest depot in vision range is at " + closestDepotL1);
                this.currentTarget = closestDepotL1;
            }
        
        }
        return this.moveToward(this.currentTarget[0], this.currentTarget[1]);
    }
    
    makeMineList(mineMap) {
        const result = [];
        for (let x = 0; x < mineMap[0].length; ++x) {
            for (let y = 0; y < mineMap.length; ++y) {
                if (mineMap[y][x]) {
                    result.push([x, y]);
                }
            }
        }
        return result;
    }
    
    preferKarboniteOverFuel() {
        // seems that karbonite is rare, fuel is plentiful
        return 10*this.rob.karbonite < this.rob.fuel;
    }
    
    findNearestMine(mineList) {
        // using slow walking, distance are equivilant to manhattan

        let closestMine = null;
        let closestL1Dist = -1;
        const curx = this.rob.me.x;
        const cury = this.rob.me.y;
        for (let i = 0; i < mineList.length; ++i) {
            const targetx = mineList[i][0];
            const targety = mineList[i][1];
            const dx = targetx - curx;
            const dy = targety - cury;
            const l1Dist = Math.abs(dx) + Math.abs(dy);

            if (closestL1Dist === -1 || l1Dist < closestL1Dist) {
                closestL1Dist = l1Dist;
                closestMine = mineList[i];
            }
            
        }
        return closestMine;
    }
}

class CrusaderRobot extends CommonRobot {
    init() {
        this.currentTarget = null;
        this.turnsAlive = 0;
    }
    turnImpl() {
        if (this.turnsAlive % 30 === 0) {
            // chose a random destination on the map
            const height = this.rob.map.length;
            const width = this.rob.map[0].length;
            const y = Math.floor(Math.random() * height);
            const x = Math.floor(Math.random() * width);
            this.currentTarget = [x, y];
        }
        this.turnsAlive++;

        const visible = this.rob.getVisibleRobots();

        const rp = this.rob;

        // get attackable robots
        const attackable = visible.filter(function (r) {
            if (!rp.isVisible(r)) {
                return false;
            }
            const dx = r.x - rp.me.x;
            const dy = r.y - rp.me.y;
            const dist = dx * dx + dy * dy;
            return r.team !== rp.me.team
                && SPECS.UNITS[SPECS.CRUSADER].ATTACK_RADIUS[0] <= dist
                && dist <= SPECS.UNITS[SPECS.CRUSADER].ATTACK_RADIUS[1];

        });

        if (attackable.length>0 && this.rob.fuel >= SPECS.UNITS[this.rob.me.unit].ATTACK_FUEL_COST){
            // attack first robot
            const r = attackable[0];
            return this.rob.attack(r.x - this.rob.me.x, r.y - this.rob.me.y)
        }
        
        return this.moveToward(this.currentTarget[0], this.currentTarget[1]);
    }
}

class NoopRobot extends CommonRobot { }

const robot = new MyRobot();

