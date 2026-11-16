"""Make the same 100 GET calls 5 ways in Python and time each.

Usage: python api_race.py [runs]
Target 1: a local server started here that waits 100 ms before every answer.
Target 2: one public HTTPS endpoint (Cloudflare's trace page), to show the TLS handshake cost.
Concurrent ways are capped at 10 calls in flight. Prints the median of the runs per way.
"""
import asyncio
import socket
import statistics
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import aiohttp
import httpx
import requests

CALLS = 100
IN_FLIGHT = 10
DELAY = 0.1
RUNS = int(sys.argv[1]) if len(sys.argv) > 1 else 5


class Slow(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def do_GET(self):
        time.sleep(DELAY)
        body = b'{"ok": true}'
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        # Headers and body in one write, so a kept-alive connection never waits on a delayed ACK.
        self._headers_buffer.append(b"\r\n" + body)
        self.flush_headers()

    def setup(self):
        super().setup()
        self.connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    def log_message(self, *args):
        pass


def get_loop(url):
    for _ in range(CALLS):
        requests.get(url, timeout=30).raise_for_status()


def session_loop(url):
    with requests.Session() as s:
        for _ in range(CALLS):
            s.get(url, timeout=30).raise_for_status()


def thread_pool(url):
    local = threading.local()

    def call(_):
        if not hasattr(local, "s"):
            local.s = requests.Session()
        local.s.get(url, timeout=30).raise_for_status()

    with ThreadPoolExecutor(IN_FLIGHT) as pool:
        list(pool.map(call, range(CALLS)))


async def _httpx(url):
    limits = httpx.Limits(max_connections=IN_FLIGHT)
    async with httpx.AsyncClient(limits=limits, timeout=30) as c:
        for r in await asyncio.gather(*(c.get(url) for _ in range(CALLS))):
            r.raise_for_status()


async def _aiohttp(url):
    async with aiohttp.ClientSession(connector=aiohttp.TCPConnector(limit=IN_FLIGHT)) as s:
        async def call():
            async with s.get(url) as r:
                r.raise_for_status()
                await r.read()
        await asyncio.gather(*(call() for _ in range(CALLS)))


WAYS = {
    "requests.get in a loop": get_loop,
    "requests.Session in a loop": session_loop,
    "thread pool of 10 + Session": thread_pool,
    "asyncio + httpx": lambda url: asyncio.run(_httpx(url)),
    "asyncio + aiohttp": lambda url: asyncio.run(_aiohttp(url)),
}


def race(label, url):
    print(f"## {label}: {url}, {CALLS} calls, {IN_FLIGHT} in flight, median of {RUNS} runs", flush=True)
    for name, way in WAYS.items():
        times = []
        for _ in range(RUNS):
            start = time.perf_counter()
            way(url)
            times.append(time.perf_counter() - start)
        runs = " ".join(f"{t:.2f}" for t in times)
        print(f"{name},{statistics.median(times):.3f},{runs}", flush=True)


def main():
    server = ThreadingHTTPServer(("127.0.0.1", 0), Slow)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    print(f"# python {sys.version.split()[0]}, requests {requests.__version__}, httpx {httpx.__version__}, aiohttp {aiohttp.__version__}")
    print(f"# {time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())}")
    race(f"local server, {int(DELAY * 1000)} ms delay", f"http://127.0.0.1:{server.server_port}/")
    race("public HTTPS", "https://1.1.1.1/cdn-cgi/trace")
    server.shutdown()


if __name__ == "__main__":
    main()
