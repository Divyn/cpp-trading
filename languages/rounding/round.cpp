#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

int main() {
    const std::vector<double> cases = {0.5, 1.5, 2.5, -0.5, -1.5, -2.5};
    const std::vector<std::pair<std::string, std::function<double(double)>>> fns = {
        {"std::round", [](double x) { return std::round(x); }},
        {"std::rint", [](double x) { return std::rint(x); }},
    };
    for (const auto& [name, f] : fns) {
        std::printf("cpp,%s", name.c_str());
        for (double x : cases) std::printf(",%lld", (long long)f(x));
        for (int sign : {1, -1}) {
            double r = 0, e = 0;
            for (int k = 0; k < 1000; ++k) { double p = sign * (100.5 + k); r += f(p); e += p; }
            std::printf(",%g", r - e);
        }
        std::printf("\n");
    }
}
