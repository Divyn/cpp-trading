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

