#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <map>
#include <random>
#include <iomanip>

using namespace std;
using namespace std::chrono;

const int N       = 1000000;
const int N_SMALL = 10000;
const int RUNS    = 5;

// Global sink — prevents compiler from optimising away unused results
volatile double sink = 0;

// ── timer ─────────────────────────────────────────────────────────────────────
template<typename Fn>
double timeit(Fn fn) {
    vector<double> times;
    for (int r = 0; r < RUNS; r++) {
        auto start = high_resolution_clock::now();
        fn();
        auto end   = high_resolution_clock::now();
        times.push_back(duration<double, milli>(end - start).count());
    }
    sort(times.begin(), times.end());
    return times[RUNS / 2]; // median
}

// ── generate data ─────────────────────────────────────────────────────────────
vector<double> make_prices(int n) {
    mt19937 rng(42);
    normal_distribution<double> dist(0.0, 1.0);
    vector<double> p(n);
    p[0] = 100.0;
    for (int i = 1; i < n; i++) p[i] = 100.0 + dist(rng);
    return p;
}

int main() {
    auto prices = make_prices(N);

    cout << "=================================================================\n";
    cout << "BENCHMARK: C++ vs Python — Trading Signal Operations\n";
    cout << "Dataset: " << N << " price ticks | Runs: " << RUNS << " (median)\n";
    cout << "Compiled with: g++ -O2\n";
    cout << "=================================================================\n\n";

    // ── 1. Log Returns ────────────────────────────────────────────────────────
    cout << "[1] Log Returns\n";

    auto log_returns = [&]() {
        vector<double> ret(N - 1);
        for (int i = 1; i < N; i++)
            ret[i-1] = log(prices[i] / prices[i-1]);
        sink = ret.back(); // prevent dead code elimination
        return ret;
    };

    double t_lr = timeit(log_returns);
    cout << "  C++ loop     (1M rows): " << fixed << setprecision(2) << t_lr << " ms\n\n";

    // ── 2. Rolling Mean ───────────────────────────────────────────────────────
    cout << "[2] Rolling Mean (20-period)\n";
    const int WINDOW = 20;

    auto rolling_mean = [&]() {
        vector<double> result(N - WINDOW + 1);
        double window_sum = 0;
        for (int i = 0; i < WINDOW; i++) window_sum += prices[i];
        result[0] = window_sum / WINDOW;
        for (int i = WINDOW; i < N; i++) {
            window_sum += prices[i] - prices[i - WINDOW];
            result[i - WINDOW + 1] = window_sum / WINDOW;
        }
        sink = result.back(); // prevent dead code elimination
        return result;
    };

    double t_rm = timeit(rolling_mean);
    cout << "  C++ sliding window (1M rows): " << t_rm << " ms\n\n";

    // ── 3. EMA ────────────────────────────────────────────────────────────────
    cout << "[3] EMA (alpha=0.1)\n";
    const double ALPHA = 0.1;

    auto ema = [&]() {
        vector<double> result(N);
        result[0] = prices[0];
        for (int i = 1; i < N; i++)
            result[i] = ALPHA * prices[i] + (1.0 - ALPHA) * result[i-1];
        sink = result.back();
        return result;
    };

    double t_ema = timeit(ema);
    cout << "  C++ loop     (1M rows): " << t_ema << " ms\n\n";

    // ── 4. OHLCV Aggregation ──────────────────────────────────────────────────
    cout << "[4] OHLCV Aggregation (100 ticks/block)\n";

    struct OHLCV { double open, high, low, close, volume; };

    auto ohlcv = [&]() {
        mt19937 vrng(42);
        normal_distribution<double> vdist(0.0, 1000.0);
        map<int, OHLCV> result;
        for (int i = 0; i < N; i++) {
            int block = i / 100;
            double vol = abs(vdist(vrng));
            if (result.find(block) == result.end()) {
                result[block] = {prices[i], prices[i], prices[i], prices[i], 0};
            }
            auto& c = result[block];
            c.high   = max(c.high, prices[i]);
            c.low    = min(c.low,  prices[i]);
            c.close  = prices[i];
            c.volume += vol;
        }
        sink = result.rbegin()->second.close;
        return result;
    };

    double t_ohlcv = timeit(ohlcv);
    cout << "  C++ map      (1M rows): " << t_ohlcv << " ms\n\n";

    // ── 5. Rolling Z-Score ────────────────────────────────────────────────────
    cout << "[5] Rolling Z-Score (30-period)\n";
    const int ZWINDOW = 30;

    auto zscore = [&]() {
        vector<double> result(N - ZWINDOW + 1);
        for (int i = ZWINDOW - 1; i < N; i++) {
            double sum = 0, sq_sum = 0;
            for (int j = i - ZWINDOW + 1; j <= i; j++) {
                sum    += prices[j];
                sq_sum += prices[j] * prices[j];
            }
            double mu  = sum / ZWINDOW;
            double std = sqrt(sq_sum / ZWINDOW - mu * mu);
            result[i - ZWINDOW + 1] = (prices[i] - mu) / (std + 1e-9);
        }
        sink = result.back();
        return result;
    };

    double t_zs = timeit(zscore);
    cout << "  C++ loop     (1M rows): " << t_zs << " ms\n\n";

    // ── Summary ───────────────────────────────────────────────────────────────
    cout << "=================================================================\n";
    cout << "C++ RESULTS SUMMARY\n";
    cout << "=================================================================\n";
    cout << left << setw(30) << "Operation"
         << right << setw(16) << "C++ Time (ms)" << "\n";
    cout << string(48, '-') << "\n";
    cout << left << setw(30) << "Log returns"
         << right << setw(14) << t_lr << " ms\n";
    cout << left << setw(30) << "Rolling mean (20)"
         << right << setw(14) << t_rm << " ms\n";
    cout << left << setw(30) << "EMA (alpha=0.1)"
        << right << setw(14) << t_ema << " ms\n";
    cout << left << setw(30) << "OHLCV aggregation"
         << right << setw(14) << t_ohlcv << " ms\n";
    cout << left << setw(30) << "Rolling z-score (30)"
         << right << setw(14) << t_zs << " ms\n";
    cout << "=================================================================\n";

    return 0;
}
