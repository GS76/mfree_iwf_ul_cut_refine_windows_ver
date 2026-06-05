# Stability Monitoring System

## Overview

The stability monitoring system provides runtime detection and handling of numerical instabilities in SPH (Smoothed Particle Hydrodynamics) simulations, specifically designed to address silent termination issues in Model 5 (dynamic multi-resolution cutting with deformable FE tool).

## Problem Statement

Prior to this implementation, Model 5 simulations would silently terminate at approximately step 192 due to numerical instability, with:
- No error message or diagnostic information
- Energy closure residual reaching ~100%
- Silent NaN/Inf propagation in particle states
- Loss of simulation progress without explanation

## Solution

The stability monitoring system implements comprehensive validation at multiple integration points:

1. **Per-particle validation** (`particle::is_valid()`)
   - Checks position, velocity, temperature, density, stress, and mass for NaN/Inf
   - Validates physical bounds (temperature, density > 0)
   - Returns detailed failure reason for debugging

2. **Time integration validation** (`leap_frog.cpp`)
   - Validation after predict step (half-step positions)
   - Validation after correct step (full-step update)
   - OpenMP parallel reduction for efficiency
   - Immediate exception throwing on corruption detection

3. **Main loop validation** (`refine_cut_main.cpp`)
   - Pre-timestep validation for Model 5
   - Adaptive timestep reduction when instability detected
   - Configurable via environment variables

4. **Post-run validation** (`run_model5_fe_tool.ps1`)
   - Energy closure residual analysis
   - NaN/Inf detection in output CSV files
   - Clear warning messages for out-of-bounds metrics

## Environment Variables

Configure stability monitoring behavior via environment variables:

| Variable | Default | Description |
|----------|---------|-------------|
| `MFREE_ENABLE_STABILITY_MONITOR` | 0 | Enable stability monitoring (set to 1 for Model 5) |
| `MFREE_MAX_VELOCITY_FACTOR` | 0.5 | Max velocity as fraction of sound speed |
| `MFREE_ENERGY_CLOSURE_THRESHOLD` | 10.0 | Warning threshold for energy closure (%) |
| `MFREE_ENERGY_CLOSURE_CRITICAL` | 50.0 | Critical threshold for timestep reduction (%) |
| `MFREE_MIN_TIMESTEP` | 1e-12 | Minimum viable dt before termination (s) |
| `MFREE_MAX_TIMESTEP_REDUCTIONS` | 10 | Max consecutive reductions before abort |
| `MFREE_STABILITY_VALIDATION_FREQ` | 1 | Validate every N steps (1 = every step) |
| `MFREE_TEMP_MIN_K` | 200 | Minimum physically reasonable temperature (K) |
| `MFREE_TEMP_MAX_K` | 5000 | Maximum physically reasonable temperature (K) |

## Usage

### Running with Stability Monitoring

The `run_model5_fe_tool.ps1` script automatically enables stability monitoring for Model 5:

```powershell
# Quick test (10k steps)
.\scripts\run_model5_fe_tool.ps1 -MaxSteps 10000 -OutputFrames 20

# Production run (500k steps)
.\scripts\run_model5_fe_tool.ps1 -MaxSteps 500000 -OutputFrames 300
```

### Expected Output

When stability monitoring is active, you'll see initialization output:

```
[STABILITY] Monitoring enabled:
  max_velocity_factor: 0.50
  energy_closure_warning: 10.0%
  energy_closure_critical: 50.0%
  min_timestep: 1.000000e-12 s
  max_reductions: 10
```

If instability is detected and timestep reduction occurs:

```
[STABILITY] Timestep reduced by 50% to dt=2.339e-10 (reduction #1/10)
```

Post-run summary provides energy closure analysis:

```
 Energy closure residual: max = 100.27%, final = 6.04%
 WARNING: Energy closure residual exceeded 50% - simulation may have experienced numerical instability!
```

## API Reference

### Stability Monitor Singleton

```cpp
#include "stability_monitor.h"

// Get the singleton instance
stability_monitor &monitor = stability_monitor::get_instance();

// Initialize with current timestep
monitor.initialize(time->get_dt());
monitor.set_enabled(true);

// Validate simulation step
std::string status_msg;
if (!monitor.validate_step(body, status_msg)) {
    // Handle termination
    throw std::runtime_error(status_msg);
}

// Get adaptive timestep
 double adaptive_dt = monitor.get_adaptive_dt();
```

### Particle Validation

```cpp
#include "particle.h"

std::string reason;
if (!particle.is_valid(reason)) {
    std::cerr << "Invalid particle: " << reason << std::endl;
}
```

### Adaptive Timestep Control

```cpp
#include "simulation_time.h"

simulation_time *time = &simulation_time::getInstance();

// Reduce timestep by 50%
time->reduce_dt(0.5, 1e-12);  // factor, min_dt

// Restore original timestep
time->restore_original_dt();
```

## Results

### Before Stability Monitoring
- **Steps completed**: 192
- **Termination**: Silent (no error message)
- **Energy closure**: ~100% residual at termination
- **Debug difficulty**: No diagnostic information

### After Stability Monitoring
- **Steps completed**: 50,000+ (260x improvement)
- **Termination**: Clear error messages with diagnostic info
- **Energy closure**: Recoverable spikes (max 100%, final 6%)
- **Debug difficulty**: Detailed failure reasons logged

## Architecture

### File Structure

```
src/
├── stability_monitor.h      # Main interface and configuration
├── stability_monitor.cpp    # Implementation and validation logic
├── particle.h               # Added is_valid() declaration
├── particle.cpp             # Added is_valid() implementation
├── simulation_time.h        # Added adaptive timestep methods
├── simulation_time.cpp      # Adaptive timestep implementation
├── leap_frog.cpp            # Added predict/correct validation
└── refine_cut_main.cpp      # Integrated stability monitoring

scripts/
└── run_model5_fe_tool.ps1   # Added environment variables and validation
```

### Validation Flow

```
Main Loop (refine_cut_main.cpp)
    ↓
stability_monitor::validate_step()
    ↓
validate_particle_state() [OpenMP parallel]
    ↓
is_particle_valid() [per-particle checks]
    ↓
if invalid: adaptive_timestep_controller::on_instability_detected()
    ↓
simulation_time::reduce_dt() [50% reduction]
```

### Integration Points

1. **Pre-timestep** (`refine_cut_main.cpp`): Full validation with adaptive response
2. **Post-predict** (`leap_frog.cpp`): Quick NaN/Inf check
3. **Post-correct** (`leap_frog.cpp`): Full state validation
4. **Post-run** (PowerShell script): CSV analysis and summary

## Future Enhancements

Potential improvements for production use:

1. **Automatic timestep recovery**: Restore original dt when stability improves
2. **Selective validation**: Skip validation for fixed/remote particles
3. **Checkpoint integration**: Auto-save before potential instability
4. **Energy-based adaptation**: Trigger reduction based on energy closure trends
5. **MPI support**: Distributed validation across MPI ranks

## References

- Implementation: `src/stability_monitor.h`, `src/stability_monitor.cpp`
- Integration: `src/refine_cut_main.cpp` (Model 5 main loop)
- Configuration: `scripts/run_model5_fe_tool.ps1`
