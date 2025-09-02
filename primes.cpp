#include <bits/stdc++.h>
using namespace std;

static vector<int> simple_primes_upto(int n) {
    if (n < 2) return {};
    int m = (n - 1) >> 1;
    vector<uint8_t> comp(m, 0);
    int lim = (int)((sqrt((double)n) - 3)/2) + 1;
    for (int i = 0; i <= lim; ++i) if (!comp[i]) {
        int p = 2*i + 3;
        long long s = (1LL*p*p - 3) >> 1;
        for (long long j = s; j < m; j += p) comp[(size_t)j] = 1;
    }
    vector<int> primes = {2};
    for (int i = 0; i < m; ++i) if (!comp[i]) primes.push_back(2*i+3);
    return primes;
}

static pair<uint64_t,uint64_t>
sieve_segment(uint64_t low, uint64_t high, const vector<int>& base, int pi_max,
              vector<uint64_t>& bits)
{
    if ((low & 1) == 0) ++low;
    if (high <= low) return {0, 0};

    const uint64_t lenOdds = (high - low) >> 1;
    const size_t words = (size_t)((lenOdds + 63) >> 6);

    fill(bits.begin(), bits.begin() + words, 0ULL);
    {
        const uint64_t p = 3;
        uint64_t start = (low + p - 1)/p * p;
        if ((start & 1) == 0) start += p;
        uint64_t idx = (start - low) >> 1;
        for (uint64_t j = idx; j < lenOdds; j += p)
            bits[(size_t)(j >> 6)] |= (1ULL << (j & 63));
    }
    {
        const uint64_t p = 5;
        uint64_t start = (low + p - 1)/p * p;
        if ((start & 1) == 0) start += p;
        uint64_t idx = (start - low) >> 1;
        for (uint64_t j = idx; j < lenOdds; j += p)
            bits[(size_t)(j >> 6)] |= (1ULL << (j & 63));
    }

    for (int k = 3; k <= pi_max; ++k) {
        const uint64_t p = (uint64_t)base[k];
        uint64_t start = (low + p - 1) / p * p;
        if ((start & 1) == 0) start += p;
        uint64_t idx = (start - low) >> 1;
        const uint64_t step = p;
        for (uint64_t j = idx; j < lenOdds; j += step) {
            bits[(size_t)(j >> 6)] |= (1ULL << (j & 63));
        }
    }
    
    uint64_t count = 0;
    uint64_t lastPrime = 0;
    if (words) {
        const unsigned tailBits = (unsigned)(lenOdds & 63U);
        const uint64_t tailMask = (tailBits == 0 ? ~0ULL : ((1ULL << tailBits) - 1ULL));

        size_t wEnd = (tailBits == 0 ? words : words - 1);
        for (size_t w = 0; w < wEnd; ++w) {
            count += (uint64_t)__builtin_popcountll(~bits[w]);
        }
        if (tailBits) {
            const uint64_t inv = ~bits[words - 1] & tailMask;
            count += (uint64_t)__builtin_popcountll(inv);
        }
        for (size_t w = words; w-- > 0;) {
            uint64_t inv = ~bits[w];
            if (w == words - 1 && (lenOdds & 63U)) {
                inv &= tailMask;
            }
            if (inv) {
                int b = 63 - __builtin_clzll(inv);
                uint64_t j = (w << 6) + (unsigned)b;
                lastPrime = low + (j << 1);
                break;
            }
        }
    }

    return {count, lastPrime};
}

int main() {
    using clk = chrono::steady_clock;
    auto t0 = clk::now();
    const uint64_t TARGET_BITMAP_BYTES = 8ULL << 20;
    const uint64_t TARGET_ODDS = (TARGET_BITMAP_BYTES * 8ULL);
    const size_t   MAX_WORDS  = (size_t)((TARGET_ODDS + 63) >> 6);

    vector<uint64_t> bits(MAX_WORDS, 0ULL);
    uint64_t limit = 2000000;
    vector<int> base = simple_primes_upto((int)limit);

    uint64_t total = 1;
    uint64_t lastPrime = 2;

    uint64_t low = 3;
    uint64_t segSpan = TARGET_ODDS << 1;
    uint64_t high = low + segSpan;
    int pi_max = 0;
    uint64_t need = (uint64_t)floor(sqrt((long double)high)) + 1;
    if (need > (uint64_t)base.back()) {
        limit = (uint64_t)max<uint64_t>(need * 11 / 10, limit);
        if (limit > (uint64_t)INT_MAX) limit = (uint64_t)INT_MAX;
        base = simple_primes_upto((int)limit);
    }
    while ((uint64_t)base[pi_max] * (uint64_t)base[pi_max] < high && pi_max + 1 < (int)base.size()) ++pi_max;

    while (true) {
        uint64_t sq = (uint64_t)base.back() * (uint64_t)base.back();
        if (sq < high && base.back() < INT_MAX-3) {
            uint64_t newLim = min<uint64_t>((uint64_t)INT_MAX, (uint64_t)( (double)high * 1.05 ));
            base = simple_primes_upto((int)newLim);
            pi_max = 0;
            while ((uint64_t)base[pi_max]*(uint64_t)base[pi_max] < high && pi_max + 1 < (int)base.size()) ++pi_max;
        }

        auto [cnt, lastHere] = sieve_segment(low, high, base, pi_max, bits);
        total += cnt;
        if (lastHere) lastPrime = lastHere;

        auto ms = chrono::duration_cast<chrono::milliseconds>(clk::now() - t0).count();
        if (ms >= 980) break;

        low  = high;
        high = low + segSpan;

        while ((uint64_t)base[pi_max]*(uint64_t)base[pi_max] < high && pi_max + 1 < (int)base.size()) ++pi_max;
        if (high < low) break;
    }

    cout << "Primes found (~1s): " << total << "\n";
    cout << "Largest prime: " << lastPrime << "\n";
    return 0;
}

// initial attempt: 4,360,025-th prime (155,189,239)
// second attempt: 14,629,450-th prime (268,435,399)