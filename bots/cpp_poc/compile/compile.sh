
# this has to be in a different directory from the source to prevent bc19run from failing
# TODO(daniel): convert this to a CMakeLists.txt
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

em++ -std=c++14 -o $CURRENT_DIR/../my_lib_GENERATED.js $CURRENT_DIR/../my_lib.cpp -O3 -s WASM=1 -s NO_EXIT_RUNTIME=1  -s EXPORTED_FUNCTIONS='[]' -s EXTRA_EXPORTED_RUNTIME_METHODS='[]' --bind -s SINGLE_FILE -s EXPORT_ES6 -s MODULARIZE -s ENVIRONMENT_MAY_BE_WEB=0 -s ENVIRONMENT_MAY_BE_WORKER=0 -s ENVIRONMENT_MAY_BE_NODE=0 -s ENVIRONMENT_MAY_BE_SHELL=0 -s ENVIRONMENT_MAY_BE_WEB_OR_WORKER=0 -s TOTAL_MEMORY=67108864 -s BINARYEN_ASYNC_COMPILATION=0

