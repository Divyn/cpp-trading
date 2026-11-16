#!/bin/sh
# Round the same numbers with each language's default round function (and its other rounding function).
# Columns: language,function,0.5,1.5,2.5,-0.5,-1.5,-2.5,drift on 1000 prices 100.5..1099.5,drift on the same 1000 as negatives
set -e
cd "$(dirname "$0")"
g++ -O2 -std=c++17 round.cpp -o round_cpp
rustc -C opt-level=3 round.rs -o round_rust 2>/dev/null
go build -o round_go round.go
javac -d . Round.java
{
  echo "# $(nproc) vCPU, $(free -m | awk '/Mem:/{print $2}') MB RAM, $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "# $(python3 --version) | node $(node --version) | $(g++ --version | head -1) | $(rustc --version) | $(go version) | $(java -version 2>&1 | head -1)"
  echo "language,function,0.5,1.5,2.5,-0.5,-1.5,-2.5,drift_prices,drift_negatives"
  for pass in 1 2 3; do
    python3 round.py; node round.js; ./round_cpp; ./round_rust; ./round_go; java Round
  done
} > results_rounding.log
cat results_rounding.log
