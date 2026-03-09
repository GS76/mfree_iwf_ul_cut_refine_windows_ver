# Specification: Project Documentation

## Goal
Create comprehensive documentation for the `mfree_iwf_ul_cut_refine_windows_ver` project, covering its core purpose, architecture, and user-configurable parameters.

## Outputs
1.  `TECHNICAL_OVERVIEW.md`: High-level description of the project, its scientific basis (UL-RKPM), software architecture, and key modules.
2.  `CONFIGURATION_GUIDE.md`: Detailed documentation of all configurable parameters, including command-line arguments, hardcoded constants, and material properties.

## Scope
### 1. Core Purpose
*   Simulation of metal cutting processes (orthogonal cutting).
*   Method: Meshfree Updated Lagrangian Reproducing Kernel Particle Method (UL-RKPM).
*   Features: Adaptive refinement, thermal coupling, plasticity (Johnson-Cook), contact mechanics.

### 2. Architecture
*   **Language**: C++17.
*   **Parallelism**: OpenMP (recently added).
*   **Math Library**: GLM.
*   **Structure**:
    *   **Main Loop**: Explicit time integration (Leapfrog).
    *   **Modules**: `leap_frog`, `contact`, `thermal`, `plasticity`, `adaptivity`, `derivatives`, `correctors`.
    *   **Data**: `body` class managing `particles`.

### 3. Configurable Parameters
*   **Command Line**: `-m` (Model selection).
*   **Simulation Settings**: Time step, total time, output frequency.
*   **Physical Constants**: Material properties (Young's modulus, Poisson's ratio, density).
*   **Plasticity**: Johnson-Cook parameters (A, B, C, n, m, etc.).
*   **Numerical Constants**: Artificial viscosity/stress parameters, stabilization (XSPH).
*   **Geometry**: Tool and workpiece dimensions (defined in benchmarks).

## Plan
1.  **Analyze Architecture**: Review `src/` to map out the interaction between modules and the main loop.
2.  **Analyze Parameters**: Systematically check `simulation_data.h`, `refine_cut_main.cpp`, and benchmark files (`benchmarks/*.h`) to list all adjustable values.
3.  **Draft Documentation**: Write the markdown files with clear headings, descriptions, and code references.
