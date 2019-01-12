cmake_minimum_required(VERSION 3.9...3.13)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(cpp_starter VERSION 0.1
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# TODO: make this an option
set(MEMORY_LIMIT 67108864)
# TODO: make this an option
set(ARTIFACT_NAME wasm_lib_GENERATED)

set(SRC_FILES
        cpp_starter.cpp)

# this must be shared, because em++ magic is applied during linking.
add_library(${ARTIFACT_NAME} SHARED ${SRC_FILES})

# emscripten flags starting with -s must listed here, since they contain whitespace
set_target_properties(${ARTIFACT_NAME} PROPERTIES LINK_FLAGS "-s WASM=1 -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS='[]' -s EXTRA_EXPORTED_RUNTIME_METHODS='[]' --bind -s SINGLE_FILE -s EXPORT_ES6 -s MODULARIZE -s ENVIRONMENT_MAY_BE_WEB=0 -s ENVIRONMENT_MAY_BE_WORKER=0 -s ENVIRONMENT_MAY_BE_NODE=0 -s ENVIRONMENT_MAY_BE_SHELL=0 -s ENVIRONMENT_MAY_BE_WEB_OR_WORKER=0 -s BINARYEN_ASYNC_COMPILATION=0 -s TOTAL_MEMORY=${MEMORY_LIMIT}")

# make the format "X.js" instead of "libX.so"
set_target_properties(${ARTIFACT_NAME} PROPERTIES PREFIX "")
set_target_properties(${ARTIFACT_NAME} PROPERTIES SUFFIX ".js")

# TODO this copies the final js file to the general js directory, requiring this to be an in-source build. Clean up?
#add_custom_command(TARGET ${ARTIFACT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${ARTIFACT_NAME}> ../../${ARTIFACT_NAME}.js
#)