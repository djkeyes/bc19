import {BCAbstractRobot, SPECS} from 'battlecode';

const doPrecomputation = false;

let precomputedData = null;
function doLongAndComplicatedPrecomputation() {
    // just prepend a bunch of numbers to a list
    // you can try tuning this number to get different timeouts
    const loopCount = 20000;
    precomputedData = [];
    for (let i=0; i < loopCount; i++) {
        precomputedData.unshift(i);
    }
}

if (doPrecomputation) {
    // this happens during VM instantiation
    // It's not completely deterministic, but I think this is the outcome:
    // if this takes <SPECS.CHESS_INITIAL ms, robot is alive, and this.me.time is unaffected
    // else if this takes <SPECS.TURN_MAX_TIME, we get error "Took too long to initialize", and "TypeError: robot.hook is not a function"
    // else robot is dead with error "Error: Script execution timed out"
    doLongAndComplicatedPrecomputation();
}

let prevTime = null;
class MyRobot extends BCAbstractRobot {
    turn() {
        if ((precomputedData === null) && !doPrecomputation) {
            // this happens during gameplay -- result: robot gets frozen, but stays alive
            doLongAndComplicatedPrecomputation();
        }
        
        // show some timing data
        let toPrint = 'time at beginning of turn: ' + this.me.time;
        if (prevTime !== null) {
            let elapsed = prevTime - (this.me.time - SPECS.CHESS_EXTRA);
            toPrint += ' (time elapsed last turn: ' + elapsed + ')';
        }
        prevTime = this.me.time;
        this.log(toPrint);
    }
}

var robot = new MyRobot();
