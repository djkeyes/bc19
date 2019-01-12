import {BCAbstractRobot, SPECS} from 'battlecode';

import Module from './my_lib_GENERATED.js'

let bindings = null;
Module().then(function (Module) {
    bindings = Module;
});

let num_turns = 0;
let total_cpp_millis = 0;
let total_js_millis = 0;

let num_turns_since_startup = 0;
const startup_time = new Date();


// do nothing
class MyRobot extends BCAbstractRobot {
    turn() {
        const time_at_turn = new Date();

        const sleepDuration = 5;
        var now = new Date().getTime();
        while(new Date().getTime() < now + sleepDuration){ /* do nothing */ }

        this.log('started turn');
        if (bindings) {
            const num_turns_at_binding = num_turns_since_startup;
            const time_at_load = new Date();
            const time_to_load_js = time_at_turn - startup_time;
            const time_to_load_cpp = time_at_load - startup_time;
            this.log('time to load js: ' + time_to_load_js);
            this.log('time to load cpp: ' + time_to_load_cpp);
            this.log('turn when binding completed: ' + num_turns_at_binding);

            num_turns++;
            this.log('map size: ' + this.map.length + ' row x ' + this.map[0].length + ' cols');

            if (false) {
                var instance = new bindings.MyRobot(10, "hello");
                instance.incrementX();
                this.log(instance.x); // 11
                instance.x = 20; // 20
                this.log(bindings.MyRobot.getStringFromInstance(instance)); // "hello"


                // let map = [[1,0,1],[0,0,0],[1,1,0]];
                // let map = instance.make_grid();
                // let map = instance.make_grid(this.map[0].length, this.map.length)
                let map = new bindings.GridChar(this.map[0].length, this.map.length);
                for (let row = 0; row < this.map.length; ++row) {
                    for (let col = 0; col < this.map[0].length; ++col) {
                        const val = this.map[row][col] ? 1 : 0;
                        map.set(row, col, val);
                    }
                }
                // for (let row = 0; row < this.map.length; ++row) {
                //     let str = '';
                //     for (let col = 0; col < this.map[0].length; ++col) {
                //         str += map.get(row, col);
                //     }
                //     this.log(str);
                // }
                // for (let row = 0; row < this.map.length; ++row) {
                //     let str = '';
                //     for (let col = 0; col < this.map[0].length; ++col) {
                //         const val = this.map[row][col] ? 1 : 0;
                //         str += val;
                //     }
                //     this.log(str);
                // }

                let startTime = new Date();
                let cpp_result = instance.all_pairs_shortest_path(map);
                let endTime = new Date();
                let timeDiff = endTime - startTime; //in ms
                this.log('time for all pairs shortest path in cpp: ' + timeDiff + "ms");
                total_cpp_millis += timeDiff;
                this.log('avg for all pairs shortest path in cpp: ' + total_cpp_millis / num_turns + "ms");

                startTime = new Date();
                let js_result = this.all_pairs_shortest_path(this.map);
                endTime = new Date();
                timeDiff = endTime - startTime; //in ms
                this.log('time for all pairs shortest path in js: ' + timeDiff + "ms");
                total_js_millis += timeDiff;
                this.log('avg for all pairs shortest path in js: ' + total_js_millis / num_turns + "ms");
                this.log('cpp debug result: ' + cpp_result);
                this.log('js debug result: ' + js_result);

                instance.delete();
            }

            if (false) {
                this.log('lerp result: ' + bindings.lerp(1, 2, 0.5));

                for (let i = 0; i <= 5; i++) {
                    this.log('sqrt result: ' + bindings.int_sqrt(i))
                }
                for (let i = 0; i <= 5; i++) {
                    this.log('square result: ' + bindings.square(i))
                }

                let ptr = bindings.allocate();
                let ptr2 = bindings.allocate();
                this.log("got pointer: " + ptr);
                this.log("got pointer: " + ptr2);
                bindings.deallocate(ptr);
                bindings.deallocate(ptr2);

                bindings.doStuffWithVector();
            }
        } else {
            this.log('not yet initialized');
        }

        num_turns_since_startup++;
    }

    all_pairs_shortest_path() {
        const rows = this.map.length;
        const cols = this.map[0].length;
        const dirs = [[-1, 0], [0, -1], [1, 0], [0, 1]];
        let dist = new Array(rows);
        for (let start_r = 0; start_r < rows; start_r++) {
            dist[start_r] = new Array(cols);
            for (let start_c = 0; start_c < cols; start_c++) {
                dist[start_r][start_c] = new Array(rows);
                for (let r = 0; r < rows; r++) {
                    dist[start_r][start_c][r] = new Array(cols);
                    for (let c = 0; c < cols; c++) {
                        dist[start_r][start_c][r][c] = -1;
                    }
                }

                if (!this.map[start_r][start_c]) {
                    continue;
                }
                // for dubious efficiency, implement our own queue
                const queue = new CircularQueue(2 * (rows + cols + 5));
                queue.push([start_r, start_c]);
                dist[start_r][start_c][start_r][start_c] = 0;
                while (queue.size > 0) {
                    const cur = queue.pop();
                    const cur_r = cur[0];
                    const cur_c = cur[1];
                    const next_dist = dist[start_r][start_c][cur_r][cur_c] + 1;
                    for (let d = 0; d < dirs.length; d++) {
                        const next_r = cur_r + dirs[d][0];
                        const next_c = cur_c + dirs[d][1];
                        if (next_r >= 0
                            && next_c >= 0
                            && next_r < rows
                            && next_c < rows
                            && this.map[next_r][next_c]) {
                            if (dist[start_r][start_c][next_r][next_c] === -1) {
                                dist[start_r][start_c][next_r][next_c] = next_dist;
                                queue.push([next_r, next_c]);
                            }
                        }
                    }
                }
            }
        }

        let longest_dist = -1;
        for (let r = 0; r < rows; r++) {
            for (let c = 0; c < cols; c++) {
                for (let r1 = 0; r1 < rows; r1++) {
                    for (let c1 = 0; c1 < cols; c1++) {
                        longest_dist = Math.max(longest_dist, dist[r][c][r1][c1]);
                    }
                }

            }
        }
        return longest_dist;
    }
}

// circular queue, with no bounds checks.
class CircularQueue {
    constructor(max_size) {
        this.data = new Array(max_size);
        this.max_size = max_size;
        this.head = 0;
        this.tail = 0;
        this.size = 0;
    }

    push(element) {
        this.data[this.tail++] = element;
        this.tail %= this.max_size;
        this.size++;
        if (this.size > this.max_size) {
            throw Error('too big');
        }
    }

    pop() {
        const result = this.data[this.head++];
        this.head %= this.max_size;
        this.size--;
        if (this.size < 0) {
            throw Error('too small');
        }
        return result;
    }

}

var robot = new MyRobot();
