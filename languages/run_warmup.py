"""How much slower is each language's first replay than its fifth?

Run in this folder after run.py (it builds the 6 programs and ops.bin):
    python3 run_warmup.py [starts]
Starts every program fresh STARTS times (default 20), each replaying ops.bin 5
times into a new book, and compares the first replay with the fifth.
Score: median over all starts of first replay seconds / fifth replay seconds.
Writes results_warmup.log.
"""
import statistics
import sys

from race import machine, save, sh

STARTS = int(sys.argv[1]) if len(sys.argv) > 1 else 20
LANGUAGES = {
    "C++": "./orderbook_cpp ops.bin 5",
    "Rust": "./orderbook_rust ops.bin 5",
    "Go": "./orderbook_go ops.bin 5",
    "Java": "java OrderBook ops.bin 5",
    "Node": "node orderbook.js ops.bin 5",
    "Python": "python3 orderbook.py ops.bin 5",
}


def main():
    log = machine()
    ratios = {name: [] for name in LANGUAGES}
    firsts = {name: [] for name in LANGUAGES}
    fifths = {name: [] for name in LANGUAGES}
    for s in range(1, STARTS + 1):
        for name, cmd in LANGUAGES.items():
            line = sh(cmd)
            log.append(f"start {s}: {name}: {line}")
            runs = [float(x) for x in line.split(",")[6:]]
            firsts[name].append(runs[0])
            fifths[name].append(runs[4])
            ratios[name].append(runs[0] / runs[4])
    log.append("# " + sh("date -u +%Y-%m-%dT%H:%M:%SZ"))
    log.append("language,median_first_s,median_fifth_s,median_first_over_fifth,min_ratio,max_ratio")
    for name in sorted(ratios, key=lambda n: statistics.median(ratios[n])):
        r = ratios[name]
        log.append(
            f"{name},{statistics.median(firsts[name]):.4f},{statistics.median(fifths[name]):.4f},"
            f"{statistics.median(r):.2f},{min(r):.2f},{max(r):.2f}"
        )
    save("results_warmup.log", log)


if __name__ == "__main__":
    main()
