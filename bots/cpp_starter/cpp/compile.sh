#!/usr/bin/env bash

# this has to be in a different directory from the source to prevent bc19run from failing

set -e

current_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

build_dir=${current_dir}/build
# if you have cmake caching problems, it may help to remove build dir
#[ -e ${build_dir}/ ] && rm -r ${build_dir}

[ -e ${build_dir}/ ] || mkdir ${build_dir}

pushd ${build_dir}

export CXX=em++
export CC=emcc

#cmake -DCMAKE_BUILD_TYPE=Debug ..
#cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
#cmake -DCMAKE_BUILD_TYPE=Release ..
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
make -j4 VERBOSE=1

popd > /dev/null # build

cp ${build_dir}/wasm_lib_GENERATED.js ${current_dir}/../
# Encode the .wasm file ourself, because Emscripten's version is slow & can't be eagerly pre-loaded.
node ${current_dir}/wasm2js.js ${build_dir}/wasm_lib_GENERATED.wasm ${current_dir}/../wasm_loader_GENERATED.js
