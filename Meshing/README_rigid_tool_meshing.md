# Rigid Tool Meshing (Gmsh SDK)

This repository vendors a Gmsh SDK in `Meshing/gmsh-4.15.2-Windows64-sdk/`. The script below generates a 2D triangular mesh of the rigid cutting tool cross-section in the same world coordinate system used by the solver, with a prescribed refinement zone around the cutting-edge radius center.

## Inputs

The script supports two geometry sources:

- `--tool-txt`: a `tool_*.txt` file written by `tool::print(step)` (contains tool polygon vertices plus fillet center/radius)
- `--tl-angles`: tool reconstructed from `(tl, length, height, rake, clearance, fillet_radius)` (matches the constructor used in benchmarks)

## Refinement Specification (as requested)

- Cutting edge radius center: uses fillet center from `tool_*.txt` (or `--refine-center-x/y` override)
- Critical refinement zone: circular region, diameter `0.2 mm`
- Fine element size: `0.002 mm`
- Coarsening: `Threshold` size-field transitioning from the fine size at radius `0.1 mm` to a user-controlled maximum size

All refinement sizes are provided in millimeters and internally converted to the model unit (selected by `--unit`).

## Boundary / Physical Groups

The exported `.msh` is Gmsh v2 ASCII and includes physical tags that the FE-tool loader already reads:

- `TOOL_DOMAIN` (2D): tag `1`
- `TOOL_BOUNDARY` (1D): tag `100`
- `CUTTING_EDGE` (1D): tag `101` (only when a fillet arc is present)
- Point groups (0D):
  - `TOOL_VERTICES`: `200`
  - `REFINE_CENTER`: `201`
- Additional face tags (1D):
  - `TOP_FACE`: `110`
  - `RAKE_FACE`: `111`
  - `CLEARANCE_FACE`: `113`
  - `BACK_FACE`: `114`

## Usage

From the repository root:

```powershell
python .\Meshing\generate_rigid_tool_mesh.py `
  --tool-txt .\results\tool_0000000.txt `
  --unit m `
  --out-msh .\Meshing\out\tool.msh `
  --out-report .\Meshing\out\tool_mesh_report.json `
  --out-tool-meta .\Meshing\out\tool_geometry.json
```

If the bundled SDK path does not match your setup, point the script to your Gmsh Python module:

- `--gmsh-lib`: path to the directory containing `gmsh.py` (typically `<gmsh-sdk>/lib`)
- `--gmsh-root`: path to the SDK root (script will use `<root>/lib`)

Environment variable alternatives (path-list separated by `;` on Windows):

- `MFREE_GMSH_LIB`, `GMSH_PYTHON_LIB`
- `MFREE_GMSH_ROOT`, `GMSH_SDK_DIR`

Or reconstruct from benchmark-style parameters:

```powershell
python .\Meshing\generate_rigid_tool_mesh.py `
  --tl-angles `
  --tl-x -0.000410 --tl-y 0.000986074 `
  --length 0.000413176 --height 0.000431 `
  --rake-deg 0.00001 --clearance-deg 11 `
  --fillet-radius 0.00005 `
  --swap-rake-clearance `
  --unit m `
  --out-msh .\Meshing\out\tool.msh `
  --out-report .\Meshing\out\tool_mesh_report.json `
  --out-tool-meta .\Meshing\out\tool_geometry.json
```

## Quality Assurance Output

The script writes a JSON report containing:

- node count, triangle count
- minimum triangle angle (degrees)
- minimum radius-ratio quality (higher is better, 1 is equilateral)
