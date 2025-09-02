// for verification of primes.exe
#include <bits/stdc++.h>
using namespace std;

int nth_prime(int n) {
    if (n == 1) return 2;
    // Overestimate size using n log n + n log log n (Rosser–Schoenfeld).
    double nn = n;
    int limit = (int)(nn * (log(nn) + log(log(nn)))) + 10;
    vector<char> sieve(limit+1, true);
    sieve[0] = sieve[1] = false;
    for (int i = 2; i*i <= limit; i++) if (sieve[i]) {
        for (int j = i*i; j <= limit; j += i) sieve[j] = false;
    }
    int count = 0;
    for (int i = 2; i <= limit; i++) if (sieve[i]) {
        count++;
        if (count == n) return i;
    }
    return -1;
}

int main() {
    int n;
    cout << "Enter n: ";
    cin >> n;
    cout << n << "-th prime = " << nth_prime(n) << "\n";
}
