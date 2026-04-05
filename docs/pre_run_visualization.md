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
- `tool_000000.vtk` (rigid tool triangulated patch)
- `fe_tool_000000.vtk` (optional: only if `MFREE_FE_TOOL_MSH` attaches an FE tool mesh)
- `precheck.json` (setup diagnostics)

## ParaView Visualization Setup

1. Open `results/out_000000.vtk` and `results/tool_000000.vtk` (and `results/fe_tool_000000.vtk` if present).
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
