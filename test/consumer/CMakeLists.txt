cmake_minimum_required(VERSION 3.21)
project(tts_cpp_consumer LANGUAGES C CXX)

# Smoke test for an installed tts-cpp package (.github/workflows/release.yml),
# used the way Basu's Read Aloud engine uses it: find_package(tts-cpp), then a
# shared library with tts-cpp and ggml linked statically inside, and a program
# that loads it.
#
#   cmake -S test/consumer -B build-consumer -DCMAKE_PREFIX_PATH=<package>
#         [-DTTS_PROBE_REQUIRE=Vulkan|MTL]
#   ctest --test-dir build-consumer --output-on-failure
#
# TTS_PROBE_REQUIRE fails the test unless ggml registered that backend, which
# it does only where the runner can open it. With model files, the program
# also reads a sentence:
#   tts_probe_run [--require <backend>] <t3.gguf> <s3gen.gguf> ["text"]

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TTS_PROBE_REQUIRE "" CACHE STRING "ggml backend the probe must find registered (empty: none)")

find_package(tts-cpp 0.1 CONFIG REQUIRED)

add_library(tts_probe SHARED probe.cpp)
target_link_libraries(tts_probe PRIVATE tts-cpp::tts-cpp)
set_target_properties(tts_probe PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Every symbol resolved at link time (so the package's link interface is
    # complete), the archives' objects position-independent (or this link
    # fails), the C++ runtime inside, and nothing of tts-cpp or ggml
    # re-exported.
    target_link_options(tts_probe PRIVATE
        -Wl,-z,defs -Wl,--exclude-libs,ALL -static-libstdc++ -static-libgcc)
endif()

add_executable(tts_probe_run main.cpp)
target_link_libraries(tts_probe_run PRIVATE tts_probe)

enable_testing()
if (TTS_PROBE_REQUIRE)
    add_test(NAME probe COMMAND tts_probe_run --require ${TTS_PROBE_REQUIRE})
else()
    add_test(NAME probe COMMAND tts_probe_run)
endif()
