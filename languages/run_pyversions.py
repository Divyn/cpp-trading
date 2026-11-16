"""Race the same orderbook.py, unchanged, on CPython 3.11 to 3.15, and 3.15 with its JIT on.

Run in this folder after run.py has made ops.bin: python3 run_pyversions.py <pythons dir> [passes]
Every version is the python-build-standalone build that `uv python install` downloads,
so all six come from the same build recipe. Writes results_pyversions.log.
"""
import glob
import sys

from race import machine, race, save, sh

PYTHONS = sys.argv[1]
PASSES = int(sys.argv[2]) if len(sys.argv) > 2 else 3
RUNS = 5


def python(version):
    return glob.glob(f"{PYTHONS}/cpython-{version}.*-linux-x86_64-gnu/bin/python{version}")[0]


CONTENDERS = {f"CPython {v}": f"PYTHON_JIT=0 {python(v)} orderbook.py ops.bin {RUNS}" for v in ("3.11", "3.12", "3.13", "3.14", "3.15")}
CONTENDERS["CPython 3.15 + JIT"] = f"PYTHON_JIT=1 {python('3.15')} orderbook.py ops.bin {RUNS}"
CHECK_JIT = "import sys; print(sys.version.split()[0], 'jit enabled' if getattr(sys, '_jit', None) and sys._jit.is_enabled() else 'jit off')"


def main():
    log = [f"# {name}: " + sh(cmd.split(' orderbook.py')[0] + f' -c "{CHECK_JIT}"') for name, cmd in CONTENDERS.items()]
    log += machine()
    race(CONTENDERS, PASSES, log)
    save("results_pyversions.log", log)


if __name__ == "__main__":
    main()
