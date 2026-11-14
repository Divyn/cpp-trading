"""Race 8 compiler settings on the C++ order book.

Run in this folder after run.py has made ops.bin: python3 run_compilers.py [passes]
The profile-guided build trains on a different set of orders (seed 7), so it is
not tuned to the exact orders it is timed on. Writes results_compilers.log.
"""
import sys

from race import machine, race, save, sh

PASSES = int(sys.argv[1]) if len(sys.argv) > 1 else 3
RUNS = 5

BUILDS = {
    "g++ -O0": "g++ -O0 -std=c++17 orderbook.cpp -o ob_gcc_O0",
    "g++ -O1": "g++ -O1 -std=c++17 orderbook.cpp -o ob_gcc_O1",
    "g++ -O2": "g++ -O2 -std=c++17 orderbook.cpp -o ob_gcc_O2",
    "g++ -O3": "g++ -O3 -std=c++17 orderbook.cpp -o ob_gcc_O3",
    "g++ -O3 -march=native": "g++ -O3 -march=native -std=c++17 orderbook.cpp -o ob_gcc_O3_native",
    "g++ -O3 + profile": (
        "rm -f *.gcda && g++ -O3 -fprofile-generate -std=c++17 orderbook.cpp -o ob_gcc_pgo_train"
        " && ./gen_ops 1000000 7 > train.bin && ./ob_gcc_pgo_train train.bin 1 > /dev/null"
        " && g++ -O3 -fprofile-use -fprofile-correction -std=c++17 orderbook.cpp -o ob_gcc_O3_pgo"
    ),
    "clang -O2": "clang++ -O2 -std=c++17 orderbook.cpp -o ob_clang_O2",
    "clang -O3": "clang++ -O3 -std=c++17 orderbook.cpp -o ob_clang_O3",
}
BINARIES = {
    "g++ -O0": "ob_gcc_O0", "g++ -O1": "ob_gcc_O1", "g++ -O2": "ob_gcc_O2", "g++ -O3": "ob_gcc_O3",
    "g++ -O3 -march=native": "ob_gcc_O3_native", "g++ -O3 + profile": "ob_gcc_O3_pgo",
    "clang -O2": "ob_clang_O2", "clang -O3": "ob_clang_O3",
}


def main():
    sh("g++ -O2 -std=c++17 gen_ops.cpp -o gen_ops")
    for cmd in BUILDS.values():
        sh(cmd)
    log = ["# " + sh("g++ --version | head -1"), "# " + sh("clang++ --version | head -1")] + machine()
    race({name: f"./{b} ops.bin {RUNS}" for name, b in BINARIES.items()}, PASSES, log)
    save("results_compilers.log", log)


if __name__ == "__main__":
    main()
