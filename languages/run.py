"""Build all 6 order books, replay ops.bin in each, check the trades match, print the score.

Run in this folder: python3 run.py [passes]
Writes results.log (every raw line plus the summary).
"""
import statistics
import subprocess
import sys

PASSES = int(sys.argv[1]) if len(sys.argv) > 1 else 3
RUNS = 5

BUILD = [
    "g++ -O2 -std=c++17 gen_ops.cpp -o gen_ops",
    "./gen_ops 1000000 > ops.bin",
    "g++ -O2 -std=c++17 orderbook.cpp -o orderbook_cpp",
    "rustc -C opt-level=3 orderbook.rs -o orderbook_rust",
    "go build -o orderbook_go orderbook.go",
    "javac -d . OrderBook.java",
]
VERSIONS = ["g++ --version | head -1", "rustc --version", "go version", "java -version 2>&1 | head -1", "node --version", "python3 --version"]
LANGUAGES = {
    "cpp": f"./orderbook_cpp ops.bin {RUNS}",
    "rust": f"./orderbook_rust ops.bin {RUNS}",
    "go": f"./orderbook_go ops.bin {RUNS}",
    "java": f"java OrderBook ops.bin {RUNS}",
    "node": f"node orderbook.js ops.bin {RUNS}",
    "python": f"python3 orderbook.py ops.bin {RUNS}",
}


def sh(cmd):
    return subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True).stdout.strip()


def main():
    log = []
    for cmd in BUILD:
        sh(cmd)
    for cmd in VERSIONS:
        log.append("# " + sh(cmd))
    log.append("# " + sh("nproc") + " vCPU, " + sh("free -m | awk '/Mem:/{print $2}'") + " MB RAM")
    log.append("# " + sh("date -u +%Y-%m-%dT%H:%M:%SZ"))

    count = 1000000
    pass_medians = {name: [] for name in LANGUAGES}
    totals = {}
    for p in range(1, PASSES + 1):
        for name, cmd in LANGUAGES.items():
            line = sh(cmd)
            log.append(f"pass {p}: {line}")
            parts = line.split(",")
            totals.setdefault(name, set()).add(tuple(parts[1:6]))
            pass_medians[name].append(statistics.median(float(s) for s in parts[6:]))
    log.append("# " + sh("date -u +%Y-%m-%dT%H:%M:%SZ"))

    reference = totals["cpp"]
    log.append("language,median_s,million_ops_per_s,pass_medians_s,fills,traded,cancelled,best_bid,best_ask,check")
    for name, medians in sorted(pass_medians.items(), key=lambda kv: statistics.median(kv[1]), reverse=True):
        median = statistics.median(medians)
        same = totals[name] == reference and len(reference) == 1
        log.append(
            f"{name},{median:.4f},{count / median / 1e6:.3f},"
            f"{';'.join(f'{m:.4f}' for m in medians)},{','.join(next(iter(totals[name])))},"
            f"{'match' if same else 'MISMATCH'}"
        )
    text = "\n".join(log) + "\n"
    open("results.log", "w").write(text)
    print(text, end="")


if __name__ == "__main__":
    main()
