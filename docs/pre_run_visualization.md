# Pre-Run Visualization (SPH Workpiece + Tool)

This codebase does not ship an interactive “pre-processor” GUI. Pre-run inspection is performed by exporting visualization files (VTK legacy) and opening them in ParaView.

## Generate a Pre-Run Snapshot (no time integration)

Run any benchmark model with `MFREE_PREPROCESS_ONLY=1`. This builds neighbor lists, exports VTK for the SPH particles and the rigid tool, and writes a `precheck.json` report (overlap, density, neighbor stats).

Example (model 1):

```powershell
$env:MFREE_PREPROCESS_ONLY=1
.\build\Release\mfree_iwf.exe -m 1
```

Outputs in `results/`:

- `out_000000.vtk` (SPH particles)
- `tool_000000.vtk` (rigid tool triangulated patch; absent if `MFREE_NO_RIGID_TOOL=1`)
- `fe_tool_000000.vtk` (optional: only if `MFREE_FE_TOOL_MSH` attaches an FE tool mesh)
- `precheck.json` (setup diagnostics)
- `validation_summary.json` (aggregate metrics: deformation + contact-pressure stats)

Defaults used by the cutting benchmarks (overridable via environment variables):

- `MFREE_FEED_PER_REV_MM=0.2` (feed rate in mm/rev)
- `MFREE_BASE_TARGET_FEED_MM=0.1` (baseline depth-of-cut in mm; total target feed is base + feed_per_rev)
- `MFREE_WORKPIECE_THICKNESS_MM=0.5` (workpiece thickness in mm; top surface is held fixed at the benchmark value)
- `MFREE_FE_TOOL_MSH=./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh` (default tool mesh when present)
- `MFREE_NO_RIGID_TOOL=1` (optional: do not construct a rigid tool; contact uses the FE tool boundary loop)
- `MFREE_CONTACT_MU=0.35` (only used when `MFREE_NO_RIGID_TOOL=1`)

Optional coupled-motion (Galilean) setup:

- `MFREE_COUPLED_MOTION=1`
- `MFREE_PRIMARY_MOVING_BODY=tool|workpiece|both`
- `MFREE_COUPLED_MOTION_RATIO` in `[0,1]` (fraction of cutting speed assigned to the tool; overrides `MFREE_PRIMARY_MOVING_BODY`)

## Validate Models 1–4 in One Sweep

```powershell
.\scripts\validate_models_1_4.ps1 -PrimaryMovingBody tool -PreprocessOnly -UseMeshForContact:$false
```

Outputs go to `results/validate_model_1/` … `results/validate_model_4/` (each contains the VTKs plus `precheck.json` and `validation_summary.json`).

To validate FE-only contact (no rigid tool object), run:

```powershell
.\scripts\validate_models_1_4.ps1 -PrimaryMovingBody tool -PreprocessOnly -NoRigidTool
```

To generate a short “engaged” run (so contact-pressure and deformation fields become non-zero), run:

```powershell
.\scripts\validate_models_1_4_interaction.ps1 -PrimaryMovingBody tool -MaxSteps 5000 -EngageDistanceMM 0.001 -UseMeshForContact:$false
```

FE-only engaged run:

```powershell
.\scripts\validate_models_1_4_interaction.ps1 -PrimaryMovingBody tool -MaxSteps 5000 -EngageDistanceMM 0.001 -NoRigidTool
```

## ParaView Visualization Setup

1. Open `results/out_000000.vtk` and `results/tool_000000.vtk` (and `results/fe_tool_000000.vtk` if present). If `MFREE_NO_RIGID_TOOL=1`, open `results/fe_tool_000000.vtk` instead of `results/tool_000000.vtk`.
2. Set distinct rendering:
   - SPH particles: `Representation=Point Gaussian` (or `Points`), color by `density` or `num_neighbors`.
   - Tool: `Representation=Surface With Edges` and set `Opacity` (e.g., 0.2–0.4) so the interface is visible.
3. Boundary condition regions:
   - Color SPH by `fixed` to highlight fixed particles.
4. Mesh/field quality indicators:
   - SPH: `num_neighbors`, `glob_density_err`, `density`.
   - Tool: visually inspect the triangulated tool surface; for FE tool (if present), inspect `fe_tool_000000.vtk` fields `temperature`, `power`, `nodal_force`.
5. Save the visualization state:
   - `File → Save State…` (ParaView writes a `.pvsm` referencing the above VTK files).

## Intersection / Gap Checks

`results/precheck.json` includes:

- `tool.overlap.count`: number of SPH particles initially inside the rigid tool (intersection indicator).
- `tool.overlap.max_depth`: maximum penetration depth (world units).

If overlap is non-zero and unintended, reduce initial interpenetration by adjusting tool placement (`tl`, feed correction, or nudge) before running the full time loop.
