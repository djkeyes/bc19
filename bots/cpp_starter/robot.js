import {BCAbstractRobot} from 'battlecode';

import Module from './wasm_lib_GENERATED.js'

// Two slow parts of Emscripten are loading the binary data and instantiating
// WebAssembly module. We can do these two steps ahead of time, since they are
// fast enough to run during initialization.
import {wasmLoader} from './wasm_loader_GENERATED.js';

let ModuleWithCallbacks = {};
let module = new WebAssembly.Module(wasmLoader());
ModuleWithCallbacks["instantiateWasm"] = function (info, receiveInstanceCallback) {
    let instance = new WebAssembly.Instance(module, info)
    receiveInstanceCallback(instance, module);
    return instance.exports;
};

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
