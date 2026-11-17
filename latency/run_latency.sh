#!/bin/sh
# Run on each test server. Takes turns: 51 Python requests, then 51 C++ requests,
# 20 rounds, so both clients see the same minutes of market and network.
# The first request of each turn opens the connection (TCP and TLS handshake) and is
# kept in the raw log but left out of the medians.
# Output: raw_<region>.csv
set -e
REGION=${1:?region name}
ROUNDS=${2:-20}
g++ -O2 -std=c++17 book_client.cpp -o book_client -lcurl
OUT=raw_$REGION.csv
{
  echo "# region=$REGION host=$(hostname) start=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "# $(python3 --version) $(g++ --version | head -1) curl=$(curl --version | head -1 | cut -d' ' -f2)"
  echo "# $(nproc) vCPU, $(free -m | awk '/Mem:/{print $2}') MB RAM, $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2)"
  echo "# api.hyperliquid.xyz resolves to $(getent ahostsv4 api.hyperliquid.xyz | head -1 | cut -d' ' -f1)"
  echo "client,i,network_ms,code_ms,best_bid,best_ask,bytes"
} > "$OUT"
r=1
while [ "$r" -le "$ROUNDS" ]; do
  python3 book_client.py 51 150 >> "$OUT"
  ./book_client 51 150 >> "$OUT"
  r=$((r + 1))
done
echo "# end=$(date -u +%Y-%m-%dT%H:%M:%SZ)" >> "$OUT"
