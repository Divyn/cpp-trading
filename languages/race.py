"""Shared runner for the races in this folder.

Each contender is a shell command that replays ops.bin RUNS times and prints
    name,fills,traded,cancelled,best_bid,best_ask,seconds_run1,...
race() runs every contender PASSES times, checks that all of them made the
same trades, and ranks them by the median of the pass medians.
"""
import statistics
import subprocess

COUNT = 1000000


def sh(cmd):
    return subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True).stdout.strip()


def machine():
    return [
        "# " + sh("nproc") + " vCPU, " + sh("free -m | awk '/Mem:/{print $2}'") + " MB RAM, "
        + sh("lscpu | sed -n 's/^Model name: *//p'"),
        "# " + sh("date -u +%Y-%m-%dT%H:%M:%SZ"),
    ]


def race(contenders, passes, log):
    pass_medians = {name: [] for name in contenders}
    totals = {name: set() for name in contenders}
    for p in range(1, passes + 1):
        for name, cmd in contenders.items():
            line = sh(cmd)
            log.append(f"pass {p}: {name}: {line}")
            parts = line.split(",")
            totals[name].add(tuple(parts[1:6]))
            pass_medians[name].append(statistics.median(float(s) for s in parts[6:]))
    log.append("# " + sh("date -u +%Y-%m-%dT%H:%M:%SZ"))

    reference = next(iter(totals.values()))
    log.append("contender,median_s,million_ops_per_s,pass_medians_s,fills,traded,cancelled,best_bid,best_ask,check")
    for name, medians in sorted(pass_medians.items(), key=lambda kv: statistics.median(kv[1]), reverse=True):
        median = statistics.median(medians)
        same = totals[name] == reference and len(reference) == 1
        log.append(
            f"{name},{median:.4f},{COUNT / median / 1e6:.3f},"
            f"{';'.join(f'{m:.4f}' for m in medians)},{','.join(next(iter(totals[name])))},"
            f"{'match' if same else 'MISMATCH'}"
        )


def save(path, log):
    text = "\n".join(log) + "\n"
    open(path, "w").write(text)
    print(text, end="")
