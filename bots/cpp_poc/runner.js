

//import Module from './foo.js'
Module = require('./foo').module

let bindings = null;
Module().then(function (Module) {
    bindings = Module;
});

Module.onRuntimeInitialized = async _ => {
    console.log('lerp result: ' + Module.lerp(1, 2, 0.5));

    for (let i=0; i <= 5; i++) {
        console.log('sqrt result: ' + Module.int_sqrt(i))
    }
    for (let i=0; i <= 5; i++) {
        console.log('square result: ' + Module.square(i))
    }

    let ptr = Module.allocate();
    let ptr2 = Module.allocate();
    console.log("got pointer: " + ptr);
    console.log("got pointer: " + ptr2);
    Module.deallocate(ptr);
    Module.deallocate(ptr2);

    Module.doStuffWithVector();

};


/*import {BCAbstractRobot, SPECS} from 'battlecode';

// do nothing
class MyRobot extends BCAbstractRobot {
    turn() {
        var result = Module.ccall('meaning', // name of C function
            number, // return type
            null, // argument types
            null); // arguments
        this.log(result);
    }
}

var robot = new MyRobot();*/
