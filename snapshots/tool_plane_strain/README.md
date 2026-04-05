# Tool Plane-Strain Thermo-Mechanical Snapshot

This snapshot uses the in-code 2D FE tool (triangles + plane-strain thermoelasticity) coupled to the SPH workpiece through mapped contact forces and mapped thermal power.

Outputs are intended for ParaView inspection (`results/*.vtk`) and a JSON setup report (`results/precheck.json`).

## Mesh

Generate a 2D triangular tool mesh (Gmsh v2 ASCII `.msh`) using:

- [generate_mesh.ps1](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/snapshots/tool_plane_strain/generate_mesh.ps1)

## Run (Setup Snapshot Only)

Run the coupled pre-check (no time integration) using:

- [run_precheck.ps1](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/snapshots/tool_plane_strain/run_precheck.ps1)

This produces:

- `results/out_000000.vtk` (SPH particles with `density`, `num_neighbors`, `fixed`, etc.)
- `results/fe_tool_000000.vtk` (FE tool with `temperature`, `power`, `nodal_force`, deformed coordinates)
- `results/tool_000000.vtk` (contact polygon surface triangulation)
- `results/precheck.json` (mesh stats, overlap checks, mapped force/power balance, contact residuals)

## Coupling Controls (Environment)

- `MFREE_FE_TOOL_MSH`: path to the mesh file
- `MFREE_USE_FE_TOOL_FOR_CONTACT=1`: use FE tool boundary as the contact tool geometry
- `MFREE_DEFORMABLE_FE_TOOL=1`: enable deformable contact iterations (mapped forces → plane strain FE solve)
- `MFREE_DEFORMABLE_TOOL_TOL`: relative tolerance for mapped nodal force and nodal power residuals (e.g. `0.01`)
- `MFREE_DEFORMABLE_TOOL_MAX_ITERS`: maximum contact iterations
- `MFREE_DEFORMABLE_TOOL_RELAX`: displacement under-relaxation in the contact iteration (0..1)

Tool material overrides:

- `MFREE_FE_TOOL_RHO`, `MFREE_FE_TOOL_CP`, `MFREE_FE_TOOL_K`
- `MFREE_FE_TOOL_E`, `MFREE_FE_TOOL_NU`, `MFREE_FE_TOOL_ALPHA`

Boundary constraints:

- `MFREE_FE_TOOL_FIX_TAGS`: optional `;`-separated physical tags to fully fix (x,y) DOFs on those boundary edges
- If unset, nodes on the maximum-x boundary are fixed (x,y) using an auto tolerance `MFREE_FE_TOOL_FIX_X_TOL`

