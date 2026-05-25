# Pre-Run Visualization (Model 5 FE-Tool-Only)

This branch uses Model 5 as the only supported run mode. Pre-run inspection is performed by exporting VTK files and opening them in ParaView.

## Generate a Model 5 Pre-Run Snapshot (no time integration)

```powershell
$env:MFREE_PREPROCESS_ONLY=1
$env:MFREE_FE_TOOL_MSH=".\snapshots\tool_plane_strain\meshes\tool_h_0.01mm.msh"
$env:MFREE_NO_RIGID_TOOL=1
$env:MFREE_USE_FE_TOOL_FOR_CONTACT=1
$env:MFREE_CONTACT_MU=0.35
.\build\Release\mfree_iwf.exe -m 5
```

Outputs in `results/`:

- `out_000000.vtk` (SPH workpiece)
- `fe_tool_000000.vtk` (FE tool)
- `precheck.json` (setup diagnostics)
- `validation_summary.json` (aggregate metrics)

Defaults used by the active Model 5 workflow (overridable by environment variables):

- `MFREE_FEED_PER_REV_MM=0.2`
- `MFREE_BASE_TARGET_FEED_MM=0.1`
- `MFREE_WORKPIECE_THICKNESS_MM=0.5`
- `MFREE_CONTACT_MU=0.35`

## Geometric Validation (FE Tool vs SPH Workpiece)

Use the model5 geometric validation workflow and inspect:

- `geom_validation.json`
- `geom_validation_000000.vtk`

## FE Tool BC Validation (Top/Rear Tags)

```powershell
.\scripts\fe_tool_bc_validate.ps1 -NoRigidTool -CuttingSpeed_m_min 100 -TopTag 110 -RearTag 114 -Ambient_C 25 -AnchorUx
```

Outputs in `results/fe_bc_validate/model_5/`:

- `fe_bc_top_edge.csv`
- `fe_bc_rear_edge.csv`
- `fe_bc_report.json`
- `fe_bc_convergence.txt`

## ParaView Visualization Setup

1. Open `results/out_000000.vtk` and `results/fe_tool_000000.vtk`.
2. Render SPH as `Point Gaussian` (or `Points`) and color by `density` or `num_neighbors`.
3. Render FE tool as `Surface With Edges` with partial opacity.
4. Inspect FE fields (`temperature`, `power`, `nodal_force`) and save a `.pvsm` state.

## Legacy Model 1–4 Notes

Legacy model1-4 scripts are archived under `scripts/legacy/` for historical traceability and are not part of the active forward workflow on this branch.
