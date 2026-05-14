import math
import os
from typing import Iterable

from .contracts import MeshSourceType, MeshingJobConfig


def _require(value: object, name: str) -> None:
    if value is None:
        raise ValueError(f"Missing required field: {name}")


def _require_finite_positive(value: float, name: str) -> None:
    if not math.isfinite(value) or value <= 0.0:
        raise ValueError(f"{name} must be a finite number > 0")


def _require_finite_nonnegative(value: float, name: str) -> None:
    if not math.isfinite(value) or value < 0.0:
        raise ValueError(f"{name} must be a finite number >= 0")


def _ensure_not_empty(path: str, name: str) -> None:
    if not path or not path.strip():
        raise ValueError(f"{name} must be a non-empty path")


def _validate_unit(unit: str) -> None:
    if unit not in {"m", "cm", "mm"}:
        raise ValueError(f"Unsupported unit: {unit}")


def _validate_required_paths(paths: Iterable[tuple[str, str]]) -> None:
    for value, name in paths:
        _ensure_not_empty(value, name)


def _validate_refine_center_pair(job: MeshingJobConfig) -> None:
    has_x = job.refinement.refine_center_x is not None
    has_y = job.refinement.refine_center_y is not None
    if has_x != has_y:
        raise ValueError("refine_center_x and refine_center_y must be provided together")


def validate_job_config(job: MeshingJobConfig) -> None:
    _validate_required_paths([(job.out_msh, "out_msh")])
    _validate_unit(job.refinement.unit)
    _validate_refine_center_pair(job)

    _require_finite_positive(job.refinement.refine_diameter_mm, "refine_diameter_mm")
    _require_finite_positive(job.refinement.fine_size_mm, "fine_size_mm")
    _require_finite_positive(job.refinement.transition_length_mm, "transition_length_mm")
    _require_finite_positive(job.refinement.max_size_mm, "max_size_mm")

    source = job.source
    if source.source_type == MeshSourceType.TOOL_TXT:
        _require(source.tool_txt, "tool_txt")
        _ensure_not_empty(source.tool_txt, "tool_txt")
        if not os.path.isfile(source.tool_txt):
            raise ValueError(f"tool_txt file not found: {source.tool_txt}")
        return

    if source.source_type != MeshSourceType.TL_ANGLES:
        raise ValueError(f"Unsupported source_type: {source.source_type}")

    _require(source.tl_x, "tl_x")
    _require(source.tl_y, "tl_y")
    _require(source.length, "length")
    _require(source.height, "height")
    _require(source.rake_deg, "rake_deg")
    _require(source.clearance_deg, "clearance_deg")
    _require(source.fillet_radius, "fillet_radius")

    if not math.isfinite(source.tl_x) or not math.isfinite(source.tl_y):
        raise ValueError("tl_x/tl_y must be finite numbers")
    _require_finite_positive(source.length, "length")
    _require_finite_positive(source.height, "height")
    if not math.isfinite(source.rake_deg):
        raise ValueError("rake_deg must be a finite number")
    if not math.isfinite(source.clearance_deg) or source.clearance_deg < -90.0 or source.clearance_deg > 180.0:
        raise ValueError("clearance_deg must be a finite number in [-90, 180]")
    _require_finite_nonnegative(source.fillet_radius, "fillet_radius")
