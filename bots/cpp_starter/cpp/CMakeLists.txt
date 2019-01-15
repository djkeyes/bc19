cmake_minimum_required(VERSION 3.9...3.13)

if (${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif ()

project(cpp_starter VERSION 0.1
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# TODO: make this an option
set(MEMORY_LIMIT 67108864)
# TODO: make this an option
set(ARTIFACT_NAME wasm_lib_GENERATED)

set(SRC_FILES
        cpp_starter.cpp
        cpp_starter.h
        my_robot.cpp
        fast_rand.h
        UnitCounts.h
        CastleTalker.h
        CommonRobot.h
        CastleRobot.cpp
        CastleRobot.h
        PilgrimRobot.cpp
        PilgrimRobot.h
        DeterministicResourceClusterer.cpp
        DeterministicResourceClusterer.h
        Coordinate.h
        Pathfinder.cpp
        Pathfinder.h)

# this must be shared, because em++ magic is applied during linking.
add_library(${ARTIFACT_NAME} SHARED ${SRC_FILES})

# Flags listed here are concatenated into a list and passed to the emscripten linker.
# Emscripten flags starting with -s must listed here, since they contain whitespace.
set(EMSCRIPTEN_SETTINGS
        "-s WASM"
        "--bind" # for embind
        "-s EXPORT_ES6" # ES6-style imports
        "-s MODULARIZE" # required for EXPORT_ES6
        "-s BINARYEN_ASYNC_COMPILATION=0" # disable async module loading, since BC19 is synchronous
        # consider "-s ALLOW_MEMORY_GROWTH=1" if you're getting OOM, and not sure how much to allocate
        "-s TOTAL_MEMORY=${MEMORY_LIMIT}"
        # disable as much as possible
        "-s FILESYSTEM=0"
        "-s NO_EXIT_RUNTIME=1"
        "-s EXPORTED_FUNCTIONS='[]'"
        "-s EXTRA_EXPORTED_RUNTIME_METHODS='[]'"
        "-s ENVIRONMENT=shell" # can't specify vanilla JS, but 'shell' generates the least boilerplate
        )

set(EMSCRIPTEN_SETTINGS_STRING "")
foreach (SINGLE_SETTING ${EMSCRIPTEN_SETTINGS})
    set(EMSCRIPTEN_SETTINGS_STRING "${EMSCRIPTEN_SETTINGS_STRING} ${SINGLE_SETTING}")
endforeach ()
message("settings: ${EMSCRIPTEN_SETTINGS_STRING}")
set_target_properties(${ARTIFACT_NAME} PROPERTIES LINK_FLAGS "${EMSCRIPTEN_SETTINGS_STRING}")

# just disable exceptions
target_compile_options(${ARTIFACT_NAME} PUBLIC -fno-exceptions)
# TODO: Disable exceptions and rtti. This breaks some features of embind, so we need to fix those.
## disable exceptions and rtti
#target_compile_definitions(${ARTIFACT_NAME} PUBLIC -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0)
#target_compile_options(${ARTIFACT_NAME} PUBLIC -fno-exceptions -fno-rtti)

# make the format "X.js" instead of "libX.so"
set_target_properties(${ARTIFACT_NAME} PROPERTIES PREFIX "")
set_target_properties(${ARTIFACT_NAME} PROPERTIES SUFFIX ".js")

# TODO this copies the final js file to the general js directory, requiring this to be an in-source build. Clean up?
#add_custom_command(TARGET ${ARTIFACT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${ARTIFACT_NAME}> ../../${ARTIFACT_NAME}.js
#)