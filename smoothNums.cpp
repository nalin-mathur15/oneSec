#include <bits/stdc++.h>
using namespace std;

static vector<int> primesTill(int n) {
    if (n<2) return {};
    int m = n+1;
    vector<uint8_t> s(m, 1);
    s[0] = s[1] = 0;
    for(int i = 2; (int64_t)i*i <= n; i++) {
        if(s[i]) {
            for(int j = i*i; j <= n; j += i) {
                s[j] = 0;
            }
        }
    }
    vector<int> p; p.reserve(m / (max(1, (int)log(n))));
    for(int i = 2; i <= n; i++) if(s[i]) p.push_back(i);
    return p;
}

static inline bool verify(uint64_t n, const vector<int> &primes) {
    for(int p : primes) {
        if ((uint64_t)p > n) break;
        while(n % (uint64_t)p == 0) n /= (uint64_t)p;
        if(n == 1) return true;
    }
    return (n==1);
}

static uint64_t countSeg(uint64_t L, uint64_t H, const vector<int> &primes, const vector<float> &logprimes, vector<float> &acc, float eps = 1e-4f) {
    const uint64_t N = H-L;
    memset(acc.data(), 0, sizeof(float) * (size_t)N);
    for(size_t i = 0; i < primes.size(); i++) {
        uint64_t p = (uint64_t)primes[i];
        float logp = logprimes[i];
        for(uint64_t pk = p; pk < H;) {
            uint64_t start = (L+pk-1) / pk * pk;
            for(uint64_t x = start; x < H; x += pk) {
                acc[(size_t)(x-L)] += logp;
            }
            if(pk > (numeric_limits<uint64_t>::max() / p)) break;
            pk *= p;
        }
    }
    uint64_t c = 0;
    for(uint64_t i = 0; i < N; i++) {
        uint64_t n = L + i;
        if(n <=  1) {
            c++;
            continue;
        }
        float logn = log((float)n);
        if((acc[i] + eps * logn) >= logn){
            if(verify(n, primes)) c++;
        }
    }
    return c;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <y> [threads]\n";
        return 1;
    }
    int y = stoi(argv[1]);
    if (y < 2) {
        cout << "Primes bound y must be >=2. Trivially counting 1 only.\n";
        cout << "Total y-smooth (~1s): 1\nLargest N checked: 1\n";
        return 0;
    }
    unsigned hw = max(1u, thread::hardware_concurrency());
    unsigned T  = (argc >= 3 ? (unsigned)max(1, atoi(argv[2])) : hw);
    const uint64_t FLOATS_PER_THREAD = 1000000ULL;
    const uint64_t SPAN_PER_THREAD   = FLOATS_PER_THREAD;
    vector<int> base = primesTill(y);
    vector<float> logprimes; logprimes.reserve(base.size());
    for (int p : base) logprimes.push_back(log((float)p));

    using clk = chrono::steady_clock;
    auto t0 = clk::now();
    auto deadline = t0 + chrono::milliseconds(980);
    vector<vector<float>> acc(T, vector<float>((size_t)FLOATS_PER_THREAD));
    uint64_t total = 0;
    uint64_t low = 1;
    uint64_t batchSpan = SPAN_PER_THREAD * (uint64_t)T;

    while (true) {
        vector<thread> ths; ths.reserve(T);
        vector<uint64_t> counts(T, 0);
        for (unsigned t = 0; t < T; ++t) {
            uint64_t L = low + (uint64_t)t * SPAN_PER_THREAD;
            uint64_t H = L + SPAN_PER_THREAD;
            ths.emplace_back([&, t, L, H]() {
                counts[t] = countSeg(L, H, base, logprimes, acc[t]);
            });
        }
        for (auto& th : ths) th.join();

        for (unsigned t = 0; t < T; ++t) total += counts[t];

        if (clk::now() >= deadline) {
            cout << "Total y-smooth (~1s): " << total << "\n";
            cout << "Largest N checked: " << (low + batchSpan - 1) << "\n";
            cout << "y used: " << y << ", threads: " << T << "\n";
            break;
        }
        low += batchSpan;
        if (low + batchSpan < low) break;
    }
    return 0;
}

// initial attempt: 10597310 y-smooth; y=1e5
 