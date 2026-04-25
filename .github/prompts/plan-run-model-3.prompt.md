## Plan: Run Model 3 Simulation with FE Tool Mesh

**TL;DR** - Execute the mfree_iwf executable with model 3 (dynamic multi-resolution cutting) using the newly generated FE tool mesh. The mesh is already in place, so run the command from the build directory with appropriate environment variables for preprocessing and results.

**Steps**
1. Navigate to the build directory where the executable is located.
2. Set environment variables for preprocessing mode, results directory, and FE tool mesh path.
3. Run the executable with `-m 3` to execute model 3 (cutting_ref_multi_resol_dynamic).
4. Monitor the output for successful mesh loading and simulation progress.
5. Check the results directory for generated VTK files and logs.

**Relevant files**
- `build/mfree_iwf.exe` — The main executable to run the simulation.
- `snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh` — The FE tool mesh file (already generated).

**Verification**
1. Confirm the executable runs without errors and loads the mesh successfully.
2. Check that VTK output files are generated in the results directory.
3. Verify the log shows proper FE tool attachment and simulation completion.

**Decisions**
- Using preprocessing mode first to validate setup before full simulation.
- FE tool mesh path is set to the generated file location.
- Model 3 selected as it uses dynamic multi-resolution refinement with the FE tool.