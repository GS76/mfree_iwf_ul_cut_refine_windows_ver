import argparse
import json
import math
import os
import sys
from dataclasses import asdict, dataclass
from typing import Dict, List, Optional, Sequence, Tuple


@dataclass(frozen=True)
class Vec2:
    x: float
    y: float


@dataclass(frozen=True)
class ToolGeometry:
    vertices: List[Vec2]
    fillet_center: Optional[Vec2]
    fillet_radius: float
    fillet_t1: Optional[float]
    fillet_t2: Optional[float]
    fillet_vertex_start_index: Optional[int]


def _unit_mm_to_model(unit: str) -> float:
    if unit == "m":
        return 1e-3
    if unit == "cm":
        return 1e-1
    if unit == "mm":
        return 1.0
    raise ValueError(f"Unsupported unit: {unit}")


def _vec2_add(a: Vec2, b: Vec2) -> Vec2:
    return Vec2(a.x + b.x, a.y + b.y)


def _vec2_sub(a: Vec2, b: Vec2) -> Vec2:
    return Vec2(a.x - b.x, a.y - b.y)


def _vec2_dot(a: Vec2, b: Vec2) -> float:
    return a.x * b.x + a.y * b.y


def _vec2_norm(a: Vec2) -> float:
    return math.hypot(a.x, a.y)


def _vec2_normalize(a: Vec2) -> Vec2:
    n = _vec2_norm(a)
    if n <= 0.0:
        return Vec2(0.0, 0.0)
    return Vec2(a.x / n, a.y / n)


def _bbox(points: Sequence[Vec2]) -> Tuple[Vec2, Vec2]:
    xs = [p.x for p in points]
    ys = [p.y for p in points]
    return Vec2(min(xs), min(ys)), Vec2(max(xs), max(ys))


def _angle_deg_from_vec(v: Vec2) -> float:
    return math.degrees(math.atan2(v.y, v.x))


def load_tool_from_txt(path: str) -> ToolGeometry:
    with open(path, "r", encoding="utf-8") as f:
        raw = [ln.strip() for ln in f.readlines() if ln.strip()]

    n = int(raw[0])
    if n < 3:
        raise ValueError("Tool file must contain at least 3 segment points")

    verts: List[Vec2] = []
    for i in range(n):
        parts = raw[1 + i].split()
        if len(parts) < 2:
            raise ValueError("Invalid point line in tool file")
        verts.append(Vec2(float(parts[0]), float(parts[1])))

    fillet_center = None
    fillet_radius = 0.0
    fillet_t1 = None
    fillet_t2 = None
    fillet_vertex_start_index = None

    if len(raw) >= 1 + n + 1:
        parts = raw[1 + n].split()
        if len(parts) >= 5:
            fillet_center = Vec2(float(parts[0]), float(parts[1]))
            fillet_radius = float(parts[2])
            fillet_t1 = float(parts[3])
            fillet_t2 = float(parts[4])
            if n >= 5:
                fillet_vertex_start_index = 2

    return ToolGeometry(
        vertices=verts,
        fillet_center=fillet_center,
        fillet_radius=fillet_radius,
        fillet_t1=fillet_t1,
        fillet_t2=fillet_t2,
        fillet_vertex_start_index=fillet_vertex_start_index,
    )


def build_tool_from_tl_angles(
    tl: Vec2,
    length: float,
    height: float,
    rake_deg: float,
    clearance_deg: float,
    fillet_radius: float,
) -> ToolGeometry:
    tr = Vec2(tl.x + length, tl.y)
    bl = Vec2(tl.x, tl.y - height)

    alpha_rake = math.radians(rake_deg)
    alpha_free = math.radians(180.0 - 90.0 - clearance_deg)

    down = Vec2(0.0, -1.0)
    rot_rake = ((math.cos(alpha_rake), -math.sin(alpha_rake)), (math.sin(alpha_rake), math.cos(alpha_rake)))
    rot_free = ((math.cos(alpha_free), -math.sin(alpha_free)), (math.sin(alpha_free), math.cos(alpha_free)))

    trc = Vec2(tr.x + down.x * rot_rake[0][0] + down.y * rot_rake[0][1], tr.y + down.x * rot_rake[1][0] + down.y * rot_rake[1][1])
    blc = Vec2(bl.x + down.x * rot_free[0][0] + down.y * rot_free[0][1], bl.y + down.x * rot_free[1][0] + down.y * rot_free[1][1])

    def line_from_points(p1: Vec2, p2: Vec2) -> Tuple[float, float, bool]:
        if abs(p1.x - p2.x) < 1e-16:
            return (float("inf"), p1.x, True)
        a = (p1.y - p2.y) / (p1.x - p2.x)
        b = p1.y - a * p1.x
        return (a, b, False)

    def line_intersect(l1: Tuple[float, float, bool], l2: Tuple[float, float, bool]) -> Vec2:
        a1, b1, v1 = l1
        a2, b2, v2 = l2
        if v1 and v2:
            return Vec2(float("inf"), float("inf"))
        if v1:
            x = b1
            y = a2 * x + b2
            return Vec2(x, y)
        if v2:
            x = b2
            y = a1 * x + b1
            return Vec2(x, y)
        if abs(a1 - a2) < 1e-16:
            return Vec2(float("inf"), float("inf"))
        x = (b2 - b1) / (a1 - a2)
        y = a1 * x + b1
        return Vec2(x, y)

    l1 = line_from_points(tr, trc)
    l2 = line_from_points(bl, blc)
    br = line_intersect(l1, l2)

    verts = [tl, tr, br, bl]

    if fillet_radius <= 0.0 or not math.isfinite(fillet_radius):
        return ToolGeometry(vertices=verts, fillet_center=None, fillet_radius=0.0, fillet_t1=None, fillet_t2=None, fillet_vertex_start_index=None)

    nt = _vec2_normalize(_vec2_sub(tr, br))
    nl = _vec2_normalize(_vec2_sub(bl, br))
    nm = _vec2_normalize(Vec2(0.5 * (nt.x + nl.x), 0.5 * (nt.y + nl.y)))

    lm = line_from_points(br, _vec2_add(br, nm))
    ltr = line_from_points(tr, br)

    def solve_quad(a: float, b: float, c: float) -> Tuple[float, float]:
        disc = b * b - 4.0 * a * c
        if disc < 0.0:
            return (float("nan"), float("nan"))
        s = math.sqrt(disc)
        return ((-b + s) / (2.0 * a), (-b - s) / (2.0 * a))

    A0, B0, V0 = lm
    a, b, v = ltr
    if V0 or v:
        raise ValueError("Unexpected vertical line in fillet construction")
    A = a - A0
    B = b - B0
    C = fillet_radius * math.sqrt(a * a + 1.0)
    x1, x2 = solve_quad(A * A, 2.0 * A * B, B * B - C * C)
    xm = min(x1, x2)
    p = Vec2(xm, A0 * xm + B0)

    def closest_point_on_line(line: Tuple[float, float, bool], xq: Vec2) -> Vec2:
        aa, bb, vv = line
        if vv:
            return Vec2(bb, xq.y)
        A = aa
        B = -1.0
        Cc = bb
        px = (B * (B * xq.x - A * xq.y) - A * Cc) / (A * A + B * B)
        py = (A * (-B * xq.x + A * xq.y) - B * Cc) / (A * A + B * B)
        return Vec2(px, py)

    trc2 = closest_point_on_line(ltr, p)
    lbl = line_from_points(bl, br)
    blc2 = closest_point_on_line(lbl, p)

    def myatan2(y: float, x: float) -> float:
        t = math.atan2(y, x)
        return t if t > 0.0 else t + 2.0 * math.pi

    t1 = myatan2(p.y - trc2.y, p.x - trc2.x)
    t2 = myatan2(p.y - blc2.y, p.x - blc2.x)

    verts2 = [tl, tr, trc2, blc2, bl]
    return ToolGeometry(vertices=verts2, fillet_center=p, fillet_radius=fillet_radius, fillet_t1=t1, fillet_t2=t2, fillet_vertex_start_index=2)


def _triangle_angles_deg(a: Vec2, b: Vec2, c: Vec2) -> Tuple[float, float, float]:
    ab = _vec2_sub(b, a)
    ac = _vec2_sub(c, a)
    bc = _vec2_sub(c, b)
    ba = _vec2_sub(a, b)
    ca = _vec2_sub(a, c)
    cb = _vec2_sub(b, c)

    def angle(u: Vec2, v: Vec2) -> float:
        nu = _vec2_norm(u)
        nv = _vec2_norm(v)
        if nu <= 0.0 or nv <= 0.0:
            return 0.0
        x = max(-1.0, min(1.0, _vec2_dot(u, v) / (nu * nv)))
        return math.degrees(math.acos(x))

    return (angle(ab, ac), angle(ba, bc), angle(ca, cb))


def _triangle_quality_radius_ratio(a: Vec2, b: Vec2, c: Vec2) -> float:
    la = _vec2_norm(_vec2_sub(b, c))
    lb = _vec2_norm(_vec2_sub(a, c))
    lc = _vec2_norm(_vec2_sub(a, b))
    s = 0.5 * (la + lb + lc)
    area2 = abs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x))
    area = 0.5 * area2
    if area <= 0.0:
        return 0.0
    r_in = area / s
    r_circ = (la * lb * lc) / (4.0 * area)
    if r_circ <= 0.0:
        return 0.0
    return (2.0 * r_in) / r_circ


def _split_env_paths(value: Optional[str]) -> List[str]:
    if not value:
        return []
    parts = [p.strip() for p in value.split(os.pathsep)]
    return [p for p in parts if p]


def _ensure_gmsh_importable(script_dir: str, gmsh_lib: Optional[str], gmsh_root: Optional[str]) -> str:
    candidates: List[str] = []

    candidates.extend(_split_env_paths(gmsh_lib))
    candidates.extend(_split_env_paths(os.getenv("MFREE_GMSH_LIB")))
    candidates.extend(_split_env_paths(os.getenv("GMSH_PYTHON_LIB")))

    roots: List[str] = []
    roots.extend(_split_env_paths(gmsh_root))
    roots.extend(_split_env_paths(os.getenv("MFREE_GMSH_ROOT")))
    roots.extend(_split_env_paths(os.getenv("GMSH_SDK_DIR")))
    for r in roots:
        candidates.append(os.path.join(r, "lib"))

    candidates.append(os.path.join(script_dir, "gmsh-4.15.2-Windows64-sdk", "lib"))
    candidates.append(os.path.join(script_dir, "gmsh-4.15.2-Windows64", "lib"))

    sdk_lib = next((p for p in candidates if os.path.isdir(p) and os.path.isfile(os.path.join(p, "gmsh.py"))), None)
    if not sdk_lib:
        hint = (
            "Please ensure GMSH_SDK_DIR or MFREE_GMSH_ROOT is set, or install the Gmsh SDK in "
            "Meshing/gmsh-4.15.2-Windows64-sdk/, or pass --gmsh-root/--gmsh-lib."
        )
        raise ImportError(f"Gmsh Python module not found. Tried: {candidates}. {hint}")
    if sdk_lib not in sys.path:
        sys.path.insert(0, sdk_lib)
    return sdk_lib


def _gmsh_build_and_mesh(
    geom: ToolGeometry,
    unit: str,
    refine_center: Vec2,
    refine_diameter_mm: float,
    fine_size_mm: float,
    transition_length_mm: float,
    max_size_mm: float,
    msh_out: str,
    geo_out: Optional[str],
    report_out: Optional[str],
    gmsh_lib: Optional[str],
    gmsh_root: Optional[str],
) -> Dict:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    _ensure_gmsh_importable(script_dir, gmsh_lib, gmsh_root)

    import gmsh  # type: ignore

    mm_to_u = _unit_mm_to_model(unit)
    refine_radius = 0.5 * refine_diameter_mm * mm_to_u
    fine_size = fine_size_mm * mm_to_u
    max_size = max_size_mm * mm_to_u
    transition = transition_length_mm * mm_to_u

    if refine_radius <= 0.0:
        raise ValueError("Refine diameter must be positive")
    if fine_size <= 0.0:
        raise ValueError("Fine size must be positive")
    if max_size <= 0.0:
        raise ValueError("Max size must be positive")
    if transition <= 0.0:
        raise ValueError("Transition length must be positive")

    gmsh.initialize([])
    gmsh.model.add("rigid_tool")

    gmsh.option.setNumber("General.Terminal", 1)
    gmsh.option.setNumber("Mesh.MshFileVersion", 2.2)
    gmsh.option.setNumber("Mesh.Binary", 0)
    gmsh.option.setNumber("Mesh.Algorithm", 6)
    gmsh.option.setNumber("Mesh.MeshSizeMin", fine_size)
    gmsh.option.setNumber("Mesh.MeshSizeMax", max_size)

    minp, maxp = _bbox(geom.vertices)
    diag = math.hypot(maxp.x - minp.x, maxp.y - minp.y)
    base_lc = max(fine_size, min(max_size, 0.1 * diag))

    p_tags: List[int] = []
    for v in geom.vertices:
        p_tags.append(gmsh.model.occ.addPoint(v.x, v.y, 0.0, base_lc))

    center_tag = gmsh.model.occ.addPoint(refine_center.x, refine_center.y, 0.0, fine_size)

    arc_curve_tag: Optional[int] = None
    curve_tags: List[int] = []

    n = len(p_tags)
    fillet_idx = geom.fillet_vertex_start_index

    def add_line(i0: int, i1: int) -> int:
        return gmsh.model.occ.addLine(p_tags[i0], p_tags[i1])

    if fillet_idx is not None and geom.fillet_center is not None and n >= fillet_idx + 2:
        for i in range(n):
            i0 = i
            i1 = (i + 1) % n
            if i0 == fillet_idx:
                arc_curve_tag = gmsh.model.occ.addCircleArc(p_tags[i0], center_tag, p_tags[i1])
                curve_tags.append(arc_curve_tag)
            else:
                curve_tags.append(add_line(i0, i1))
    else:
        for i in range(n):
            curve_tags.append(add_line(i, (i + 1) % n))

    loop = gmsh.model.occ.addCurveLoop(curve_tags)
    surf = gmsh.model.occ.addPlaneSurface([loop])
    gmsh.model.occ.synchronize()

    pg_domain = gmsh.model.addPhysicalGroup(2, [surf], tag=1)
    gmsh.model.setPhysicalName(2, pg_domain, "TOOL_DOMAIN")

    pg_all_bnd = gmsh.model.addPhysicalGroup(1, curve_tags, tag=100)
    gmsh.model.setPhysicalName(1, pg_all_bnd, "TOOL_BOUNDARY")

    pg_vertices = gmsh.model.addPhysicalGroup(0, p_tags, tag=200)
    gmsh.model.setPhysicalName(0, pg_vertices, "TOOL_VERTICES")
    pg_refine_center = gmsh.model.addPhysicalGroup(0, [center_tag], tag=201)
    gmsh.model.setPhysicalName(0, pg_refine_center, "REFINE_CENTER")

    boundary_groups: Dict[str, Optional[int]] = {
        "TOOL_DOMAIN": 1,
        "TOOL_BOUNDARY": 100,
        "TOOL_VERTICES": 200,
        "REFINE_CENTER": 201,
        "TOP_FACE": None,
        "RAKE_FACE": None,
        "CUTTING_EDGE": None,
        "CLEARANCE_FACE": None,
        "BACK_FACE": None,
    }

    if len(curve_tags) == 5 and arc_curve_tag is not None:
        boundary_groups["TOP_FACE"] = 110
        boundary_groups["RAKE_FACE"] = 111
        boundary_groups["CUTTING_EDGE"] = 101
        boundary_groups["CLEARANCE_FACE"] = 113
        boundary_groups["BACK_FACE"] = 114

        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [curve_tags[0]], tag=110), "TOP_FACE")
        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [curve_tags[1]], tag=111), "RAKE_FACE")
        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [arc_curve_tag], tag=101), "CUTTING_EDGE")
        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [curve_tags[3]], tag=113), "CLEARANCE_FACE")
        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [curve_tags[4]], tag=114), "BACK_FACE")
    elif len(curve_tags) == 4:
        boundary_groups["TOP_FACE"] = 110
        boundary_groups["RAKE_FACE"] = 111
        boundary_groups["CLEARANCE_FACE"] = 113
        boundary_groups["BACK_FACE"] = 114

        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [curve_tags[0]], tag=110), "TOP_FACE")
        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [curve_tags[1]], tag=111), "RAKE_FACE")
        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [curve_tags[2]], tag=113), "CLEARANCE_FACE")
        gmsh.model.setPhysicalName(1, gmsh.model.addPhysicalGroup(1, [curve_tags[3]], tag=114), "BACK_FACE")

    field_dist = gmsh.model.mesh.field.add("Distance")
    gmsh.model.mesh.field.setNumbers(field_dist, "NodesList", [center_tag])

    field_thresh = gmsh.model.mesh.field.add("Threshold")
    gmsh.model.mesh.field.setNumber(field_thresh, "InField", field_dist)
    gmsh.model.mesh.field.setNumber(field_thresh, "DistMin", refine_radius)
    gmsh.model.mesh.field.setNumber(field_thresh, "DistMax", refine_radius + transition)
    gmsh.model.mesh.field.setNumber(field_thresh, "SizeMin", fine_size)
    gmsh.model.mesh.field.setNumber(field_thresh, "SizeMax", max_size)

    gmsh.model.mesh.field.setAsBackgroundMesh(field_thresh)

    gmsh.model.mesh.generate(2)
    gmsh.model.mesh.optimize("Netgen")

    if geo_out:
        gmsh.write(geo_out)
    gmsh.write(msh_out)

    node_tags, node_coords, _ = gmsh.model.mesh.getNodes()
    nodes: Dict[int, Vec2] = {}
    for i, tag in enumerate(node_tags):
        x = node_coords[3 * i + 0]
        y = node_coords[3 * i + 1]
        nodes[int(tag)] = Vec2(x, y)

    types, elem_tags, elem_node_tags = gmsh.model.mesh.getElements(dim=2)
    tri_count = 0
    min_angle = 180.0
    min_quality = 1.0
    for t, tags, conn in zip(types, elem_tags, elem_node_tags):
        if int(t) != 2:
            continue
        tri_count += len(tags)
        for k in range(len(tags)):
            n0 = int(conn[3 * k + 0])
            n1 = int(conn[3 * k + 1])
            n2 = int(conn[3 * k + 2])
            a = nodes[n0]
            b = nodes[n1]
            c = nodes[n2]
            angs = _triangle_angles_deg(a, b, c)
            min_angle = min(min_angle, angs[0], angs[1], angs[2])
            q = _triangle_quality_radius_ratio(a, b, c)
            min_quality = min(min_quality, q)

    report = {
        "unit": unit,
        "tool_bbox": {"min": asdict(minp), "max": asdict(maxp)},
        "mesh": {
            "nodes": int(len(node_tags)),
            "triangles": int(tri_count),
            "min_angle_deg": float(min_angle),
            "min_quality_radius_ratio": float(min_quality),
        },
        "refinement": {
            "center": asdict(refine_center),
            "diameter_mm": float(refine_diameter_mm),
            "fine_size_mm": float(fine_size_mm),
            "transition_length_mm": float(transition_length_mm),
            "max_size_mm": float(max_size_mm),
        },
        "physical_groups": boundary_groups,
    }

    if report_out:
        with open(report_out, "w", encoding="utf-8") as f:
            json.dump(report, f, indent=2)

    gmsh.finalize()
    return report


def _tool_metadata(geom: ToolGeometry) -> Dict:
    bb0, bb1 = _bbox(geom.vertices)
    length = bb1.x - bb0.x
    height = bb1.y - bb0.y
    width = 1.0

    rake_angle_deg = None
    clearance_angle_deg = None
    if len(geom.vertices) >= 4:
        v = _vec2_sub(geom.vertices[2], geom.vertices[1])
        rake_angle_deg = _angle_deg_from_vec(v)
        v2 = _vec2_sub(geom.vertices[3], geom.vertices[2])
        clearance_angle_deg = _angle_deg_from_vec(v2)

    edge_arc = None
    if geom.fillet_center is not None and geom.fillet_vertex_start_index is not None:
        i0 = geom.fillet_vertex_start_index
        i1 = (i0 + 1) % len(geom.vertices)
        edge_arc = {
            "start": asdict(geom.vertices[i0]),
            "end": asdict(geom.vertices[i1]),
        }

    return {
        "bbox": {"min": asdict(bb0), "max": asdict(bb1)},
        "dimensions": {"length": float(length), "height": float(height), "width": float(width)},
        "vertices": [asdict(v) for v in geom.vertices],
        "edge": {
            "fillet_center": asdict(geom.fillet_center) if geom.fillet_center else None,
            "fillet_radius": float(geom.fillet_radius),
            "arc": edge_arc,
        },
        "orientation": {"rake_face_dir_deg": rake_angle_deg, "clearance_face_dir_deg": clearance_angle_deg},
        "num_vertices": int(len(geom.vertices)),
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--tool-txt", type=str, help="Path to tool_*.txt generated by tool::print(step)")
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

    args = ap.parse_args()

    if args.tool_txt:
        geom = load_tool_from_txt(args.tool_txt)
    else:
        missing = []
        if args.tl_x is None:
            missing.append("--tl-x")
        if args.tl_y is None:
            missing.append("--tl-y")
        if args.length is None:
            missing.append("--length")
        if args.height is None:
            missing.append("--height")
        if args.rake_deg is None:
            missing.append("--rake-deg")
        if args.clearance_deg is None:
            missing.append("--clearance-deg")
        if args.fillet_radius is None:
            missing.append("--fillet-radius")
        if missing:
            raise SystemExit(f"--tl-angles requires: {', '.join(missing)}")

        if not math.isfinite(args.tl_x) or not math.isfinite(args.tl_y):
            raise SystemExit("--tl-x/--tl-y must be finite numbers")
        if not math.isfinite(args.length) or args.length <= 0.0:
            raise SystemExit("--length must be a finite number > 0")
        if not math.isfinite(args.height) or args.height <= 0.0:
            raise SystemExit("--height must be a finite number > 0")
        if not math.isfinite(args.rake_deg):
            raise SystemExit("--rake-deg must be a finite number")
        if not math.isfinite(args.clearance_deg) or args.clearance_deg < -90.0 or args.clearance_deg > 180.0:
            raise SystemExit("--clearance-deg must be a finite number in [-90, 180]")
        if args.clearance_deg < 0.0 or args.clearance_deg > 30.0:
            print(
                f"WARNING: unusual clearance angle {args.clearance_deg} deg (typical range is 0..15 deg)",
                file=sys.stderr,
            )
        if not math.isfinite(args.fillet_radius) or args.fillet_radius < 0.0:
            raise SystemExit("--fillet-radius must be a finite number >= 0")

        rake_deg = args.rake_deg
        clearance_deg = args.clearance_deg
        swap_env = os.getenv("MFREE_SWAP_TOOL_RAKE_CLEARANCE")
        if args.swap_rake_clearance or (swap_env and int(swap_env) != 0):
            rake_deg, clearance_deg = clearance_deg, rake_deg

        geom = build_tool_from_tl_angles(
            tl=Vec2(args.tl_x, args.tl_y),
            length=args.length,
            height=args.height,
            rake_deg=rake_deg,
            clearance_deg=clearance_deg,
            fillet_radius=args.fillet_radius,
        )

    if args.refine_center_x is not None and args.refine_center_y is not None:
        refine_center = Vec2(args.refine_center_x, args.refine_center_y)
    elif geom.fillet_center is not None:
        refine_center = geom.fillet_center
    else:
        bb0, bb1 = _bbox(geom.vertices)
        refine_center = Vec2(0.5 * (bb0.x + bb1.x), 0.5 * (bb0.y + bb1.y))

    tool_meta = _tool_metadata(geom)
    os.makedirs(os.path.dirname(os.path.abspath(args.out_msh)), exist_ok=True)
    if args.out_geo:
        os.makedirs(os.path.dirname(os.path.abspath(args.out_geo)), exist_ok=True)
    if args.out_report:
        os.makedirs(os.path.dirname(os.path.abspath(args.out_report)), exist_ok=True)
    if args.out_tool_meta:
        os.makedirs(os.path.dirname(os.path.abspath(args.out_tool_meta)), exist_ok=True)
        with open(args.out_tool_meta, "w", encoding="utf-8") as f:
            json.dump(tool_meta, f, indent=2)

    report = _gmsh_build_and_mesh(
        geom=geom,
        unit=args.unit,
        refine_center=refine_center,
        refine_diameter_mm=args.refine_diameter_mm,
        fine_size_mm=args.fine_size_mm,
        transition_length_mm=args.transition_length_mm,
        max_size_mm=args.max_size_mm,
        msh_out=args.out_msh,
        geo_out=args.out_geo,
        report_out=args.out_report,
        gmsh_lib=args.gmsh_lib,
        gmsh_root=args.gmsh_root,
    )

    print(json.dumps({"tool": tool_meta, "mesh": report["mesh"]}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

