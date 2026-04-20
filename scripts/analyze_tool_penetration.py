import argparse
import glob
import os
import re
from typing import Tuple, List


def _find_vtk_indices(results_dir: str, prefix: str) -> List[int]:
	pat = os.path.join(results_dir, f"{prefix}_*.vtk")
	out = []
	for p in glob.glob(pat):
		base = os.path.basename(p)
		m = re.match(rf"^{re.escape(prefix)}_(\d+)\.vtk$", base)
		if not m:
			continue
		out.append(int(m.group(1)))
	return sorted(set(out))


def _read_vtk_points_bbox(path: str) -> Tuple[float, float, float, float]:
	xmin = float("inf")
	xmax = float("-inf")
	ymin = float("inf")
	ymax = float("-inf")
	with open(path, "r", encoding="utf-8", errors="replace") as f:
		in_points = False
		remaining = 0
		for line in f:
			if not in_points:
				if line.startswith("POINTS "):
					parts = line.split()
					if len(parts) >= 3:
						remaining = int(parts[1])
						in_points = True
					continue
			else:
				if remaining <= 0:
					break
				parts = line.strip().split()
				if len(parts) < 2:
					continue
				x = float(parts[0])
				y = float(parts[1])
				if x < xmin:
					xmin = x
				if x > xmax:
					xmax = x
				if y < ymin:
					ymin = y
				if y > ymax:
					ymax = y
				remaining -= 1
	if xmin == float("inf"):
		return 0.0, 0.0, 0.0, 0.0
	return xmin, xmax, ymin, ymax


def analyze(results_dir: str, frames: List[int]) -> List[dict]:
	rows = []
	for i in frames:
		wp = os.path.join(results_dir, f"out_{i:06d}.vtk")
		ft = os.path.join(results_dir, f"fe_tool_{i:06d}.vtk")
		if not os.path.isfile(wp) or not os.path.isfile(ft):
			continue
		wp_xmin, wp_xmax, wp_ymin, wp_ymax = _read_vtk_points_bbox(wp)
		ft_xmin, ft_xmax, ft_ymin, ft_ymax = _read_vtk_points_bbox(ft)
		penetration = wp_ymax - ft_ymin
		rows.append(
			{
				"frame": i,
				"wp_ymax": wp_ymax,
				"ft_ymin": ft_ymin,
				"penetration_m": penetration,
				"wp_bbox": (wp_xmin, wp_xmax, wp_ymin, wp_ymax),
				"ft_bbox": (ft_xmin, ft_xmax, ft_ymin, ft_ymax),
			}
		)
	return rows


def main() -> int:
	ap = argparse.ArgumentParser()
	ap.add_argument("--results-dir", required=True)
	ap.add_argument("--frames", default="auto", help="comma-separated list or 'auto' (first,mid,last)")
	args = ap.parse_args()

	indices = _find_vtk_indices(args.results_dir, "out")
	if not indices:
		raise SystemExit(f"no out_*.vtk found in {args.results_dir}")

	if args.frames.strip().lower() == "auto":
		first = indices[0]
		last = indices[-1]
		mid = indices[len(indices) // 2]
		frames = sorted(set([first, mid, last]))
	else:
		frames = []
		for p in args.frames.split(","):
			p = p.strip()
			if p:
				frames.append(int(p))

	rows = analyze(args.results_dir, frames)
	print("results_dir,frame,wp_ymax,ft_ymin,penetration_m")
	for r in rows:
		print(f"{args.results_dir},{r['frame']},{r['wp_ymax']:.15e},{r['ft_ymin']:.15e},{r['penetration_m']:.15e}")
	return 0


if __name__ == "__main__":
	raise SystemExit(main())

