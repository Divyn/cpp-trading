// Writes the 1,000,000 operations every language replays, to ops.bin.
//
// The generator is the same as book_levels_bench.cpp (seed 42): 30% cancels of
// one of the last 10,000 orders, 10% orders priced through the middle so they
// trade, the rest resting 1 to 50 ticks away. The middle walks every 100 ops.
//
// File: int32 count, then count records of 4 little-endian int32:
//   id, price (ticks), quantity, flags (1 = buy, 2 = cancel)
//
// Build: g++ -O2 -std=c++17 gen_ops.cpp -o gen_ops
// Run:   ./gen_ops [operations] [seed] > ops.bin   (seed defaults to 42)

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

int main(int argc, char** argv) {
    int count = argc > 1 ? std::atoi(argv[1]) : 1000000;
    std::mt19937 rng(argc > 2 ? std::atoi(argv[2]) : 42);
    std::uniform_int_distribution<int> percent(0, 99);
    std::uniform_int_distribution<int> side(0, 1);
    std::uniform_int_distribution<int> away(1, 50);
    std::uniform_int_distribution<int> through(0, 5);
    std::uniform_int_distribution<int> size(1, 100);
    std::uniform_int_distribution<int> walk(-1, 1);

    std::vector<int32_t> out;
    out.reserve(1 + 4 * (size_t)count);
    out.push_back(count);
    int mid = 50000;
    int next_id = 1;

    for (int i = 0; i < count; i++) {
        if (i % 100 == 0) mid += walk(rng);
        int roll = percent(rng);

        if (roll < 30 && next_id > 1) {
            int low = std::max(1, next_id - 10000);
            std::uniform_int_distribution<int> pick(low, next_id - 1);
            out.insert(out.end(), {pick(rng), 0, 0, 2});
            continue;
        }

        bool is_buy = side(rng) == 1;
        int price;
        if (roll < 40) price = is_buy ? mid + through(rng) : mid - through(rng);
        else price = is_buy ? mid - away(rng) : mid + away(rng);
        out.insert(out.end(), {next_id++, price, size(rng), is_buy ? 1 : 0});
    }
    std::fwrite(out.data(), sizeof(int32_t), out.size(), stdout);
    return 0;
}
