# The same order book in 6 languages

Every version replays the same 1,000,000 operations from `ops.bin` and must end
with the same trades. Only the language changes.

The rules, the same in every file:

- Best price first, then oldest order first at that price.
- Each side keeps its price levels in an array sorted by price, best price at
  the end, found with a binary search.
- Each price level is a linked list of orders, oldest first. The links live in
  arrays indexed by order id, so a cancel unlinks its order directly.
- A cancel of an order that is already filled or cancelled does nothing.

Each program loads `ops.bin` first (not timed), then replays it 5 times, each
time into a new empty book, and prints:

    language,fills,traded,cancelled,best_bid,best_ask,seconds_run1,...,seconds_run5

`run.py` builds everything, runs every language 3 times (passes), checks that
all of them made the same trades as C++, and prints the median.

Results are in `results.log` (1 vCPU Vultr server, 2026-11-12). C++ is built
with `-O2` and Rust with `-C opt-level=3` (what `cargo build --release` uses).
A side check after the race, C++ built with `-O3` on the same server, 3 passes
of 5 runs: pass medians 0.0543, 0.0556, 0.0553 s, so 18.08 million operations
per second, against Rust's 18.14.

Build and run (Debian 13):

    apt-get install -y g++ rustc golang-go openjdk-21-jdk-headless nodejs python3
    python3 run.py

## Follow-up races

Same `ops.bin`, same server, same score (million operations per second, median
of 5 runs, 3 passes), and every contender must make the same trades. Run each
script in this folder after `run.py`. `race.py` holds the shared timing code.

| Script | What it races | Results |
| --- | --- | --- |
| `run_compilers.py` | `orderbook.cpp` built 8 ways: g++ -O0 to -O3, -march=native, profile-guided, clang -O2 and -O3 (needs `clang`) | `results_compilers.log` |
| `run_python.py` | `orderbook.py` unchanged under CPython, PyPy, Cython, mypyc and Nuitka | `results_python.log` |
| `run_js.py` | `orderbook.js` unchanged under Node 20, 22, 24 and 26, Bun and Deno | `results_js.log` |
| `run_warmup.py` | first replay vs fifth replay for all 6 languages, 20 fresh starts each | `results_warmup.log` |

`gen_ops.cpp` takes an optional seed (`./gen_ops 1000000 7`). The profile-guided
build trains on seed 7, so it is not tuned to the orders it is timed on.
