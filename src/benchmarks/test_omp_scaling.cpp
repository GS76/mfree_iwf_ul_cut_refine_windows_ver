#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <windows.h>
#include <iomanip>
#include <algorithm>
#include <map>
#include <numeric>
#include <string>

// Function to get current core ID on Windows
int get_core_id() {
    return GetCurrentProcessorNumber();
}

// Function to get system processor count
int get_system_procs() {
    // try to get active processor count for all groups (handles > 64 cores)
    // ALL_PROCESSOR_GROUPS is 0xffff
    return GetActiveProcessorCount(0xffff);
}

// Compute-bound kernel: Monte Carlo Pi estimation
// Heavy floating point arithmetic, minimal memory access
// Returns pair<duration, imbalance_metric>
std::pair<double, double> benchmark_compute(long long iterations, int threads) {
    std::vector<double> thread_times(threads, 0.0);
    double start_time = omp_get_wtime();
    
    double sum = 0.0;
    
    #pragma omp parallel reduction(+:sum)
    {
        int tid = omp_get_thread_num();
        double t_start = omp_get_wtime();
        
        // Local work
        long long local_iter = iterations / threads;
        // Handle remainder for last thread
        if (tid == threads - 1) {
            local_iter += iterations % threads;
        }

        double local_sum = 0.0;
        double step = 1.0 / (double)iterations;
        
        // Manual loop splitting to measure thread time
        long long start_idx = (iterations / threads) * tid;
        
        for (long long i = 0; i < local_iter; i++) {
            double x = (start_idx + i + 0.5) * step;
            local_sum += 4.0 / (1.0 + x * x);
        }
        sum += local_sum;
        
        double t_end = omp_get_wtime();
        thread_times[tid] = t_end - t_start;
    }
    
    double end_time = omp_get_wtime();
    
    // Calculate load imbalance: (max_time - avg_time) / max_time
    double max_t = *std::max_element(thread_times.begin(), thread_times.end());
    double avg_t = std::accumulate(thread_times.begin(), thread_times.end(), 0.0) / threads;
    double imbalance = (max_t > 0) ? (max_t - avg_t) / max_t * 100.0 : 0.0;
    
    return {end_time - start_time, imbalance};
}

// Memory-bound kernel: Vector Addition (SAXPY)
// Large data set to exceed cache, testing memory bandwidth
double benchmark_memory(size_t size) {
    std::vector<double> a(size, 1.0);
    std::vector<double> b(size, 2.0);
    std::vector<double> c(size, 0.0);
    double scalar = 3.14159;
    
    // Warmup
    #pragma omp parallel for
    for (size_t i = 0; i < size; i++) {
        c[i] = a[i] + scalar * b[i];
    }
    
    double start_time = omp_get_wtime();
    
    #pragma omp parallel for
    for (size_t i = 0; i < size; i++) {
        c[i] = a[i] + scalar * b[i];
    }
    
    double end_time = omp_get_wtime();
    return end_time - start_time;
}

void check_affinity(int num_threads) {
    std::vector<int> core_ids(num_threads, -1);
    
    #pragma omp parallel num_threads(num_threads)
    {
        int thread_id = omp_get_thread_num();
        core_ids[thread_id] = get_core_id();
        
        #pragma omp critical
        {
            std::cout << "  Thread " << std::setw(2) << thread_id 
                      << " is running on Core " << std::setw(2) << core_ids[thread_id] << std::endl;
        }
    }
    
    // Check for unique cores
    std::vector<int> sorted_ids = core_ids;
    std::sort(sorted_ids.begin(), sorted_ids.end());
    auto last = std::unique(sorted_ids.begin(), sorted_ids.end());
    bool unique = (last == sorted_ids.end());
    
    std::cout << "  -> Affinity Check: " << (unique ? "PASSED (Unique Cores)" : "WARNING (Core Overlap Detected)") << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "==========================================================" << std::endl;
    std::cout << " OpenMP Scalability & Affinity Validation Benchmark" << std::endl;
    std::cout << "==========================================================" << std::endl;
    
    // 1. Topology Check
    int omp_max = omp_get_max_threads();
    int sys_procs = get_system_procs();
    
    std::cout << "System Topology Verification:" << std::endl;
    std::cout << "  OpenMP Max Threads: " << omp_max << std::endl;
    std::cout << "  System Processors:  " << sys_procs << " (GetActiveProcessorCount)" << std::endl;
    
    if (omp_max == sys_procs) {
        std::cout << "  -> Status: MATCH (Correctly detected)" << std::endl;
    } else {
        std::cout << "  -> Status: MISMATCH (Warning: OpenMP may not be using all cores)" << std::endl;
    }
    
    #ifdef _OPENMP
    std::cout << "  OpenMP Version: " << _OPENMP << std::endl;
    #endif

    // 2. Instrumentation Pause
    std::cout << "\n[Instrumentation Step]" << std::endl;
    std::cout << "Please open Task Manager or Resource Monitor now to observe CPU usage." << std::endl;
    std::cout << "Press ENTER to start the benchmark..." << std::endl;
    std::cin.get();

    int max_procs = omp_get_num_procs();
    
    // Define thread counts to test
    std::vector<int> test_threads;
    for (int i = 1; i <= max_procs; i *= 2) {
        test_threads.push_back(i);
    }
    if (test_threads.back() != max_procs) {
        test_threads.push_back(max_procs);
    }
    
    // Workload sizes
    long long compute_iter = 500000000; // 500 Million iterations
    size_t memory_size = 50000000;      // 50 Million doubles (~1.2GB total)
    
    // Test 1: Affinity
    std::cout << "\n[Test 1] Thread Affinity & Binding" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    omp_set_dynamic(0); // Disable dynamic adjustment of threads
    
    for (int t : test_threads) {
        std::cout << "\nTesting with " << t << " threads:" << std::endl;
        omp_set_num_threads(t);
        check_affinity(t);
    }
    
    // Test 2: Compute Bound
    std::cout << "\n[Test 2] Strong Scaling Performance (Compute Bound)" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Iterations: " << compute_iter << std::endl;
    std::cout << std::setw(8) << "Threads" << " | " 
              << std::setw(10) << "Time(s)" << " | " 
              << std::setw(8) << "Speedup" << " | " 
              << std::setw(8) << "Effic." << " | "
              << std::setw(8) << "Imbal." << std::endl;
    std::cout << "---------|------------|----------|----------|----------" << std::endl;
    
    double t1_time = 0.0;
    
    for (int t : test_threads) {
        omp_set_num_threads(t);
        auto result = benchmark_compute(compute_iter, t);
        double duration = result.first;
        double imbalance = result.second;
        
        if (t == 1) t1_time = duration;
        
        double speedup = t1_time / duration;
        double efficiency = (speedup / t) * 100.0;
        
        std::cout << std::setw(8) << t << " | " 
                  << std::setw(10) << std::fixed << std::setprecision(4) << duration << " | " 
                  << std::setw(7) << std::setprecision(2) << speedup << "x | " 
                  << std::setw(7) << efficiency << "% | " 
                  << std::setw(7) << imbalance << "%" << std::endl;
    }
    
    // Test 3: Memory Bandwidth
    std::cout << "\n[Test 3] Memory Bandwidth Scaling (Memory Bound)" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    double data_size_gb = (double)memory_size * 3 * 8 / (1024.0 * 1024.0 * 1024.0); // 3 arrays * 8 bytes
    std::cout << "Vector Size: " << memory_size << " elements (" << std::setprecision(2) << data_size_gb << " GB read/write)" << std::endl;
    
    std::cout << std::setw(8) << "Threads" << " | " 
              << std::setw(10) << "Time(s)" << " | " 
              << std::setw(8) << "Speedup" << " | " 
              << std::setw(10) << "BW(GB/s)" << std::endl;
    std::cout << "---------|------------|----------|------------" << std::endl;
    
    for (int t : test_threads) {
        omp_set_num_threads(t);
        double duration = benchmark_memory(memory_size);
        
        if (t == 1) t1_time = duration;
        
        double speedup = t1_time / duration;
        double bandwidth = data_size_gb / duration;
        
        std::cout << std::setw(8) << t << " | " 
                  << std::setw(10) << std::fixed << std::setprecision(4) << duration << " | " 
                  << std::setw(7) << std::setprecision(2) << speedup << "x | " 
                  << std::setw(10) << bandwidth << std::endl;
    }
    
    std::cout << "\nValidation Complete." << std::endl;
    return 0;
}
