# Commands

## Running Order Object Test

```
g++ trading_object.cpp -o trading_object
./trading_object
```

## Running OrderBook Test

```
g++ -std=c++17 orderbook.cpp -o orderbook
./orderbook
```

## Running OrderBook Partial Fills (Part 2)

```
g++ -std=c++17 orderbook_partial_fills.cpp -o orderbook_partial_fills
./orderbook_partial_fills
```

## Fetching Latest BTC Price

```
g++ -std=c++17 btc_price.cpp -o btc_price -lcurl
./btc_price
```

## Running Order Execution Simulator

```
g++ -std=c++17 -pthread order_execution.cpp -o order_execution
./order_execution
```

## Running DEX Market Maker (Liquidity Provision in Price Bands)

```
g++ -std=c++17 dex_market_maker.cpp -o dex_market_maker
./dex_market_maker
```

