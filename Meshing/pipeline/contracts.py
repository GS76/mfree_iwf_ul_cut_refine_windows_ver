from dataclasses import dataclass
from enum import Enum
from typing import Optional


class MeshSourceType(str, Enum):
    TOOL_TXT = "tool_txt"
    TL_ANGLES = "tl_angles"


@dataclass(frozen=True)
class MeshSourceConfig:
    source_type: MeshSourceType
    tool_txt: Optional[str] = None
    tl_x: Optional[float] = None
    tl_y: Optional[float] = None
    length: Optional[float] = None
    height: Optional[float] = None
    rake_deg: Optional[float] = None
    clearance_deg: Optional[float] = None
    fillet_radius: Optional[float] = None
    swap_rake_clearance: bool = False


@dataclass(frozen=True)
class MeshRefinementConfig:
    unit: str = "m"
    refine_center_x: Optional[float] = None
    refine_center_y: Optional[float] = None
    refine_diameter_mm: float = 0.2
    fine_size_mm: float = 0.002
    transition_length_mm: float = 0.6
    max_size_mm: float = 0.05


@dataclass(frozen=True)
class MeshingJobConfig:
    source: MeshSourceConfig
    refinement: MeshRefinementConfig
    out_msh: str
    out_geo: Optional[str] = None
    out_report: Optional[str] = None
    out_tool_meta: Optional[str] = None
    gmsh_lib: Optional[str] = None
    gmsh_root: Optional[str] = None
