import argparse
import json
import logging
import os
import sys
from datetime import datetime, timezone


def _configure_logging(out_dir: str, verbose: bool) -> logging.Logger:
	os.makedirs(out_dir, exist_ok=True)
	logger = logging.getLogger("pvbatch")
	logger.setLevel(logging.DEBUG)
	logger.handlers.clear()

	fmt = logging.Formatter("%(asctime)s %(levelname)s %(message)s")

	ch = logging.StreamHandler(sys.stdout)
	ch.setLevel(logging.DEBUG if verbose else logging.INFO)
	ch.setFormatter(fmt)
	logger.addHandler(ch)

	fh = logging.FileHandler(os.path.join(out_dir, "paraview_batch_vis.log"), mode="w", encoding="utf-8")
	fh.setLevel(logging.DEBUG)
	fh.setFormatter(fmt)
	logger.addHandler(fh)

	return logger


def _write_report(out_dir: str, report: dict) -> None:
	os.makedirs(out_dir, exist_ok=True)
	path = os.path.join(out_dir, "paraview_batch_vis_report.json")
	with open(path, "w", encoding="utf-8") as f:
		json.dump(report, f, indent=2, sort_keys=True)


def _require_file(path: str, label: str, logger: logging.Logger) -> None:
	if not path:
		raise ValueError(f"missing {label} path")
	if not os.path.isfile(path):
		raise FileNotFoundError(f"{label} not found: {path}")
	logger.info("%s: %s", label, path)


def _try_import_paraview(logger: logging.Logger):
	try:
		from paraview.simple import (  # type: ignore
			CreateView,
			CSVReader,
			GetActiveViewOrCreate,
			GetColorTransferFunction,
			GetOpacityTransferFunction,
			LegacyVTKReader,
			ResetCamera,
			SaveScreenshot,
			Show,
			Text,
			Glyph,
			TableToPoints,
			ColorBy,
			HideScalarBarIfNotNeeded,
			SetActiveView,
			SetActiveSource,
			RenameSource,
			Delete,
		)
	except Exception as e:
		msg = (
			"ParaView Python modules not available.\n"
			"Run this script with pvpython (ParaView).\n"
			"Example:\n"
			'  "C:\\Program Files\\ParaView 5.xx.x\\bin\\pvpython.exe" scripts\\paraview_batch_vis.py --help\n'
		)
		logger.error(msg)
		raise RuntimeError("paraview_import_failed") from e

	return {
		"CreateView": CreateView,
		"CSVReader": CSVReader,
		"GetActiveViewOrCreate": GetActiveViewOrCreate,
		"GetColorTransferFunction": GetColorTransferFunction,
		"GetOpacityTransferFunction": GetOpacityTransferFunction,
		"LegacyVTKReader": LegacyVTKReader,
		"ResetCamera": ResetCamera,
		"SaveScreenshot": SaveScreenshot,
		"Show": Show,
		"Text": Text,
		"Glyph": Glyph,
		"TableToPoints": TableToPoints,
		"ColorBy": ColorBy,
		"HideScalarBarIfNotNeeded": HideScalarBarIfNotNeeded,
		"SetActiveView": SetActiveView,
		"SetActiveSource": SetActiveSource,
		"RenameSource": RenameSource,
		"Delete": Delete,
	}


def _has_point_array(src, name: str) -> bool:
	try:
		info = src.GetPointDataInformation()
		return info.GetArray(name) is not None
	except Exception:
		return False


def _set_window_location(display, loc: str) -> None:
	try:
		display.WindowLocation = loc
		return
	except Exception:
		pass
	mapping = {
		"UpperCenter": "Upper Center",
		"UpperLeft": "Upper Left",
		"UpperRight": "Upper Right",
		"LowerLeft": "Lower Left",
		"LowerRight": "Lower Right",
		"LowerCenter": "Lower Center",
	}
	try:
		display.WindowLocation = mapping.get(loc, loc)
	except Exception:
		return


def _apply_diverging_cool_to_warm(lut, pwf) -> None:
	try:
		lut.ColorSpace = "Diverging"
	except Exception:
		pass
	try:
		lut.NanColor = [1.0, 1.0, 0.0]
	except Exception:
		pass
	try:
		lut.RGBPoints = [
			0.0, 0.23137254902, 0.298039215686, 0.752941176471,
			0.5, 0.865, 0.865, 0.865,
			1.0, 0.705882352941, 0.0156862745098, 0.149019607843,
		]
	except Exception:
		pass
	try:
		pwf.Points = [
			0.0, 1.0, 0.5, 0.0,
			1.0, 1.0, 0.5, 0.0,
		]
	except Exception:
		pass


def _setup_2d_camera(render_view, src) -> None:
	b = src.GetDataInformation().GetBounds()
	xmin, xmax, ymin, ymax, zmin, zmax = b
	cx = 0.5 * (xmin + xmax)
	cy = 0.5 * (ymin + ymax)
	w = max(1e-12, xmax - xmin)
	h = max(1e-12, ymax - ymin)
	scale = 0.55 * max(w, h)
	render_view.CameraParallelProjection = 1
	render_view.CameraFocalPoint = [cx, cy, 0.0]
	render_view.CameraPosition = [cx, cy, 1.0]
	render_view.CameraViewUp = [0.0, 1.0, 0.0]
	render_view.CameraParallelScale = scale


def _edge_nodes_viz(pv, args, logger: logging.Logger) -> str:
	reader = pv["LegacyVTKReader"](FileNames=[args.fe_vtk])
	pv["RenameSource"]("fe_tool", reader)

	view = pv["CreateView"]("RenderView")
	pv["SetActiveView"](view)
	view.ViewSize = [args.width, args.height]
	view.Background = [1.0, 1.0, 1.0]

	disp = pv["Show"](reader, view)
	disp.Representation = "Surface"
	disp.Opacity = 0.35

	if _has_point_array(reader, "temperature"):
		pv["ColorBy"](disp, ("POINTS", "temperature"))
		lut = pv["GetColorTransferFunction"]("temperature")
		pwf = pv["GetOpacityTransferFunction"]("temperature")
		_apply_diverging_cool_to_warm(lut, pwf)
		disp.SetScalarBarVisibility(view, True)
	else:
		pv["ColorBy"](disp, None)

	top_csv = pv["CSVReader"](FileName=[args.top_csv])
	rear_csv = pv["CSVReader"](FileName=[args.rear_csv])
	pv["RenameSource"]("top_nodes_csv", top_csv)
	pv["RenameSource"]("rear_nodes_csv", rear_csv)

	top_pts = pv["TableToPoints"](Input=top_csv)
	top_pts.XColumn = "x"
	top_pts.YColumn = "y"
	top_pts.ZColumn = "z"
	rear_pts = pv["TableToPoints"](Input=rear_csv)
	rear_pts.XColumn = "x"
	rear_pts.YColumn = "y"
	rear_pts.ZColumn = "z"
	pv["RenameSource"]("top_nodes_points", top_pts)
	pv["RenameSource"]("rear_nodes_points", rear_pts)

	top_g = pv["Glyph"](Input=top_pts, GlyphType="Sphere")
	top_g.OrientationArray = ["POINTS", "No orientation array"]
	top_g.ScaleArray = ["POINTS", "No scale array"]
	top_g.ScaleFactor = args.node_glyph_scale
	top_g.GlyphMode = "All Points"

	rear_g = pv["Glyph"](Input=rear_pts, GlyphType="Sphere")
	rear_g.OrientationArray = ["POINTS", "No orientation array"]
	rear_g.ScaleArray = ["POINTS", "No scale array"]
	rear_g.ScaleFactor = args.node_glyph_scale
	rear_g.GlyphMode = "All Points"

	top_disp = pv["Show"](top_g, view)
	top_disp.DiffuseColor = [0.85, 0.1, 0.1]
	top_disp.AmbientColor = [0.85, 0.1, 0.1]
	top_disp.Specular = 0.2

	rear_disp = pv["Show"](rear_g, view)
	rear_disp.DiffuseColor = [0.1, 0.6, 0.1]
	rear_disp.AmbientColor = [0.1, 0.6, 0.1]
	rear_disp.Specular = 0.2

	title = pv["Text"](Text=f"FE Tool Edge Nodes (Top=Red, Rear=Green) | Model={args.model_label}")
	title_disp = pv["Show"](title, view)
	_set_window_location(title_disp, "Upper Center")

	_setup_2d_camera(view, reader)
	pv["ResetCamera"](view)

	out_path = os.path.join(args.out_dir, args.edge_png_name)
	logger.info("writing %s", out_path)
	pv["SaveScreenshot"](out_path, view, ImageResolution=[args.width, args.height])

	pv["Delete"](rear_disp)
	pv["Delete"](top_disp)
	pv["Delete"](rear_g)
	pv["Delete"](top_g)
	pv["Delete"](rear_pts)
	pv["Delete"](top_pts)
	pv["Delete"](rear_csv)
	pv["Delete"](top_csv)
	pv["Delete"](title)
	pv["Delete"](reader)
	pv["Delete"](view)

	return out_path


def _velocity_glyph_viz(pv, args, logger: logging.Logger) -> str:
	reader = pv["LegacyVTKReader"](FileNames=[args.fe_vtk])
	pv["RenameSource"]("fe_tool", reader)

	if not _has_point_array(reader, args.velocity_array):
		raise RuntimeError(f"missing point vector array: {args.velocity_array}")

	view = pv["CreateView"]("RenderView")
	pv["SetActiveView"](view)
	view.ViewSize = [args.width, args.height]
	view.Background = [1.0, 1.0, 1.0]

	disp = pv["Show"](reader, view)
	disp.Representation = "Surface"
	disp.DiffuseColor = [0.6, 0.6, 0.6]
	disp.Opacity = 0.25
	pv["ColorBy"](disp, None)

	g = pv["Glyph"](Input=reader, GlyphType="Arrow")
	g.OrientationArray = ["POINTS", args.velocity_array]
	g.ScaleArray = ["POINTS", "No scale array"]
	g.ScaleFactor = args.velocity_glyph_scale
	g.GlyphMode = "All Points"

	gdisp = pv["Show"](g, view)
	gdisp.DiffuseColor = [0.1, 0.2, 0.8]
	gdisp.AmbientColor = [0.1, 0.2, 0.8]

	speed_ms = args.cutting_speed_m_min / 60.0
	title = pv["Text"](Text=f"FE Tool Velocity Glyphs | {args.cutting_speed_m_min:g} m/min = {speed_ms:.6f} m/s | +X direction")
	title_disp = pv["Show"](title, view)
	_set_window_location(title_disp, "Upper Center")

	_setup_2d_camera(view, reader)
	pv["ResetCamera"](view)

	out_path = os.path.join(args.out_dir, args.velocity_png_name)
	logger.info("writing %s", out_path)
	pv["SaveScreenshot"](out_path, view, ImageResolution=[args.width, args.height])

	pv["Delete"](gdisp)
	pv["Delete"](g)
	pv["Delete"](title)
	pv["Delete"](disp)
	pv["Delete"](reader)
	pv["Delete"](view)

	return out_path


def _build_arg_parser() -> argparse.ArgumentParser:
	p = argparse.ArgumentParser()
	p.add_argument("--fe-vtk", required=True, help="Path to FE tool VTK (e.g., fe_tool_000000.vtk)")
	p.add_argument("--top-csv", default="", help="CSV listing of top edge nodes (fe_bc_top_edge.csv)")
	p.add_argument("--rear-csv", default="", help="CSV listing of rear edge nodes (fe_bc_rear_edge.csv)")
	p.add_argument("--out-dir", required=True, help="Output directory for PNG and report")
	p.add_argument("--width", type=int, default=1920)
	p.add_argument("--height", type=int, default=1080)
	p.add_argument("--node-glyph-scale", type=float, default=2.0e-4)
	p.add_argument("--velocity-array", default="pose_velocity")
	p.add_argument("--velocity-glyph-scale", type=float, default=1.0e-3)
	p.add_argument("--cutting-speed-m-min", type=float, default=100.0)
	p.add_argument("--model-label", default="unknown")
	p.add_argument("--edge-png-name", default="edge_nodes.png")
	p.add_argument("--velocity-png-name", default="velocity_glyphs.png")
	p.add_argument("--skip-edge", action="store_true")
	p.add_argument("--skip-velocity", action="store_true")
	p.add_argument("--verbose", action="store_true")
	return p


def main() -> int:
	parser = _build_arg_parser()
	args = parser.parse_args()
	logger = _configure_logging(args.out_dir, args.verbose)

	report = {
		"timestamp": datetime.now(timezone.utc).isoformat(),
		"status": "failed",
		"inputs": {
			"fe_vtk": args.fe_vtk,
			"top_csv": args.top_csv,
			"rear_csv": args.rear_csv,
		},
		"outputs": {},
		"errors": [],
	}

	try:
		_require_file(args.fe_vtk, "fe_vtk", logger)
		if not args.skip_edge:
			_require_file(args.top_csv, "top_csv", logger)
			_require_file(args.rear_csv, "rear_csv", logger)

		pv = _try_import_paraview(logger)

		if not args.skip_edge:
			report["outputs"]["edge_nodes_png"] = _edge_nodes_viz(pv, args, logger)
		if not args.skip_velocity:
			report["outputs"]["velocity_glyphs_png"] = _velocity_glyph_viz(pv, args, logger)

		report["status"] = "success"
		_write_report(args.out_dir, report)
		logger.info("success")
		return 0
	except Exception as e:
		logger.exception("failure")
		report["errors"].append(str(e))
		try:
			_write_report(args.out_dir, report)
		except Exception:
			pass
		return 2


if __name__ == "__main__":
	raise SystemExit(main())
