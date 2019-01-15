import {BCAbstractRobot} from 'battlecode';

import Module from './wasm_lib_GENERATED.js'

// One slow part of Wasm loading is simply reading the file into memory.
// Emscripten's base64 string decoder isn't very efficient, so substitute our
// own implementation, and run it during robot startup time.
// TODO: we can furthermore pre-load the Webassembly.Module by implementing "instantiateWasm"
let ModuleWithCallbacks = {};
import {wasmLoader} from './wasm_loader_GENERATED.js';
let wasmBinary = wasmLoader();
ModuleWithCallbacks["wasmBinary"] = wasmBinary;

let bindings = null;
let nativeRobot = null;

class MyRobot extends BCAbstractRobot {
    turn() {
        if (!nativeRobot) {
            // this takes around 60-120ms to initialize
            // Ideally, we would run it in the global scope (which gives us ~100ms of free computation time) but the
            // game has a hard 100ms timeout, after which our bots are killed irretrievably. Therefore, it's safer to
            // load the bindings here.
            // TODO: submit a PR to the engine to freeze the robot instead of killing it.
            let bindingLoader = Module(ModuleWithCallbacks);
            bindingLoader.then(function (Module) {
                bindings = Module;
            });

            nativeRobot = bindings.AbstractNativeRobot.createNativeRobotImpl(this);
        }

        return nativeRobot.turn();
    }

}

var robot = new MyRobot();
