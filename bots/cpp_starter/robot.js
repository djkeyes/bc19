import {BCAbstractRobot, SPECS} from 'battlecode';

import Module from './wasm_lib_GENERATED.js'

let bindings = null;

let nativeRobot = null;

class MyRobot extends BCAbstractRobot {
    turn() {
        if (!nativeRobot) {
            Module().then(function (Module) {
                bindings = Module;
            });
            nativeRobot = bindings.AbstractNativeRobot.createNativeRobotImpl(this);
        }
        return nativeRobot.turn();
    }

}
var robot = new MyRobot();

