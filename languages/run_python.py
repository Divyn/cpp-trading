"""Race 5 ways to run the same orderbook.py, unchanged.

Run in this folder after run.py has made ops.bin: python3 run_python.py [passes]
Needs (Debian 13): apt-get install pypy3 cython3 python3-dev mypy python3-setuptools python3-venv
Nuitka is installed with pip into .venv-nuitka. Writes results_python.log.
"""
import sys

from race import machine, race, save, sh

PASSES = int(sys.argv[1]) if len(sys.argv) > 1 else 3
RUNS = 5
IMPORT = "python3 -c 'import orderbook; orderbook.main()' ../ops.bin"

BUILDS = [
    "rm -rf cython mypyc nuitka && mkdir cython mypyc nuitka",
    "cp orderbook.py cython/ && cd cython && cythonize -q -3 -i orderbook.py",
    "cp orderbook.py mypyc/ && cd mypyc && mypyc orderbook.py",
    "[ -d .venv-nuitka ] || (python3 -m venv .venv-nuitka && .venv-nuitka/bin/pip -q install nuitka)",
    ".venv-nuitka/bin/python -m nuitka --quiet --output-dir=nuitka orderbook.py",
]
VERSIONS = [
    "python3 --version",
    "pypy3 -c 'import sys, platform; print(\"PyPy\", platform.python_version(), sys.pypy_version_info[:3])'",
    "cython --version 2>&1",
    "mypyc --version 2>&1 | head -1",
    ".venv-nuitka/bin/python -m nuitka --version 2>&1 | head -1",
]
CONTENDERS = {
    "CPython 3.13": f"python3 orderbook.py ops.bin {RUNS}",
    "PyPy": f"pypy3 orderbook.py ops.bin {RUNS}",
    "Cython": f"cd cython && {IMPORT} {RUNS}",
    "mypyc": f"cd mypyc && {IMPORT} {RUNS}",
    "Nuitka": f"./nuitka/orderbook.bin ops.bin {RUNS}",
}


def main():
    for cmd in BUILDS:
        sh(cmd)
    log = ["# " + sh(cmd) for cmd in VERSIONS] + machine()
    race(CONTENDERS, PASSES, log)
    save("results_python.log", log)


if __name__ == "__main__":
    main()
