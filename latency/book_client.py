"""Fetch Hyperliquid's BTC order book N times over one kept-alive HTTPS connection
and time each request in two parts, the same way book_client.cpp does:
  network: from sending the request to having the whole response body
  code:    parsing the JSON into bid and ask levels and reading the best prices
Read-only: it calls the public info endpoint, never places an order.

Run: python3 book_client.py 51 150     (requests, gap in ms) -> one CSV line per request
"""
import json
import sys
import time

import requests

requests_n = int(sys.argv[1]) if len(sys.argv) > 1 else 51
gap = (int(sys.argv[2]) if len(sys.argv) > 2 else 150) / 1000
session = requests.Session()

for i in range(requests_n):
    t0 = time.perf_counter()
    r = session.post("https://api.hyperliquid.xyz/info", json={"type": "l2Book", "coin": "BTC"})
    raw = r.content
    t1 = time.perf_counter()
    if r.status_code != 200:
        print(f"py,{i},error,{r.status_code}", flush=True)
        time.sleep(gap)
        continue
    book = json.loads(raw)
    bids = [(float(l["px"]), float(l["sz"])) for l in book["levels"][0]]
    asks = [(float(l["px"]), float(l["sz"])) for l in book["levels"][1]]
    best_bid, best_ask = bids[0][0], asks[0][0]
    t2 = time.perf_counter()
    print(f"py,{i},{(t1 - t0) * 1000:.3f},{(t2 - t1) * 1000:.3f},{best_bid:.1f},{best_ask:.1f},{len(raw)}", flush=True)
    time.sleep(gap)
