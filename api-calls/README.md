# The same 100 API calls, 5 ways in Python

`api_race.py` makes the same 100 GET calls 5 ways and prints the median of 5
runs for each:

1. `requests.get` in a loop
2. one `requests.Session` in a loop
3. a thread pool of 10 sharing one `Session`
4. `asyncio` with `httpx`
5. `asyncio` with `aiohttp`

The concurrent ways keep at most 10 calls in flight.

Target 1 is a test server the script starts on localhost. It waits 100 ms
before every answer, like a slow API, and needs no TLS. Target 2 is one public
HTTPS page (Cloudflare's `https://1.1.1.1/cdn-cgi/trace`), to show what the TLS
handshake costs when every call opens a new connection.

    pip install requests httpx aiohttp
    python3 api_race.py 5

Results in `results_api.log` (1 vCPU Vultr server, Python 3.13, 2026-11-16),
seconds for 100 calls:

| Way | Test server, 100 ms per call | Public HTTPS |
| --- | --- | --- |
| `requests.get` in a loop | 10.25 | 3.29 |
| `Session` in a loop | 10.17 | 0.51 |
| `asyncio` + `httpx` | 1.52 | 0.44 |
| thread pool of 10 + `Session` | 1.05 | 0.33 |
| `asyncio` + `aiohttp` | 1.03 | 0.09 |

Check the API's rate limit before you send 10 calls at once.
