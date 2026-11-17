// Fetch Hyperliquid's BTC order book N times over one kept-alive HTTPS connection
// and time each request in two parts:
//   network: from sending the request to having the whole response body
//   code:    parsing the JSON into bid and ask levels and reading the best prices
// Read-only: it calls the public info endpoint, never places an order.
//
// Build: g++ -O2 -std=c++17 book_client.cpp -o book_client -lcurl
// Run:   ./book_client 51 150     (requests, gap in ms) -> one CSV line per request
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

using Clock = std::chrono::steady_clock;

static size_t collect(char* data, size_t size, size_t n, void* out) {
    static_cast<std::string*>(out)->append(data, size * n);
    return size * n;
}

struct Level {
    double px;
    double sz;
};

int main(int argc, char** argv) {
    int requests = argc > 1 ? std::atoi(argv[1]) : 51;
    int gap_ms = argc > 2 ? std::atoi(argv[2]) : 150;
    const char* body = R"({"type":"l2Book","coin":"BTC"})";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.hyperliquid.xyz/info");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, collect);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    for (int i = 0; i < requests; ++i) {
        response.clear();
        auto t0 = Clock::now();
        CURLcode rc = curl_easy_perform(curl);
        auto t1 = Clock::now();
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (rc != CURLE_OK || status != 200) {
            std::printf("cpp,%d,error,%ld\n", i, status);
            std::this_thread::sleep_for(std::chrono::milliseconds(gap_ms));
            continue;
        }
        auto book = nlohmann::json::parse(response);
        std::vector<Level> bids, asks;
        for (auto& l : book["levels"][0]) bids.push_back({std::stod(l["px"].get<std::string>()), std::stod(l["sz"].get<std::string>())});
        for (auto& l : book["levels"][1]) asks.push_back({std::stod(l["px"].get<std::string>()), std::stod(l["sz"].get<std::string>())});
        double best_bid = bids.front().px, best_ask = asks.front().px;
        auto t2 = Clock::now();
        std::printf("cpp,%d,%.3f,%.3f,%.1f,%.1f,%zu\n", i,
                    std::chrono::duration<double, std::milli>(t1 - t0).count(),
                    std::chrono::duration<double, std::milli>(t2 - t1).count(),
                    best_bid, best_ask, response.size());
        std::this_thread::sleep_for(std::chrono::milliseconds(gap_ms));
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}
