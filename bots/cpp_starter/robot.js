import {BCAbstractRobot, SPECS} from 'battlecode';

import Module from './wasm_lib_GENERATED.js'

let bindings = null;
Module().then(function (Module) {
    bindings = Module;
});

let nativeRobot = null;

class MyRobot extends BCAbstractRobot {
    turn() {
        if (!nativeRobot) {
            nativeRobot = new bindings.NativeRobot(this);
        }
        return nativeRobot.turn();
    }

}
var robot = new MyRobot();

