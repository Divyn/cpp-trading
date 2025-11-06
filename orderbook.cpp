#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>

// Order structure - optimized for HFT (no dynamic strings initially)
struct Order {
    int id;
    double price;
    int quantity;
    bool is_buy;  // true = BUY, false = SELL (more efficient than string)
    
    Order(int i, double p, int q, bool buy) 
        : id(i), price(p), quantity(q), is_buy(buy) {}
};

// OrderBook - Core HFT data structure
// Price levels: map price -> total quantity at that price
class OrderBook {
private:
    // Bid side: highest price first (descending)
    std::map<double, int, std::greater<double>> bids;
    
    // Ask side: lowest price first (ascending - default)
    std::map<double, int> asks;
    
public:
    // Add order to the book
    void addOrder(const Order& order) {
        if (order.is_buy) {
            bids[order.price] += order.quantity;
        } else {
            asks[order.price] += order.quantity;
        }
    }
    
    // Remove order (by price level - simplified)
    void removeOrder(double price, int quantity, bool is_buy) {
        if (is_buy) {
            bids[price] -= quantity;
            if (bids[price] <= 0) {
                bids.erase(price);
            }
        } else {
            asks[price] -= quantity;
            if (asks[price] <= 0) {
                asks.erase(price);
            }
        }
    }
    
    // Get best bid (highest buy price)
    double getBestBid() const {
        if (bids.empty()) return 0.0;
        return bids.begin()->first;
    }
    
    // Get best ask (lowest sell price)
    double getBestAsk() const {
        if (asks.empty()) return 0.0;
        return asks.begin()->first;
    }
    
    // Calculate spread (bid-ask spread)
    double getSpread() const {
        double bid = getBestBid();
        double ask = getBestAsk();
        if (bid == 0.0 || ask == 0.0) return 0.0;
        return ask - bid;
    }
    
    // Display order book (top of book)
    void printTopOfBook() const {
        std::cout << "\n=== Order Book (Top Levels) ===\n";
        std::cout << std::fixed << std::setprecision(2);
        
        std::cout << "BIDS (Buy Orders):\n";
        int count = 0;
        for (const auto& [price, quantity] : bids) {
            if (count++ >= 5) break; // Show top 5 levels
            std::cout << "  " << price << " @ " << quantity << "\n";
        }
        
        std::cout << "\nASKS (Sell Orders):\n";
        count = 0;
        for (const auto& [price, quantity] : asks) {
            if (count++ >= 5) break; // Show top 5 levels
            std::cout << "  " << price << " @ " << quantity << "\n";
        }
        
        std::cout << "\nBest Bid: " << getBestBid() << "\n";
        std::cout << "Best Ask: " << getBestAsk() << "\n";
        std::cout << "Spread: " << getSpread() << "\n";
    }
};

int main() {
    // Create order book
    OrderBook book;
    
    // Add some orders (stack allocated - no new/delete!)
    std::vector<Order> orders = {
        Order(1, 123.50, 100, true),   // BUY at 123.50
        Order(2, 123.45, 50, true),    // BUY at 123.45
        Order(3, 123.55, 200, false),  // SELL at 123.55
        Order(4, 123.60, 75, false),   // SELL at 123.60
        Order(5, 123.40, 150, true),   // BUY at 123.40
    };
    
    // Add all orders to book
    for (const Order& order : orders) {
        book.addOrder(order);
    }
    
    // Display order book
    book.printTopOfBook();
    
    // Simulate order removal
    std::cout << "\n=== Removing 100 shares at 123.50 (BID) ===\n";
    book.removeOrder(123.50, 100, true);
    book.printTopOfBook();
    
    return 0;
}

