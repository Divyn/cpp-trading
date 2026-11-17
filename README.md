# Trading with C++

The code for the "Trading with C++" series on the
[Tech Fictionist](https://www.youtube.com/@TechFictionist) YouTube channel. Each
file is one small, complete program: an order book, a market maker, an on-chain
price fetch, and the benchmarks that race them. Every file builds on its own
with one `g++` command.

## The order book, step by step

Read these in order. Each part adds one thing to the last.

| Part | File | What it adds | Video |
| --- | --- | --- | --- |
| 0 | `trading_object.cpp` | An `Order` struct created on the heap and read through a pointer | |
| 1 | `orderbook.cpp` | Buy and sell sides in `std::map`, matching at the best price | [Getting Started with Creating an Orderbook in C++](https://www.youtube.com/watch?v=lcrxnnWD97g) |
| 2 | `orderbook_partial_fills.cpp` | An order that only partly fills stays on the book with what is left | [How to Handle Partial Fills in a C++ Order Book](https://www.youtube.com/watch?v=mODonoO_HrM) |
| 3 | `orderbook_price_time.cpp` | Price-time priority: best price first, then oldest order first, plus cancels | [How to Add Price-Time Priority to a C++ Order Book](https://www.youtube.com/watch?v=0RaVfZqVdyg) |

## On-chain and DeFi

| File | What it does | Video |
| --- | --- | --- |
| `btc_price.cpp` | Fetches the latest BTC price from the Bitquery API with libcurl | |
| `order_execution.cpp` | Simulates order execution on an EVM chain, with block times and a thread | [Simulating On-Chain Order Execution in C++](https://www.youtube.com/watch?v=pUNbUyPN3dQ) |
| `dex_market_maker.cpp` | A Uniswap V3 style market maker: liquidity positions in price ranges | [How to Code a Decentralized Market Maker](https://www.youtube.com/watch?v=iWW82eoP2i8) |

`btc_price.cpp` needs a Bitquery access token. Replace `ory_at_` in the
`Authorization: Bearer ory_at_` line with your full token before you build it.
Do not commit your token.

## Benchmarks

### `benchmark.cpp`

Times 5 common market data jobs on 1,000,000 generated prices: log returns,
rolling mean, EMA, OHLCV bars and rolling z-score. Each is the median of 5 runs.

### `book_levels_bench.cpp`: 4 ways to store price levels

The same price-time order book four times. Only the container that holds the
price levels changes. All four replay the same 1,000,000 operations and must
end with the same trades.

Results on a 1 vCPU Vultr server, Debian 13, g++ 14 `-O2`, 2026-11-12. Median of
3 passes, each pass the median of 5 runs. Raw output in
`book_levels_results.log`.

| Container | Million operations per second |
| --- | --- |
| `std::map` | 4.18 |
| sorted `std::vector` | 4.21 |
| `std::unordered_map` plus a cached best price | 4.97 |
| flat array indexed by price tick | 5.15 |

Video: [4 Ways to Store Prices in a C++ Order Book, Raced](https://youtube.com/shorts/1RFvssC8wkQ)

### `languages/`: the same order book in 6 languages

The same order book written in C++, Rust, Go, Java, Node and Python, with the
same rules and data layout, replaying the same 1,000,000 operations. All 6 made
the same 229,767 trades.

| Language | Million operations per second |
| --- | --- |
| Python 3.13 | 0.38 |
| Node 20 | 6.09 |
| Java 21 | 14.09 |
| Go 1.24 | 14.80 |
| C++ (g++ 14, `-O2`) | 16.24 |
| Rust 1.85 (`opt-level=3`) | 18.14 |

C++ built with `-O3` on the same server got 18.08. The rules, the build steps
and the raw results are in [`languages/README.md`](languages/README.md).

Video: [I Raced the Same Order Book in 6 Languages](https://youtube.com/shorts/xC_Mp3iLx-U)

### `api-calls/`: the same 100 API calls, 5 ways in Python

`requests` in a loop, one `Session`, a thread pool, `httpx` and `aiohttp`,
against a slow test server and a real HTTPS API. See
[`api-calls/README.md`](api-calls/README.md).

### `latency/`: Python vs C++ vs server location

The same order book request from 4 cities in Python and C++, to see whether the
language or the server's city costs more. See [`latency/README.md`](latency/README.md).

## Build and run

You need `g++` with C++17. `btc_price.cpp` also needs libcurl.

```bash
g++ -std=c++17 trading_object.cpp -o trading_object
g++ -std=c++17 orderbook.cpp -o orderbook
g++ -std=c++17 orderbook_partial_fills.cpp -o orderbook_partial_fills
g++ -std=c++17 orderbook_price_time.cpp -o orderbook_price_time
g++ -std=c++17 btc_price.cpp -o btc_price -lcurl
g++ -std=c++17 -pthread order_execution.cpp -o order_execution
g++ -std=c++17 dex_market_maker.cpp -o dex_market_maker
g++ -O2 -std=c++17 benchmark.cpp -o benchmark
g++ -O2 -std=c++17 book_levels_bench.cpp -o book_levels_bench
```

Then run any of them, for example `./orderbook_price_time`.
`book_levels_bench` takes the number of operations and runs:
`./book_levels_bench 1000000 5`.

Build the benchmarks with `-O2` or higher. Without it the timings mean little.

## C++ concepts

[`cpp_concepts.md`](cpp_concepts.md) explains every header, type and language
feature these files use, for readers new to C++.
