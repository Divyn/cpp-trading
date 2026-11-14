"""Race 6 JavaScript runtimes on the same orderbook.js, unchanged.

Run in this folder after run.py has made ops.bin: python3 run_js.py [passes]
Node 20 is Debian's package. Node 22, 24 and 26, Bun and Deno are downloaded
into .runtimes from their official release pages. Writes results_js.log.
Deno runs the file as orderbook.cjs, a copy with the CommonJS extension.
"""
import sys

from race import machine, race, save, sh

PASSES = int(sys.argv[1]) if len(sys.argv) > 1 else 3
RUNS = 5
R = ".runtimes"


def node(major):
    return (
        f"[ -x {R}/node{major} ] || (v=$(curl -fsSL https://nodejs.org/dist/latest-v{major}.x/SHASUMS256.txt"
        f" | sed -n 's/.*node-\\(v[0-9.]*\\)-linux-x64.tar.xz/\\1/p') && curl -fsSL"
        f" https://nodejs.org/dist/$v/node-$v-linux-x64.tar.xz | tar xJ -C {R} --strip-components=2"
        f" node-$v-linux-x64/bin/node && mv {R}/node {R}/node{major})"
    )


INSTALL = [
    f"mkdir -p {R}",
    node(22),
    node(24),
    node(26),
    f"[ -x {R}/bun ] || (curl -fsSL -o {R}/bun.zip https://github.com/oven-sh/bun/releases/latest/download/bun-linux-x64-baseline.zip"
    f" && cd {R} && python3 -c \"import zipfile; zipfile.ZipFile('bun.zip').extract('bun-linux-x64-baseline/bun', '.')\""
    f" && mv bun-linux-x64-baseline/bun bun && chmod +x bun && rm -rf bun.zip bun-linux-x64-baseline)",
    f"[ -x {R}/deno ] || (curl -fsSL -o {R}/deno.zip https://github.com/denoland/deno/releases/latest/download/deno-x86_64-unknown-linux-gnu.zip"
    f" && cd {R} && python3 -c \"import zipfile; zipfile.ZipFile('deno.zip').extractall('.')\" && chmod +x deno && rm deno.zip)",
    "cp orderbook.js orderbook.cjs",
]
CONTENDERS = {
    "Node 20": f"node orderbook.js ops.bin {RUNS}",
    "Node 22": f"{R}/node22 orderbook.js ops.bin {RUNS}",
    "Node 24": f"{R}/node24 orderbook.js ops.bin {RUNS}",
    "Node 26": f"{R}/node26 orderbook.js ops.bin {RUNS}",
    "Bun": f"{R}/bun orderbook.js ops.bin {RUNS}",
    "Deno": f"{R}/deno run --allow-read orderbook.cjs ops.bin {RUNS}",
}
VERSIONS = [
    "node --version", f"{R}/node22 --version", f"{R}/node24 --version", f"{R}/node26 --version",
    f"echo bun $({R}/bun --version)", f"{R}/deno --version | head -1",
]


def main():
    for cmd in INSTALL:
        sh(cmd)
    log = ["# " + sh(cmd) for cmd in VERSIONS] + machine()
    race(CONTENDERS, PASSES, log)
    save("results_js.log", log)


if __name__ == "__main__":
    main()
