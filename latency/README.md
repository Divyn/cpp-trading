# Python vs C++ vs server location

The same read-only request (Hyperliquid's BTC order book, `{"type":"l2Book","coin":"BTC"}`)
from 4 Vultr servers, Tokyo, Singapore, Mumbai and New York, with a Python client and a
C++ client. No orders, no login.

- `book_client.py`: one `requests.Session`, parses the book, reads the best prices.
- `book_client.cpp`: libcurl with one kept-open connection and nlohmann json, `-O2`.
- `run_latency.sh <region>`: 20 rounds of 51 Python then 51 C++ requests, 150 ms apart.
  The first request of each turn opens the connection and is left out of the medians.
- `summarize.py`: reads the `raw_*.csv` files and writes `results_latency.log`.

    apt install g++ libcurl4-openssl-dev nlohmann-json3-dev python3-requests
    sh run_latency.sh nrt

Results (1 vCPU servers, Python 3.13.5, g++ 14.2.0, 2026-11-17, 1,000 timed requests per
client per city, 0 errors), median milliseconds from sending the request to having the
parsed book:

| City | Python | C++ |
| --- | --- | --- |
| New York | 160.74 | 159.86 |
| Mumbai | 156.52 | 154.65 |
| Singapore | 75.52 | 73.35 |
| Tokyo | 5.92 | 4.39 |

Parsing the book took about 0.1 ms in both languages. The rest is the network.
