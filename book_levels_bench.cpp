// Four ways to store price levels in an order book, raced on the same orders.
//
// Every book uses the same matching rules as orderbook_price_time.cpp:
// best price first, oldest order first at each price, a std::list queue per
// price, and an id -> position map for cancels. Only the container that holds
// the price levels changes. Prices are whole ticks (an int), so all four books
// see the same keys.
//
// Build: g++ -O2 -std=c++17 book_levels_bench.cpp -o book_levels_bench
// Run:   ./book_levels_bench [operations] [runs]

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <map>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

struct Order {
    int id;
    int price;  // in ticks
    int quantity;
    bool is_buy;
};

using Queue = std::list<Order>;

struct Position {
    bool is_buy;
    int price;
    Queue::iterator spot;
};

// What every book must agree on after the replay
struct Totals {
    long long fills = 0;
    long long traded = 0;
    long long cancelled = 0;
    int best_bid = 0;
    int best_ask = 0;
};

// One operation in the replay: an add (quantity > 0) or a cancel (quantity == 0)
struct Op {
    Order order;
    bool cancel;
};

const int MID = 50000;    // starting price in ticks
const int RANGE = 100000; // the flat array covers ticks 0 .. RANGE-1

// ---------------------------------------------------------------------------
// Shared matching logic. Levels is any type with: empty(), best(), pop_best(),
// and a way to reach the queue at the best price.
// ---------------------------------------------------------------------------

// 1. std::map: what the series uses now
struct MapSide {
    bool is_bid;
    std::map<int, Queue> levels;  // ascending

    bool empty() const { return levels.empty(); }
    int best_price() const { return is_bid ? levels.rbegin()->first : levels.begin()->first; }
    Queue& best_queue() { return is_bid ? levels.rbegin()->second : levels.begin()->second; }
    void drop_best() {
        if (is_bid) levels.erase(std::prev(levels.end()));
        else levels.erase(levels.begin());
    }
    Queue& at(int price) { return levels[price]; }
    void erase_from(int price, Queue::iterator spot) {
        auto level = levels.find(price);
        level->second.erase(spot);
        if (level->second.empty()) levels.erase(level);
    }
};

// 2. sorted std::vector: best price kept at the back, so taking it is cheap
struct VectorSide {
    bool is_bid;
    // bids sorted ascending (best = highest = back), asks sorted descending (best = lowest = back)
    std::vector<std::pair<int, Queue>> levels;

    bool better_or_equal_toward_back(int a, int b) const { return is_bid ? a < b : a > b; }

    std::vector<std::pair<int, Queue>>::iterator find_slot(int price) {
        return std::lower_bound(levels.begin(), levels.end(), price,
                                [this](const std::pair<int, Queue>& level, int p) {
                                    return better_or_equal_toward_back(level.first, p);
                                });
    }

    bool empty() const { return levels.empty(); }
    int best_price() const { return levels.back().first; }
    Queue& best_queue() { return levels.back().second; }
    void drop_best() { levels.pop_back(); }
    Queue& at(int price) {
        auto slot = find_slot(price);
        if (slot == levels.end() || slot->first != price) {
            slot = levels.insert(slot, std::make_pair(price, Queue()));
        }
        return slot->second;
    }
    void erase_from(int price, Queue::iterator spot) {
        auto slot = find_slot(price);
        slot->second.erase(spot);
        if (slot->second.empty()) levels.erase(slot);
    }
};

// 3. flat array indexed by tick, with the best price tracked
struct ArraySide {
    bool is_bid;
    std::vector<Queue> levels = std::vector<Queue>(RANGE);
    int count = 0;  // non-empty levels
    int best = -1;

    bool empty() const { return count == 0; }
    int best_price() const { return best; }
    Queue& best_queue() { return levels[best]; }
    void find_next_best() {
        if (count == 0) { best = -1; return; }
        if (is_bid) { while (levels[best].empty()) best--; }
        else { while (levels[best].empty()) best++; }
    }
    void drop_best() {
        count--;
        find_next_best();
    }
    Queue& at(int price) {
        Queue& queue = levels[price];
        if (queue.empty()) {
            count++;
            if (best == -1 || (is_bid ? price > best : price < best)) best = price;
        }
        return queue;
    }
    void erase_from(int price, Queue::iterator spot) {
        levels[price].erase(spot);
        if (levels[price].empty()) {
            count--;
            if (price == best) find_next_best();
        }
    }
};

// 4. std::unordered_map plus a tracked best price
struct HashSide {
    bool is_bid;
    std::unordered_map<int, Queue> levels;
    int best = -1;

    bool empty() const { return levels.empty(); }
    int best_price() const { return best; }
    Queue& best_queue() { return levels.find(best)->second; }
    void find_next_best() {
        if (levels.empty()) { best = -1; return; }
        // Walk tick by tick away from the old best until a level exists
        if (is_bid) { while (levels.find(best) == levels.end()) best--; }
        else { while (levels.find(best) == levels.end()) best++; }
    }
    void drop_best() {
        levels.erase(best);
        find_next_best();
    }
    Queue& at(int price) {
        auto found = levels.find(price);
        if (found == levels.end()) {
            found = levels.emplace(price, Queue()).first;
            if (best == -1 || (is_bid ? price > best : price < best)) best = price;
        }
        return found->second;
    }
    void erase_from(int price, Queue::iterator spot) {
        auto level = levels.find(price);
        level->second.erase(spot);
        if (level->second.empty()) {
            levels.erase(level);
            if (price == best) find_next_best();
        }
    }
};

template <typename Side>
class Book {
public:
    Side bids{true};
    Side asks{false};
    std::unordered_map<int, Position> positions;
    Totals totals;

    Book() { positions.reserve(1 << 20); }

    void add(Order order) {
        Side& other = order.is_buy ? asks : bids;
        Side& own = order.is_buy ? bids : asks;

        // Fill against the other side: best price first, oldest order first
        while (order.quantity > 0 && !other.empty()) {
            int best = other.best_price();
            bool crosses = order.is_buy ? best <= order.price : best >= order.price;
            if (!crosses) break;

            Queue& queue = other.best_queue();
            Order& front = queue.front();
            int traded = std::min(order.quantity, front.quantity);
            front.quantity -= traded;
            order.quantity -= traded;
            totals.fills++;
            totals.traded += traded;

            if (front.quantity == 0) {
                positions.erase(front.id);
                queue.pop_front();
                if (queue.empty()) other.drop_best();
            }
        }
        if (order.quantity == 0) return;

        // The rest waits at the back of the queue at its price
        Queue& queue = own.at(order.price);
        queue.push_back(order);
        positions[order.id] = {order.is_buy, order.price, std::prev(queue.end())};
    }

    void cancel(int id) {
        auto found = positions.find(id);
        if (found == positions.end()) return;
        Position position = found->second;
        positions.erase(found);
        totals.cancelled += position.spot->quantity;
        Side& side = position.is_buy ? bids : asks;
        side.erase_from(position.price, position.spot);
    }

    Totals finish() {
        totals.best_bid = bids.empty() ? 0 : bids.best_price();
        totals.best_ask = asks.empty() ? 0 : asks.best_price();
        return totals;
    }
};

// The same operations for every book, from a fixed seed
std::vector<Op> make_ops(int count) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> percent(0, 99);
    std::uniform_int_distribution<int> side(0, 1);
    std::uniform_int_distribution<int> away(1, 50);
    std::uniform_int_distribution<int> through(0, 5);
    std::uniform_int_distribution<int> size(1, 100);
    std::uniform_int_distribution<int> walk(-1, 1);

    std::vector<Op> ops;
    ops.reserve(count);
    int mid = MID;
    int next_id = 1;

    for (int i = 0; i < count; i++) {
        if (i % 100 == 0) mid += walk(rng);
        int roll = percent(rng);

        if (roll < 30 && next_id > 1) {
            // Cancel one of the last 10,000 orders sent (it may already be filled)
            int low = std::max(1, next_id - 10000);
            std::uniform_int_distribution<int> pick(low, next_id - 1);
            ops.push_back({{pick(rng), 0, 0, false}, true});
            continue;
        }

        bool is_buy = side(rng) == 1;
        int price;
        if (roll < 40) {
            // Crossing order: priced through the middle, so it trades
            price = is_buy ? mid + through(rng) : mid - through(rng);
        } else {
            // Resting order: priced away from the middle
            price = is_buy ? mid - away(rng) : mid + away(rng);
        }
        ops.push_back({{next_id++, price, size(rng), is_buy}, false});
    }
    return ops;
}

template <typename Side>
double run_once(const std::vector<Op>& ops, Totals& out) {
    Book<Side> book;
    auto start = std::chrono::steady_clock::now();
    for (const Op& op : ops) {
        if (op.cancel) book.cancel(op.order.id);
        else book.add(op.order);
    }
    auto end = std::chrono::steady_clock::now();
    out = book.finish();
    return std::chrono::duration<double>(end - start).count();
}

template <typename Side>
void race(const char* name, const std::vector<Op>& ops, int runs, const Totals* expected) {
    std::vector<double> seconds;
    Totals totals;
    for (int r = 0; r < runs; r++) seconds.push_back(run_once<Side>(ops, totals));
    std::sort(seconds.begin(), seconds.end());
    double median = seconds[runs / 2];

    bool same = expected == nullptr ||
                (totals.fills == expected->fills && totals.traded == expected->traded &&
                 totals.cancelled == expected->cancelled &&
                 totals.best_bid == expected->best_bid && totals.best_ask == expected->best_ask);

    std::printf("%s,%.4f,%.3f,%.4f,%.4f,%lld,%lld,%lld,%d,%d,%s\n", name, median,
                ops.size() / median / 1e6, seconds.front(), seconds.back(), totals.fills,
                totals.traded, totals.cancelled, totals.best_bid, totals.best_ask,
                same ? "match" : "MISMATCH");
}

int main(int argc, char** argv) {
    int count = argc > 1 ? std::atoi(argv[1]) : 1000000;
    int runs = argc > 2 ? std::atoi(argv[2]) : 5;
    std::vector<Op> ops = make_ops(count);

    // The std::map book is the reference the others must match
    Totals reference;
    run_once<MapSide>(ops, reference);

    std::printf("book,median_s,million_ops_per_s,min_s,max_s,fills,traded,cancelled,best_bid,best_ask,check\n");
    race<MapSide>("std::map", ops, runs, &reference);
    race<VectorSide>("sorted std::vector", ops, runs, &reference);
    race<ArraySide>("flat array by tick", ops, runs, &reference);
    race<HashSide>("std::unordered_map + best", ops, runs, &reference);
    return 0;
}
