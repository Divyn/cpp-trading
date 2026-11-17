"""Summarize raw_<region>.csv: median and p99 per client and region, network and code apart.
The first request of each 51 request turn opens the connection and is reported on its own."""
import csv
import glob
import statistics

NAMES = {"nrt": "Tokyo", "sgp": "Singapore", "bom": "Mumbai", "ewr": "New York"}


def pct(xs, q):
    xs = sorted(xs)
    return xs[min(len(xs) - 1, int(q * len(xs)))]


print("region,client,requests,errors,network_median_ms,code_median_ms,total_median_ms,total_p99_ms,first_request_median_ms")
for path in sorted(glob.glob("raw_*.csv"), key=lambda p: ["nrt", "sgp", "bom", "ewr"].index(p[4:7])):
    region = path[4:7]
    rows = [r for r in csv.reader(l for l in open(path) if not l.startswith("#")) if r and r[0] in ("py", "cpp")]
    for client in ("py", "cpp"):
        ok = [r for r in rows if r[0] == client and r[2] != "error"]
        errors = sum(1 for r in rows if r[0] == client and r[2] == "error")
        warm = [r for r in ok if r[1] != "0"]
        first = [float(r[2]) + float(r[3]) for r in ok if r[1] == "0"]
        net = [float(r[2]) for r in warm]
        code = [float(r[3]) for r in warm]
        total = [a + b for a, b in zip(net, code)]
        print(f"{NAMES[region]},{'Python' if client == 'py' else 'C++'},{len(warm)},{errors},"
              f"{statistics.median(net):.2f},{statistics.median(code):.3f},{statistics.median(total):.2f},"
              f"{pct(total, 0.99):.2f},{statistics.median(first):.1f}")
