# Technical Overview

## 1. Introduction
The `mfree_iwf_ul_cut_refine` project is a high-performance C++ simulation framework for **metal cutting processes** (orthogonal cutting). It implements the **Updated Lagrangian Reproducing Kernel Particle Method (UL-RKPM)**, a meshfree method suitable for large deformations and dynamic problems.

Key features include:
*   **Meshfree Discretization**: Avoids mesh distortion issues common in FEM.
*   **Dynamic Refinement**: Adaptive particle insertion/deletion to capture high gradients (e.g., in shear bands).
*   **Physics Coupling**: Thermal-mechanical coupling with plasticity and contact mechanics.
*   **Parallelization**: OpenMP-accelerated for multi-core CPUs.

## 2. Software Architecture

### 2.1 Core Modules
The codebase is modular, separating the main loop, data structures, and physical solvers:

*   **Main Loop (`src/refine_cut_main.cpp`)**: The entry point. It handles initialization, argument parsing, and the main time-stepping loop.
*   **Time Integration (`src/leap_frog.cpp`)**: Implements an explicit 2nd-order Leapfrog integration scheme (`init`, `predict`, `correct`).
*   **Data Structure (`src/body.h`, `src/particle.h`)**:
    *   `body`: Manages the collection of particles, neighbor lists, and tool interaction.
    *   `particle`: Stores state variables (position, velocity, stress, temperature, etc.).
*   **Physics Solvers**:
    *   `src/contact.cpp`: Handles tool-workpiece contact and friction (Coulomb/L-Dyna).
    *   `src/thermal.cpp`: Solves the heat conduction equation.
    *   `src/plasticity.cpp`: Implements the Johnson-Cook plasticity model with radial return mapping.
    *   `src/derivatives.cpp`: Computes spatial derivatives (velocity gradient, stress divergence).
    *   `src/correctors.cpp`: Applies artificial viscosity, Monaghan stress correction, and XSPH stabilization.
    *   `src/adaptivity.cpp`: Handles dynamic particle splitting and merging.

### 2.2 Data Flow
1.  **Initialization**:
    *   Particles are generated in a grid pattern (single or multi-resolution) based on the selected benchmark model.
    *   Material properties and tool geometry are assigned.
2.  **Time Step (`leap_frog::step`)**:
    *   **Neighbor Search**: `body::construct_verlet_lists` updates spatial connectivity.
    *   **Prediction**: Update positions/velocities based on current rates.
    *   **Force Calculation**:
        *   Contact forces (tool interaction).
        *   Internal forces (stress divergence, artificial viscosity).
    *   **Thermal Solve**: Heat conduction.
    *   **Correction**: Final update of state variables.
    *   **Constitutive Update**: Plasticity (radial return) updates stress and plastic strain.
    *   **Refinement**: Check criteria and split/merge particles if necessary.
3.  **Output**:
    *   Results are written to `.vtk` (ParaView) and `.txt` files in the `results/` directory.

### 2.3 Parallelism
The project uses **OpenMP** for shared-memory parallelization. Critical loops (particle updates, derivative calculations) are decorated with `#pragma omp parallel for`.
*   **Thread Safety**: Plasticity calculations use thread-local copies of the material model to avoid race conditions.
*   **Performance**: Scales with the number of CPU cores.

## 3. Key Algorithms

### 3.1 UL-RKPM
The method uses Reproducing Kernel Particle Method shape functions to interpolate field variables. The "Updated Lagrangian" aspect means derivatives are computed with respect to the current configuration, and particles move with the material flow.

### 3.2 Johnson-Cook Plasticity
The material response is modeled using the Johnson-Cook constitutive law, which accounts for:
*   **Strain Hardening**
*   **Strain-Rate Sensitivity**
*   **Thermal Softening**

The specific variant implemented is the **Sima & Ozel (2010)** modification, which adds a `tanh` term to capture flow stress behavior in Ti6Al4V.

### 3.3 Contact Mechanics
Contact is modeled using a penalty method. The tool is defined as a rigid body composed of line segments (2D).
*   **Normal Force**: Proportional to penetration depth (`compute_contact_force_nianfei`).
*   **Friction**: Modeled using a velocity-dependent friction law (L-Dyna style).

### 3.4 Dynamic Refinement
To maintain accuracy in high-deformation zones (shear bands) without excessive computational cost, particles are dynamically:
*   **Split**: When plastic strain or other criteria exceed a threshold (`adaptivity::perform_split_...`).
*   **Merged**: In low-deformation zones to reduce particle count.
