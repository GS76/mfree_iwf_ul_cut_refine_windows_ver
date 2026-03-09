# Plan: Temperature-Dependent Property Analysis

## Objective
Provide a comprehensive analysis of all temperature-dependent material and system properties in the codebase, specifying how to configure them and the required data formats.

## Steps

1.  **Codebase Audit for Temperature Dependencies**
    *   Inspect `src/simulation_data.h` to definitively list all properties wrapped in the `TableProperty` structure (e.g., Young's Modulus, Poisson's Ratio, etc.).
    *   Review `src/plasticity.h` and `src/johnson_cook_Sima_2010.h` to identify any intrinsic temperature dependencies in the plasticity models (e.g., thermal softening terms) that might *not* be configurable via tables but are temperature-dependent by formula.
    *   Check `src/thermal.h` for any fixed constants vs. variable properties.

2.  **Configuration Source Analysis**
    *   Examine `src/benchmarks/material_library.cpp` to confirm it is the single source of truth for material definitions.
    *   Identify the specific functions (e.g., `matlib_tial6v4_...`) that need modification to enable temperature-dependent behaviors.
    *   Verify the unit systems used in existing materials (SI vs. others) to establish the unit requirements.

3.  **Data Format and Schema Definition**
    *   Analyze `src/property.h` to document the exact interpolation logic (linear, clamped extrapolation).
    *   Define the C++ syntax/schema required to populate these tables (e.g., `std::vector<double>` pairs for Temperature vs. Property).

4.  **Final Report Generation**
    *   Compile findings into a structured response covering:
        *   **Identified Properties**: List of all properties that support temperature variation.
        *   **Configuration Location**: Exact files and functions to modify.
        *   **Data Schema**: Code examples and unit specifications for defining the data tables.
