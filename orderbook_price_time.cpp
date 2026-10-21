#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>
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

// One price level: the orders waiting at that price, oldest at the front
using Queue = std::list<Order>;

// OrderBook - price levels: map price -> queue of orders at that price
class OrderBook {
private:
    // Bid side: highest price first (descending)
    std::map<double, Queue, std::greater<double>> bids;

    // Ask side: lowest price first (ascending - default)
    std::map<double, Queue> asks;

    // Where each resting order sits, so it can be cancelled by id
    struct Position {
        bool is_buy;
        double price;
        Queue::iterator spot;
    };
    std::unordered_map<int, Position> positions;

    // Fill an incoming order against the other side of the book.
    // Best price first, and at each price, the oldest order first.
    // Returns the quantity that is still not filled.
    template <typename Levels>
    int fill(Levels& levels, const Order& order) {
        int remaining = order.quantity;

        while (remaining > 0 && !levels.empty()) {
            auto best = levels.begin();
            bool crosses = order.is_buy ? best->first <= order.price
                                        : best->first >= order.price;
            if (!crosses) break;

            Queue& queue = best->second;
            Order& front = queue.front();

            int traded = std::min(remaining, front.quantity);
            std::cout << "  FILL " << traded << " @ " << best->first
                      << " (order " << front.id << ")\n";

            front.quantity -= traded;
            remaining -= traded;

            if (front.quantity == 0) {
                positions.erase(front.id);
                queue.pop_front();
            }
            if (queue.empty()) levels.erase(best);
        }

        return remaining;
    }

    // Fill against the other side, then the rest joins the back of the queue on its own side
    template <typename Own, typename Other>
    void place(Own& own, Other& other, Order order) {
        order.quantity = fill(other, order);
        if (order.quantity == 0) return;

        Queue& queue = own[order.price];
        queue.push_back(order);
        positions[order.id] = {order.is_buy, order.price, std::prev(queue.end())};
        std::cout << "  REST " << order.quantity << " @ " << order.price
                  << " (order " << order.id << ")\n";
    }

    // Take one order out of its queue. Returns the quantity it had left.
    template <typename Levels>
    int take(Levels& levels, const Position& position) {
        auto level = levels.find(position.price);
        int removed = position.spot->quantity;

        level->second.erase(position.spot);
        if (level->second.empty()) levels.erase(level);
        return removed;
    }

    template <typename Levels>
    void printSide(const Levels& levels) const {
        int count = 0;
        for (const auto& [price, queue] : levels) {
            if (count++ >= 5) break;
            std::cout << "  " << price << ":";
            for (const Order& order : queue) {
                std::cout << "  order " << order.id << " x " << order.quantity;
            }
            std::cout << "\n";
        }
    }

public:
    // Add order: fill what can be filled, the rest waits in the book
    void addOrder(const Order& order) {
        order.is_buy ? place(bids, asks, order) : place(asks, bids, order);
    }

    // Cancel an order by its id. Returns the quantity removed, 0 if it is not in the book.
    int cancelOrder(int id) {
        auto found = positions.find(id);
        if (found == positions.end()) return 0;

        Position position = found->second;
        positions.erase(found);
        return position.is_buy ? take(bids, position) : take(asks, position);
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

    // Display order book (top of book), every order in queue order
    void printTopOfBook() const {
        std::cout << "BIDS (Buy Orders):\n";
        printSide(bids);
        std::cout << "ASKS (Sell Orders):\n";
        printSide(asks);
        std::cout << "Best Bid: " << getBestBid()
                  << "  Best Ask: " << getBestAsk()
                  << "  Spread: " << getSpread() << "\n";
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(2);
    OrderBook book;

    // The book from part one, with two sellers at 123.55: order 3 first, then order 6
    std::vector<Order> orders = {
        Order(1, 123.50, 100, true),   // BUY at 123.50
        Order(2, 123.45, 50, true),    // BUY at 123.45
        Order(3, 123.55, 100, false),  // SELL at 123.55 (first in line)
        Order(4, 123.60, 75, false),   // SELL at 123.60
        Order(5, 123.40, 150, true),   // BUY at 123.40
        Order(6, 123.55, 100, false),  // SELL at 123.55 (second in line)
    };
    for (const Order& order : orders) {
        book.addOrder(order);
    }

    std::cout << "=== Starting book ===\n";
    book.printTopOfBook();

    std::cout << "\n=== BUY 150 at 123.55 ===\n";
    book.addOrder(Order(7, 123.55, 150, true));
    book.printTopOfBook();

    std::cout << "\n=== SELL 80 at 123.55 (joins the back) ===\n";
    book.addOrder(Order(8, 123.55, 80, false));
    book.printTopOfBook();

    std::cout << "\n=== Cancel order 6 ===\n";
    std::cout << "  REMOVED " << book.cancelOrder(6) << "\n";
    book.printTopOfBook();

    std::cout << "\n=== Cancel order 3 (already filled) ===\n";
    std::cout << "  REMOVED " << book.cancelOrder(3) << "\n";

    return 0;
}
