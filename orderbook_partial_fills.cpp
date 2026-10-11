#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

// Order structure - same as part one
struct Order {
    int id;
    double price;
    int quantity;
    bool is_buy;  // true = BUY, false = SELL

    Order(int i, double p, int q, bool buy)
        : id(i), price(p), quantity(q), is_buy(buy) {}
};

// OrderBook - price levels: map price -> total quantity at that price
class OrderBook {
private:
    // Bid side: highest price first (descending)
    std::map<double, int, std::greater<double>> bids;

    // Ask side: lowest price first (ascending - default)
    std::map<double, int> asks;

    // Fill an incoming order against the other side of the book, best price first.
    // Returns the quantity that is still not filled.
    template <typename Levels>
    int fill(Levels& levels, const Order& order) {
        int remaining = order.quantity;

        while (remaining > 0 && !levels.empty()) {
            auto best = levels.begin();
            bool crosses = order.is_buy ? best->first <= order.price
                                        : best->first >= order.price;
            if (!crosses) break;

            int traded = std::min(remaining, best->second);
            std::cout << "  FILL " << traded << " @ " << best->first << "\n";

            best->second -= traded;
            remaining -= traded;
            if (best->second == 0) levels.erase(best);
        }

        return remaining;
    }

    // Take quantity out of one price level. Returns what was actually there to take.
    template <typename Levels>
    int take(Levels& levels, double price, int quantity) {
        auto level = levels.find(price);
        if (level == levels.end()) return 0;

        int removed = std::min(quantity, level->second);
        level->second -= removed;
        if (level->second == 0) levels.erase(level);
        return removed;
    }

public:
    // Add order: fill what can be filled, the rest waits in the book
    void addOrder(const Order& order) {
        if (order.is_buy) {
            int remaining = fill(asks, order);
            if (remaining > 0) {
                bids[order.price] += remaining;
                std::cout << "  REST " << remaining << " @ " << order.price << "\n";
            }
        } else {
            int remaining = fill(bids, order);
            if (remaining > 0) {
                asks[order.price] += remaining;
                std::cout << "  REST " << remaining << " @ " << order.price << "\n";
            }
        }
    }

    // Cancel quantity at a price level. Returns the quantity actually removed.
    int removeOrder(double price, int quantity, bool is_buy) {
        return is_buy ? take(bids, price, quantity) : take(asks, price, quantity);
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
        std::cout << "BIDS (Buy Orders):\n";
        int count = 0;
        for (const auto& [price, quantity] : bids) {
            if (count++ >= 5) break;
            std::cout << "  " << price << " @ " << quantity << "\n";
        }

        std::cout << "ASKS (Sell Orders):\n";
        count = 0;
        for (const auto& [price, quantity] : asks) {
            if (count++ >= 5) break;
            std::cout << "  " << price << " @ " << quantity << "\n";
        }

        std::cout << "Best Bid: " << getBestBid()
                  << "  Best Ask: " << getBestAsk()
                  << "  Spread: " << getSpread() << "\n";
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(2);
    OrderBook book;

    // The book from part one, plus one more sell order at 123.70
    std::vector<Order> orders = {
        Order(1, 123.50, 100, true),   // BUY at 123.50
        Order(2, 123.45, 50, true),    // BUY at 123.45
        Order(3, 123.55, 200, false),  // SELL at 123.55
        Order(4, 123.60, 75, false),   // SELL at 123.60
        Order(5, 123.40, 150, true),   // BUY at 123.40
        Order(6, 123.70, 120, false),  // SELL at 123.70
    };
    for (const Order& order : orders) {
        book.addOrder(order);
    }

    std::cout << "=== Starting book ===\n";
    book.printTopOfBook();

    std::cout << "\n=== BUY 50 at 123.55 (smaller than the level) ===\n";
    book.addOrder(Order(7, 123.55, 50, true));
    book.printTopOfBook();

    std::cout << "\n=== BUY 300 at 123.60 (bigger than what is on sale) ===\n";
    book.addOrder(Order(8, 123.60, 300, true));
    book.printTopOfBook();

    std::cout << "\n=== SELL 100 at 123.45 ===\n";
    book.addOrder(Order(9, 123.45, 100, false));
    book.printTopOfBook();

    std::cout << "\n=== Cancel 500 at 123.40 (only 150 are there) ===\n";
    int removed = book.removeOrder(123.40, 500, true);
    std::cout << "  REMOVED " << removed << " of 500\n";
    book.printTopOfBook();

    return 0;
}
