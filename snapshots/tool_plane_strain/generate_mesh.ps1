python .\Meshing\generate_rigid_tool_mesh.py `
  --tl-angles `
  --tl-x 0 --tl-y -0.001 `
  --length 0.001 --height 0.001 `
  --rake-deg -5 --clearance-deg 5 `
  --fillet-radius 0.00005 `
  --unit m `
  --refine-diameter-mm 0.2 `
  --fine-size-mm 0.002 `
  --transition-length-mm 0.6 `
  --max-size-mm 0.05 `
  --out-msh .\snapshots\tool_plane_strain\meshes\tool_h_0.002mm.msh `
  --out-report .\snapshots\tool_plane_strain\meshes\tool_h_0.002mm_report.json

