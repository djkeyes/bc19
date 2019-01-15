#!/usr/bin/env node

let fs = require('fs');

let inputFilename = process.argv[2]
let outputFilename = process.argv[3]

let data = fs.readFileSync(inputFilename);
let asString = JSON.stringify(Array.from(data));

let fileContents = 'export var wasmLoader = function() { return new Uint8Array(' + asString + ') };';

fs.writeFileSync(outputFilename, fileContents);
