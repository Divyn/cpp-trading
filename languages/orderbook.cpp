// C++ order book. Build: g++ -O2 -std=c++17 orderbook.cpp -o orderbook_cpp
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

struct Level {
    int key;   // price for bids, -price for asks, so the best is always the largest
    int head;  // oldest order id (0 = none)
    int tail;  // newest order id
};

struct Side {
    bool bid;
    std::vector<Level> levels;  // sorted by key, best at the back

    int key(int price) const { return bid ? price : -price; }
    std::vector<Level>::iterator find(int key) {
        return std::lower_bound(levels.begin(), levels.end(), key,
                                [](const Level& l, int k) { return l.key < k; });
    }
};

struct Book {
    std::vector<int> qty, price, prev, next;
    std::vector<uint8_t> buy, alive;
    Side bids{true, {}}, asks{false, {}};
    long long fills = 0, traded = 0, cancelled = 0;

    explicit Book(int n) : qty(n), price(n), prev(n), next(n), buy(n), alive(n) {}

    void add(int id, int p, int q, bool is_buy) {
        Side& other = is_buy ? asks : bids;
        while (q > 0 && !other.levels.empty()) {
            Level& level = other.levels.back();
            int best = other.bid ? level.key : -level.key;
            if (is_buy ? best > p : best < p) break;
            int o = level.head;
            int t = std::min(q, qty[o]);
            qty[o] -= t;
            q -= t;
            fills++;
            traded += t;
            if (qty[o] == 0) {
                alive[o] = 0;
                level.head = next[o];
                if (level.head == 0) other.levels.pop_back();
                else prev[level.head] = 0;
            }
        }
        if (q == 0) return;

        Side& own = is_buy ? bids : asks;
        int k = own.key(p);
        auto level = own.find(k);
        if (level == own.levels.end() || level->key != k) level = own.levels.insert(level, Level{k, 0, 0});
        qty[id] = q;
        price[id] = p;
        buy[id] = is_buy;
        alive[id] = 1;
        next[id] = 0;
        prev[id] = level->tail;
        if (level->tail) next[level->tail] = id;
        else level->head = id;
        level->tail = id;
    }

    void cancel(int id) {
        if (!alive[id]) return;
        alive[id] = 0;
        cancelled += qty[id];
        Side& side = buy[id] ? bids : asks;
        auto level = side.find(side.key(price[id]));
        int p = prev[id], n = next[id];
        if (p) next[p] = n;
        else level->head = n;
        if (n) prev[n] = p;
        else level->tail = p;
        if (level->head == 0) side.levels.erase(level);
    }

    int best_bid() const { return bids.levels.empty() ? 0 : bids.levels.back().key; }
    int best_ask() const { return asks.levels.empty() ? 0 : -asks.levels.back().key; }
};

int main(int argc, char** argv) {
    const char* path = argc > 1 ? argv[1] : "ops.bin";
    int runs = argc > 2 ? std::atoi(argv[2]) : 5;
    FILE* f = std::fopen(path, "rb");
    int32_t count;
    if (!f || std::fread(&count, 4, 1, f) != 1) return 1;
    std::vector<int32_t> ops(4 * (size_t)count);
    if (std::fread(ops.data(), 4, ops.size(), f) != ops.size()) return 1;
    std::fclose(f);

    std::printf("cpp");
    std::vector<double> seconds;
    Book* last = nullptr;
    for (int r = 0; r < runs; r++) {
        delete last;
        Book* book = new Book(count + 2);
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < count; i++) {
            const int32_t* op = &ops[4 * (size_t)i];
            if (op[3] & 2) book->cancel(op[0]);
            else book->add(op[0], op[1], op[2], op[3] & 1);
        }
        auto end = std::chrono::steady_clock::now();
        seconds.push_back(std::chrono::duration<double>(end - start).count());
        last = book;
    }
    std::printf(",%lld,%lld,%lld,%d,%d", last->fills, last->traded, last->cancelled, last->best_bid(), last->best_ask());
    for (double s : seconds) std::printf(",%.6f", s);
    std::printf("\n");
    return 0;
}
