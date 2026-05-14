import json
import os
from dataclasses import asdict
from typing import Dict
try:
    from generate_rigid_tool_mesh import (
        Vec2,
        _bbox,
        _gmsh_build_and_mesh,
        _tool_metadata,
        build_tool_from_tl_angles,
        load_tool_from_txt,
    )
except ImportError:
    from Meshing.generate_rigid_tool_mesh import (
        Vec2,
        _bbox,
        _gmsh_build_and_mesh,
        _tool_metadata,
        build_tool_from_tl_angles,
        load_tool_from_txt,
    )

from .contracts import MeshSourceType, MeshingJobConfig
from .inputs import validate_job_config

class MeshingPipelineError(RuntimeError):
    pass


def _resolve_geometry(job: MeshingJobConfig):
    source = job.source
    if source.source_type == MeshSourceType.TOOL_TXT:
        return load_tool_from_txt(source.tool_txt)

    rake_deg = source.rake_deg
    clearance_deg = source.clearance_deg
    if source.swap_rake_clearance:
        rake_deg, clearance_deg = clearance_deg, rake_deg
    return build_tool_from_tl_angles(
        tl=Vec2(source.tl_x, source.tl_y),
        length=source.length,
        height=source.height,
        rake_deg=rake_deg,
        clearance_deg=clearance_deg,
        fillet_radius=source.fillet_radius,
    )


def _resolve_refine_center(job: MeshingJobConfig, geom) -> Vec2:
    refinement = job.refinement
    if refinement.refine_center_x is not None and refinement.refine_center_y is not None:
        return Vec2(refinement.refine_center_x, refinement.refine_center_y)
    if geom.fillet_center is not None:
        return geom.fillet_center
    bb0, bb1 = _bbox(geom.vertices)
    return Vec2(0.5 * (bb0.x + bb1.x), 0.5 * (bb0.y + bb1.y))


def _ensure_parent(path: str) -> None:
    parent = os.path.dirname(os.path.abspath(path))
    if parent:
        os.makedirs(parent, exist_ok=True)


def run_meshing_job(job: MeshingJobConfig) -> Dict:
    try:
        validate_job_config(job)
        geom = _resolve_geometry(job)
        refine_center = _resolve_refine_center(job, geom)
        tool_meta = _tool_metadata(geom)

        _ensure_parent(job.out_msh)
        if job.out_geo:
            _ensure_parent(job.out_geo)
        if job.out_report:
            _ensure_parent(job.out_report)
        if job.out_tool_meta:
            _ensure_parent(job.out_tool_meta)
            with open(job.out_tool_meta, "w", encoding="utf-8") as f:
                json.dump(tool_meta, f, indent=2)

        report = _gmsh_build_and_mesh(
            geom=geom,
            unit=job.refinement.unit,
            refine_center=refine_center,
            refine_diameter_mm=job.refinement.refine_diameter_mm,
            fine_size_mm=job.refinement.fine_size_mm,
            transition_length_mm=job.refinement.transition_length_mm,
            max_size_mm=job.refinement.max_size_mm,
            msh_out=job.out_msh,
            geo_out=job.out_geo,
            report_out=job.out_report,
            gmsh_lib=job.gmsh_lib,
            gmsh_root=job.gmsh_root,
        )
    except Exception as exc:
        raise MeshingPipelineError(f"Meshing pipeline failed: {exc}") from exc

    return {
        "job": asdict(job),
        "tool": tool_meta,
        "mesh": report.get("mesh", {}),
        "refinement_center": {"x": refine_center.x, "y": refine_center.y},
        "outputs": {
            "msh": os.path.abspath(job.out_msh),
            "geo": os.path.abspath(job.out_geo) if job.out_geo else None,
            "report": os.path.abspath(job.out_report) if job.out_report else None,
            "tool_meta": os.path.abspath(job.out_tool_meta) if job.out_tool_meta else None,
        },
    }
