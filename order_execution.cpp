#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <queue>
#include <chrono>
#include <thread>

// Order structure
struct Order {
    int id;
    double price;
    int quantity;
    bool is_buy;
    int remaining_qty;
    
    Order(int i, double p, int q, bool buy) 
        : id(i), price(p), quantity(q), is_buy(buy), remaining_qty(q) {}
};

// Fill result for executed orders
struct Fill {
    int order_id;
    double fill_price;
    int fill_quantity;
    double gas_cost;
    double slippage;
    
    Fill(int oid, double fp, int fq, double gas, double slip)
        : order_id(oid), fill_price(fp), fill_quantity(fq), gas_cost(gas), slippage(slip) {}
};

// Low-level order execution engine for on-chain trading
class OrderExecutionEngine {
private:
    // Order book: price -> quantity
    std::map<double, int, std::greater<double>> bids;  // Highest first
    std::map<double, int> asks;  // Lowest first
    
    // Pending orders waiting for block confirmation
    std::queue<Order> pending_orders;
    
    // Execution history
    std::vector<Fill> execution_history;
    
    // On-chain parameters
    double gas_price_per_unit = 0.001;  // Gas cost per unit traded
    int block_time_ms = 12000;  // 12 seconds per block (Ethereum-like)
    
    // Calculate gas cost based on order size
    double calculateGasCost(int quantity) {
        return gas_price_per_unit * quantity;
    }
    
    // Calculate slippage (difference between expected and actual price)
    double calculateSlippage(double expected_price, double actual_price, bool is_buy) {
        if (is_buy) {
            return actual_price - expected_price;  // Positive = worse (paid more)
        } else {
            return expected_price - actual_price;  // Positive = worse (got less)
        }
    }
    
    // Match incoming order against order book
    std::vector<Fill> matchOrder(const Order& order) {
        std::vector<Fill> fills;
        int remaining = order.remaining_qty;
        
        if (order.is_buy) {
            // BUY order: match against ASKS (sell orders)
            auto it = asks.begin();
            while (remaining > 0 && it != asks.end() && it->first <= order.price) {
                int available = it->second;
                int fill_qty = (remaining < available) ? remaining : available;
                double fill_price = it->first;
                
                // Remove filled quantity from book
                it->second -= fill_qty;
                if (it->second <= 0) {
                    it = asks.erase(it);
                } else {
                    ++it;
                }
                
                // Calculate costs
                double gas = calculateGasCost(fill_qty);
                double slippage = calculateSlippage(order.price, fill_price, true);
                
                fills.push_back(Fill(order.id, fill_price, fill_qty, gas, slippage));
                remaining -= fill_qty;
            }
        } else {
            // SELL order: match against BIDS (buy orders)
            auto it = bids.begin();
            while (remaining > 0 && it != bids.end() && it->first >= order.price) {
                int available = it->second;
                int fill_qty = (remaining < available) ? remaining : available;
                double fill_price = it->first;
                
                // Remove filled quantity from book
                it->second -= fill_qty;
                if (it->second <= 0) {
                    it = bids.erase(it);
                } else {
                    ++it;
                }
                
                // Calculate costs
                double gas = calculateGasCost(fill_qty);
                double slippage = calculateSlippage(order.price, fill_price, false);
                
                fills.push_back(Fill(order.id, fill_price, fill_qty, gas, slippage));
                remaining -= fill_qty;
            }
        }
        
        // If order not fully filled, add remaining to book
        if (remaining > 0) {
            Order partial_order = order;
            partial_order.remaining_qty = remaining;
            if (order.is_buy) {
                bids[order.price] += remaining;
            } else {
                asks[order.price] += remaining;
            }
        }
        
        return fills;
    }
    
public:
    // Submit order for execution (simulates on-chain transaction)
    void submitOrder(const Order& order) {
        std::cout << "\n[BLOCK] Submitting order " << order.id 
                  << " (" << (order.is_buy ? "BUY" : "SELL") 
                  << " " << order.quantity << " @ " << order.price << ")\n";
        
        // Simulate block confirmation delay
        std::this_thread::sleep_for(std::chrono::milliseconds(block_time_ms / 4));
        
        // Execute order matching
        std::vector<Fill> fills = matchOrder(order);
        
        // Record fills
        for (const auto& fill : fills) {
            execution_history.push_back(fill);
            
            std::cout << "  [FILL] Order " << fill.order_id 
                      << ": " << fill.fill_quantity << " @ " 
                      << std::fixed << std::setprecision(2) << fill.fill_price
                      << " | Gas: $" << fill.gas_cost 
                      << " | Slippage: " << fill.slippage << "\n";
        }
        
        // Check if order fully filled
        int total_filled = 0;
        for (const auto& fill : fills) {
            total_filled += fill.fill_quantity;
        }
        
        if (total_filled < order.quantity) {
            std::cout << "  [PARTIAL] " << (order.quantity - total_filled) 
                      << " units remaining in order book\n";
        } else {
            std::cout << "  [FULL FILL] Order completely executed\n";
        }
    }
    
    // Add liquidity to order book (market makers)
    void addLiquidity(double price, int quantity, bool is_buy) {
        if (is_buy) {
            bids[price] += quantity;
        } else {
            asks[price] += quantity;
        }
    }
    
    // Display current order book state
    void printOrderBook() const {
        std::cout << "\n=== ORDER BOOK ===\n";
        std::cout << std::fixed << std::setprecision(2);
        
        std::cout << "ASKS (Sell):\n";
        int count = 0;
        for (const auto& [price, qty] : asks) {
            if (count++ >= 5) break;
            std::cout << "  " << price << " @ " << qty << "\n";
        }
        
        std::cout << "\nBIDS (Buy):\n";
        count = 0;
        for (const auto& [price, qty] : bids) {
            if (count++ >= 5) break;
            std::cout << "  " << price << " @ " << qty << "\n";
        }
    }
    
    // Display execution statistics
    void printExecutionStats() const {
        std::cout << "\n=== EXECUTION STATISTICS ===\n";
        std::cout << "Total fills: " << execution_history.size() << "\n";
        
        double total_gas = 0.0;
        double total_slippage = 0.0;
        int total_volume = 0;
        
        for (const auto& fill : execution_history) {
            total_gas += fill.gas_cost;
            total_slippage += fill.slippage;
            total_volume += fill.fill_quantity;
        }
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Total volume: " << total_volume << "\n";
        std::cout << "Total gas costs: $" << total_gas << "\n";
        std::cout << "Total slippage: " << total_slippage << "\n";
    }
    
    // Get best bid/ask
    double getBestBid() const {
        if (bids.empty()) return 0.0;
        return bids.begin()->first;
    }
    
    double getBestAsk() const {
        if (asks.empty()) return 0.0;
        return asks.begin()->first;
    }
};

int main() {
    OrderExecutionEngine engine;
    
    // Setup initial liquidity (market makers)
    std::cout << "=== SETTING UP INITIAL LIQUIDITY ===\n";
    engine.addLiquidity(50000.0, 10, false);  // Sell 10 @ 50000
    engine.addLiquidity(50010.0, 15, false);  // Sell 15 @ 50010
    engine.addLiquidity(50020.0, 20, false);  // Sell 20 @ 50020
    engine.addLiquidity(49990.0, 12, true);   // Buy 12 @ 49990
    engine.addLiquidity(49980.0, 18, true);   // Buy 18 @ 49980
    engine.addLiquidity(49970.0, 25, true);  // Buy 25 @ 49970
    
    engine.printOrderBook();
    
    // Simulate order execution
    std::cout << "\n=== SIMULATING ORDER EXECUTION ===\n";
    
    // Large buy order that will partially fill
    Order buy_order1(1, 50015.0, 30, true);
    engine.submitOrder(buy_order1);
    
    engine.printOrderBook();
    
    // Small sell order
    Order sell_order1(2, 49985.0, 5, false);
    engine.submitOrder(sell_order1);
    
    engine.printOrderBook();
    
    // Another buy order
    Order buy_order2(3, 50025.0, 8, true);
    engine.submitOrder(buy_order2);
    
    engine.printOrderBook();
    
    // Display final statistics
    engine.printExecutionStats();
    
    return 0;
}

