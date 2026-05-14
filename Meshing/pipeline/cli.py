import argparse
import json
import os
import sys

from .contracts import MeshRefinementConfig, MeshSourceConfig, MeshSourceType, MeshingJobConfig
from .runner import MeshingPipelineError, run_meshing_job


def _parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(description="Rigid tool meshing pipeline.")
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--tool-txt", type=str)
    src.add_argument("--tl-angles", action="store_true")

    ap.add_argument("--tl-x", type=float, default=None)
    ap.add_argument("--tl-y", type=float, default=None)
    ap.add_argument("--length", type=float, default=None)
    ap.add_argument("--height", type=float, default=None)
    ap.add_argument("--rake-deg", type=float, default=None)
    ap.add_argument("--clearance-deg", type=float, default=None)
    ap.add_argument("--fillet-radius", type=float, default=None)
    ap.add_argument("--swap-rake-clearance", action="store_true")

    ap.add_argument("--unit", type=str, choices=["m", "cm", "mm"], default="m")
    ap.add_argument("--refine-center-x", type=float, default=None)
    ap.add_argument("--refine-center-y", type=float, default=None)
    ap.add_argument("--refine-diameter-mm", type=float, default=0.2)
    ap.add_argument("--fine-size-mm", type=float, default=0.002)
    ap.add_argument("--transition-length-mm", type=float, default=0.6)
    ap.add_argument("--max-size-mm", type=float, default=0.05)

    ap.add_argument("--gmsh-lib", type=str, default=None)
    ap.add_argument("--gmsh-root", type=str, default=None)

    ap.add_argument("--out-msh", type=str, required=True)
    ap.add_argument("--out-geo", type=str, default=None)
    ap.add_argument("--out-report", type=str, default=None)
    ap.add_argument("--out-tool-meta", type=str, default=None)
    ap.add_argument("--json-output", type=str, default=None, help="Optional path to write full pipeline result JSON.")
    return ap.parse_args()


def _source_from_args(args: argparse.Namespace) -> MeshSourceConfig:
    if args.tool_txt:
        return MeshSourceConfig(source_type=MeshSourceType.TOOL_TXT, tool_txt=args.tool_txt)
    swap_env = os.getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE")
    env_swap = False
    if swap_env:
        try:
            env_swap = int(swap_env) != 0
        except ValueError:
            env_swap = False
    return MeshSourceConfig(
        source_type=MeshSourceType.TL_ANGLES,
        tl_x=args.tl_x,
        tl_y=args.tl_y,
        length=args.length,
        height=args.height,
        rake_deg=args.rake_deg,
        clearance_deg=args.clearance_deg,
        fillet_radius=args.fillet_radius,
        swap_rake_clearance=(args.swap_rake_clearance or env_swap),
    )


def _job_from_args(args: argparse.Namespace) -> MeshingJobConfig:
    return MeshingJobConfig(
        source=_source_from_args(args),
        refinement=MeshRefinementConfig(
            unit=args.unit,
            refine_center_x=args.refine_center_x,
            refine_center_y=args.refine_center_y,
            refine_diameter_mm=args.refine_diameter_mm,
            fine_size_mm=args.fine_size_mm,
            transition_length_mm=args.transition_length_mm,
            max_size_mm=args.max_size_mm,
        ),
        out_msh=args.out_msh,
        out_geo=args.out_geo,
        out_report=args.out_report,
        out_tool_meta=args.out_tool_meta,
        gmsh_lib=args.gmsh_lib,
        gmsh_root=args.gmsh_root,
    )


def main() -> int:
    args = _parse_args()
    job = _job_from_args(args)
    try:
        result = run_meshing_job(job)
    except (MeshingPipelineError, ValueError) as exc:
        print(str(exc), file=sys.stderr)
        return 2
    if args.json_output:
        out_path = os.path.abspath(args.json_output)
        parent = os.path.dirname(out_path)
        if parent:
            os.makedirs(parent, exist_ok=True)
        with open(out_path, "w", encoding="utf-8") as f:
            json.dump(result, f, indent=2)
    print(json.dumps({"tool": result["tool"], "mesh": result["mesh"]}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
