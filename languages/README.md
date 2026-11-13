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
