# OpenMP Scalability & Affinity Validation Report

## Executive Summary
The OpenMP validation suite was successfully executed on the target Windows environment. The system correctly identifies **12 logical processors**, and OpenMP threads are successfully spawned. However, the tests reveal significant findings regarding thread affinity and memory bandwidth saturation:
- **Scalability**: Compute-bound tasks show good scaling up to 4 threads (3.13x speedup), but efficiency drops at higher counts (4.81x speedup at 12 threads).
- **Affinity**: Default thread binding is loose. At max thread count (12), core overlap was detected, indicating threads competing for the same logical processors.
- **Memory Bandwidth**: Memory bandwidth saturates at ~16 GB/s with just 4 threads. Adding more threads does not improve memory-bound performance.

## 1. System Topology & Configuration
- **Detected Processors**: 12 (Logical)
- **OpenMP Max Threads**: 12
- **Status**: MATCH (Correct configuration)
- **OpenMP Version**: 201511 (OpenMP 4.5)

## 2. Workload Analysis

### Test 1: Thread Affinity & Binding
The test checks if threads are pinned to unique logical cores.
- **1-8 Threads**: PASSED. Threads were distributed to unique cores.
- **12 Threads**: **WARNING**. Core overlap detected (e.g., Thread 2 and Thread 8 both on Core 0).
    - *Implication*: Without `OMP_PROC_BIND=true` or `OMP_PLACES=cores`, the OS scheduler may migrate threads or stack them, causing context-switching overhead.

### Test 2: Compute-Bound Scaling (Monte Carlo)
Strong scaling test with heavy floating-point arithmetic.

| Threads | Time (s) | Speedup | Efficiency | Imbalance |
|---------|----------|---------|------------|-----------|
| 1       | 0.5000   | 1.00x   | 100.00%    | 0.00%     |
| 2       | 0.2760   | 1.81x   | 90.58%     | 0.00%     |
| 4       | 0.1600   | 3.13x   | 78.13%     | 8.59%     |
| 8       | 0.1170   | 4.27x   | 53.42%     | 11.97%    |
| 12      | 0.1040   | 4.81x   | 40.06%     | 8.90%     |

- **Observation**: Scaling is sub-linear. The drop in efficiency at 8+ threads suggests that the physical core count might be 6 (with hyperthreading) or that turbo boost frequencies are dropping significantly when all cores are active. The load imbalance (~9-12%) also contributes to the loss.

### Test 3: Memory Bandwidth (SAXPY)
Memory-bound test to identify bus saturation.

| Threads | Time (s) | Speedup | Bandwidth (GB/s) |
|---------|----------|---------|------------------|
| 1       | 0.0910   | 1.00x   | 12.28            |
| 2       | 0.0750   | 1.21x   | 14.90            |
| 4       | 0.0700   | 1.30x   | 15.97            |
| 8       | 0.0750   | 1.21x   | 14.90            |
| 12      | 0.0760   | 1.20x   | 14.71            |

- **Observation**: Bandwidth peaks at **~16 GB/s** with 4 threads.
- **Conclusion**: The system is memory bandwidth limited. For memory-intensive simulations, using more than 4 threads will likely not yield performance gains and may even degrade performance due to contention.

## Recommendations
1.  **Enable Thread Binding**: Run simulations with environment variable `OMP_PROC_BIND=TRUE` to prevent thread migration and core overlap.
2.  **Optimize Thread Count**: For this specific hardware, the "sweet spot" appears to be **4 to 6 threads**. Running with 12 threads yields diminishing returns.
3.  **Hybrid Parallelism**: Given the memory saturation, consider optimizing data structures for cache locality to reduce main memory pressure.
