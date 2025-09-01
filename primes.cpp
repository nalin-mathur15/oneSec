#include <bits/stdc++.h>
using namespace std;

static vector<int> primesUpTo(int n) {
    if (n < 2) return {};
    int m = (n-1) >> 1;
    vector<uint8_t> comp(m, 0);
    int lim = (int)((sqrt((double)n) - 3) / 2) + 1;
    for (int i = 0; i < lim; i++) {
        if (!comp[i]) {
            int p = 2*i + 3;
            long long start = (1LL*p*p - 3) >> 1;
            for (long long j = start; j < m; j += p) comp[(size_t)j] = 1;
        }
    }
    vector<int> primes = {2};
    for (int i = 0; i < m; i++) {
        if (!comp[i]) primes.push_back(2*i + 3);
    }
    return primes;
}

static uint64_t sieveSegement(uint64_t low, uint64_t high, const vector<int> &base, uint64_t &lastp) {
    if ((low & 1) == 0) low++;
    if (high <= low) return 0;

    const uint64_t l = (high - low) >> 1;
    const size_t words = (size_t)((l + 63) >> 6);
    vector<uint64_t> bits(words, 0ULL);
    const uint64_t sqrt_high = (uint64_t)floor(sqrt((long double)high));

    for (int p : base) {
        if ((uint64_t)p > sqrt_high) break;
        uint64_t start = (low + p - 1) / p * (uint64_t)p;
        if ((start & 1) == 0) start += p;
        uint64_t ind = (start - low) >> 1;
        const uint64_t step = (uint64_t)p;
        for (uint64_t j = ind; j < l; j += step) {
            bits[(size_t)(j >> 6)] |= (1ULL << (j & 63));
        }
    }
    uint64_t c = 0;
    for (uint64_t j = 0; j < l; j++) {
        if ((bits[(size_t)(j >> 6)] & (1ULL << (j & 63))) == 0) {
            lastp = low + (j << 1);
            c++;
        }
    }
    return c;
}

int main() {
    using clk = chrono::steady_clock;
    auto t0 = clk::now();
    const uint64_t SEG_ODD_MAX = 1ULL << 22;
    uint64_t lim = 1000000;
    vector<int> base = primesUpTo((int)lim);
    uint64_t t = 1;
    uint64_t lastp = 2;

    uint64_t low = 3;
    uint64_t high = low + (SEG_ODD_MAX << 1);
    while (true) {
        uint64_t need = (uint64_t)floor(sqrt((long double)high)) + 1;
        if (need > (uint64_t)base.back()) {
            lim = max(lim, need);
            lim = max(lim, (uint64_t)( (double)lim * 1.3 ));
            if (lim > (uint64_t)INT_MAX) lim = (uint64_t)INT_MAX;
            base = primesUpTo((int)lim);
        }

        t += sieveSegement(low, high, base, lastp);

        auto now = clk::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(now - t0).count();
        if (ms >= 980) break;

        low  = high;
        uint64_t seg_odds = SEG_ODD_MAX;
        if (ms < 400) seg_odds <<= 1;
        else if (ms < 700) seg_odds = (SEG_ODD_MAX * 3) >> 1;
        if (seg_odds > (1ULL << 24)) seg_odds = (1ULL << 24);
        high = low + (seg_odds << 1);
        if (high < low || high > std::numeric_limits<uint64_t>::max() - 100) break;
    }
    cout << "Primes found in around 1s: " << t << endl;
    cout << "Largest prime: " << lastp << endl;
    return 0;
}