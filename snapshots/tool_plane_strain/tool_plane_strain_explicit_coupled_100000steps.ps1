$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    # Keep this long run isolated from other result files.
    $env:MFREE_RESULTS_DIR = "results/tool_plane_strain_explicit_coupled_100000steps"
    $env:MFREE_CLEAN_RESULTS = "1"

    $env:MFREE_FE_TOOL_MSH = "snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh"
    $env:MFREE_FE_TOOL_ALIGN_CENTER = "0"

    $env:MFREE_USE_FE_TOOL_FOR_CONTACT = "1"
    $env:MFREE_CONTACT_ALPHA = "0.01"
    $env:MFREE_DEFORMABLE_FE_TOOL = "1"
    $env:MFREE_DEFORMABLE_FE_TOOL_EXPLICIT = "1"

    $env:MFREE_PREPROCESS_ONLY = "0"

    # The latest contact-conduction code uses:
    #   A_eff = sqrt(particle_area_per_depth) * MFREE_THERMAL_CONTACT_LENGTH_FACTOR
    #           * MFREE_PLANE_STRAIN_THICKNESS
    #
    # Keep this at 1.0 for the current per-unit-depth 2D formulation unless the
    # FE/SPH capacities are also consistently scaled by physical out-of-plane thickness.
    $env:MFREE_PLANE_STRAIN_THICKNESS = "1.0"
    $env:MFREE_THERMAL_CONTACT_LENGTH_FACTOR = "1.0"

    # Contact thermal model. These values are also used by the coupled timestep estimator.
    $env:MFREE_THERMAL_H_SEP = "1000"
    $env:MFREE_THERMAL_H_FULL = "100000"
    $env:MFREE_THERMAL_P_REF = "1e9"
    $env:MFREE_THERMAL_MAX_DT_PER_STEP = "1.0"
    $env:MFREE_THERMAL_FRAC_WP = "0.8"
    $env:MFREE_THERMAL_FRAC_TOOL = "0.2"

    # Use the coupled timestep estimator directly to determine the largest
    # admissible global step for this SPH workpiece + FE tool configuration.
    # Set this below 1.0 only if the printed limiter report or stability warnings
    # indicate that an additional safety margin is needed.
    $env:MFREE_DT_SCALE = "1.0"

    # Print all estimator limits and the controlling limiter at startup.
    $env:MFREE_TIMESTEP_PRINT = "1"
    $env:MFREE_TIMESTEP_WP_MECH_SAFETY = "0.25"
    $env:MFREE_TIMESTEP_WP_THERM_SAFETY = "0.20"
    $env:MFREE_TIMESTEP_TOOL_MECH_SAFETY = "0.90"

    # Slightly conservative with the updated FE conduction operator.
    $env:MFREE_TIMESTEP_TOOL_THERM_SAFETY = "0.90"

    $env:MFREE_TIMESTEP_INTERFACE_SAFETY = "0.50"

    # Do not use this as an old area-correction knob now that A_eff is fixed.
    # Leave it at 1.0 unless you are intentionally doing a sensitivity run.
    $env:MFREE_TIMESTEP_INTERFACE_AREA_FACTOR = "1.0"

    # Allow a 100000-step run. MFREE_T_FINAL_SCALE is kept high enough that
    # the executable stops by MFREE_MAX_STEPS instead of final simulation time.
    $env:MFREE_MAX_STEPS = "100000"
    $env:MFREE_T_FINAL_SCALE = "0.5"

    # Enable per-step/cumulative energy accounting output:
    # $env:MFREE_RESULTS_DIR/cutting_energy.csv
    $env:MFREE_LOG_ENERGY = "1"

    # Optional: reduce VTK output for a long diagnostic run.
    # Energy/thermal/metrics CSV logging is unaffected.
    $env:MFREE_OUTPUT_FREQ = "1000"

    # Let the solver derive explicit FE-tool substeps from the estimated timestep
    # and the FE tool critical limits.
    Remove-Item Env:MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS -ErrorAction SilentlyContinue
    Remove-Item Env:MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS -ErrorAction SilentlyContinue
    $env:MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS = "100"

    $env:MFREE_FE_TOOL_RAYLEIGH_A0 = "0"
    $env:MFREE_FE_TOOL_RAYLEIGH_A1 = "0"

    # For this diagnostic/validation run, fail hard on invalid explicit FE mechanics
    # rather than letting NaN/Inf states propagate deep into the run.
    $env:MFREE_FE_TOOL_HARD_FAIL_ON_INVALID = "1"

    $env:MFREE_FE_TOOL_RHO = "14500"

    # Use the physical/default carbide heat capacity. Avoid the old low cp=20 value
    # with the updated conduction/timestep code unless deliberately testing sensitivity.
    $env:MFREE_FE_TOOL_CP = "200"

    $env:MFREE_FE_TOOL_K = "80"
    $env:MFREE_FE_TOOL_E = "6e11"
    $env:MFREE_FE_TOOL_NU = "0.22"
    $env:MFREE_FE_TOOL_ALPHA = "4.5e-6"

    $env:MFREE_FE_TOOL_FIX_TAGS = "114"

    .\build\Release\mfree_iwf.exe -m 3
}
finally {
    Pop-Location
}