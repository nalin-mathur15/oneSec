# Prime Sprint

This project explores how far we can push **prime number generation in under one second** using C++.

## Results

- **Initial attempt**  
  *8,719,624-th prime* → **155,189,239**  
  Method: **Basic segmented sieve (odd-only)** with dynamic base prime rebuilds and no major micro-optimisations.

- **Second attempt**  
  *14,630,843-th prime* → **268,435,399**  
  Method: **Optimized segmented sieve** with  
  – fixed single bitmap allocation,  
  – popcount-based counting,  
  – correct $\text{max}\{p^2,\lceil \text{low}/p\rceil \cdot p\}$ marking,  
  – careful tail masking for accuracy.

- **Third attempt**  
  *54,400,028-th prime* → **1,073,741,789**  
  Method: **Parallel segmented sieve** (multi-threaded) with  
  – one bitmap per thread,  
  – independent segment sieving in parallel,  
  – results merged at the end.  
  Scaled up to 16 cores to turn a ~14M ceiling into **>40M** primes in ~1 second.
