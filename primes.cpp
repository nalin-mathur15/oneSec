#include <bits/stdc++.h>
using namespace std;

static vector<int> simple_primes_upto(int n) {
    if (n < 2) return {};
    int m = (n - 1) >> 1;
    vector<uint8_t> comp(m, 0);
    int lim = (int)((sqrt((double)n) - 3) / 2) + 1;
    for (int i = 0; i <= lim; ++i) if (!comp[i]) {
        int p = 2*i + 3;
        long long s = (1LL*p*p - 3) >> 1;
        for (long long j = s; j < m; j += p) comp[(size_t)j] = 1;
    }
    vector<int> primes; primes.reserve(max(1, m / (int)log(max(3, n))));
    primes.push_back(2);
    for (int i = 0; i < m; ++i) if (!comp[i]) primes.push_back(2*i + 3);
    return primes;
}

static pair<uint64_t,uint64_t> sieve_segment(uint64_t low, uint64_t high, const vector<int>& base, int pi_max, vector<uint64_t>& bits)
{
    if ((low & 1) == 0) ++low;
    if (high <= low) return {0, 0};

    const uint64_t lenOdds = (high - low) >> 1;
    const size_t words = (size_t)((lenOdds + 63) >> 6);
    fill(bits.begin(), bits.begin() + words, 0ULL);

    auto mark_with = [&](uint64_t p){
        uint64_t p2 = p * p;
        uint64_t start = ((low + p - 1) / p) * p;
        if (start < p2) start = p2;
        if ((start & 1) == 0) start += p;
        if (start >= high) return;
        uint64_t idx = (start - low) >> 1;
        const uint64_t step = p;
        for (uint64_t j = idx; j < lenOdds; j += step)
            bits[(size_t)(j >> 6)] |= (1ULL << (j & 63));
    };

    mark_with(3);
    mark_with(5);
    for (int k = 3; k <= pi_max; ++k) mark_with((uint64_t)base[k]);

    uint64_t count = 0, lastPrime = 0;
    if (words) {
        const unsigned tailBits = (unsigned)(lenOdds & 63U);
        const uint64_t tailMask = (tailBits == 0 ? ~0ULL : ((1ULL << tailBits) - 1ULL));

        size_t wEnd = (tailBits == 0 ? words : words - 1);
        for (size_t w = 0; w < wEnd; ++w) count += (uint64_t)__builtin_popcountll(~bits[w]);
        if (tailBits) count += (uint64_t)__builtin_popcountll((~bits[words - 1]) & tailMask);

        for (size_t w = words; w-- > 0;) {
            uint64_t inv = ~bits[w];
            if (w == words - 1 && tailBits) inv &= tailMask;
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
    const auto deadline = t0 + chrono::milliseconds(980);

    unsigned hw = max(1u, thread::hardware_concurrency());
    unsigned T = max(1u, hw);
    const uint64_t BYTES_PER_THREAD = 4ULL << 20;
    const uint64_t ODDS_PER_THREAD  = BYTES_PER_THREAD * 8ULL;
    const uint64_t SPAN_PER_THREAD  = ODDS_PER_THREAD << 1; 

    uint64_t low = 3;
    uint64_t segSpan = SPAN_PER_THREAD;
    uint64_t batchSpan = segSpan * T;
    uint64_t high_for_batch = low + batchSpan;

    uint64_t need = (uint64_t)floor(sqrt((long double)high_for_batch)) + 1;
    uint64_t limit = max<uint64_t>(need, 2000000);
    limit = min<uint64_t>(limit * 11 / 10 + 100, (uint64_t)INT_MAX);
    vector<int> base = simple_primes_upto((int)limit);
    int pi_max = 0;
    while ((uint64_t)base[pi_max]*(uint64_t)base[pi_max] < high_for_batch && pi_max + 1 < (int)base.size()) ++pi_max;

    vector<vector<uint64_t>> thread_bits(T, vector<uint64_t>((size_t)((ODDS_PER_THREAD + 63) >> 6), 0ULL));

    uint64_t total = 1;
    uint64_t lastPrime = 2;

    while (true) {
        uint64_t b2 = (uint64_t)base.back() * (uint64_t)base.back();
        if (b2 < high_for_batch && base.back() < INT_MAX-3) {
            uint64_t newLim = min<uint64_t>((uint64_t)INT_MAX, (uint64_t)((double)high_for_batch * 1.05));
            base = simple_primes_upto((int)newLim);
        }
        pi_max = 0;
        while ((uint64_t)base[pi_max]*(uint64_t)base[pi_max] < high_for_batch && pi_max + 1 < (int)base.size()) ++pi_max;

        vector<thread> ths;
        vector<uint64_t> counts(T, 0), lasts(T, 0);
        for (unsigned t = 0; t < T; ++t) {
            uint64_t L = low + t * segSpan;
            uint64_t H = L + segSpan;
            ths.emplace_back([&, t, L, H](){
                auto res = sieve_segment(L, H, base, pi_max, thread_bits[t]);
                counts[t] = res.first;
                lasts[t]  = res.second;
            });
        }
        for (auto& th : ths) th.join();

        for (unsigned t = 0; t < T; ++t) {
            total += counts[t];
            if (lasts[t] > lastPrime) lastPrime = lasts[t];
        }

        if (clk::now() >= deadline) break;

        low += batchSpan;
        high_for_batch = low + batchSpan;
        if (high_for_batch < low) break; 
    }

    cout << "Primes found (~1s): " << total << "\n";
    cout << "Largest prime: " << lastPrime << "\n";
    return 0;
}


// initial attempt:  8,719,624-th prime   (155,189,239)
// second attempt:  14,630,843-th prime   (268,435,399)
// third attempt:   41,405,139-th prime   (805,306,357)
// third attempt 2: 54,400,028-th prime (1,073,741,789)