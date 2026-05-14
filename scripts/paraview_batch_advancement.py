import argparse
import json
import logging
import os
import sys
from datetime import datetime, timezone


def _configure_logging(out_dir: str, verbose: bool) -> logging.Logger:
	os.makedirs(out_dir, exist_ok=True)
	logger = logging.getLogger("pvadv")
	logger.setLevel(logging.DEBUG)
	logger.handlers.clear()

	fmt = logging.Formatter("%(asctime)s %(levelname)s %(message)s")

	ch = logging.StreamHandler(sys.stdout)
	ch.setLevel(logging.DEBUG if verbose else logging.INFO)
	ch.setFormatter(fmt)
	logger.addHandler(ch)

	fh = logging.FileHandler(os.path.join(out_dir, "paraview_batch_advancement.log"), mode="w", encoding="utf-8")
	fh.setLevel(logging.DEBUG)
	fh.setFormatter(fmt)
	logger.addHandler(fh)

	return logger


def _write_report(out_dir: str, report: dict) -> None:
	os.makedirs(out_dir, exist_ok=True)
	path = os.path.join(out_dir, "paraview_batch_advancement_report.json")
	with open(path, "w", encoding="utf-8") as f:
		json.dump(report, f, indent=2, sort_keys=True)


def _require_file(path: str, label: str) -> None:
	if not path:
		raise ValueError(f"missing {label} path")
	if not os.path.isfile(path):
		raise FileNotFoundError(f"{label} not found: {path}")


def _try_import_paraview():
	from paraview.simple import (  # type: ignore
		CreateView,
		GetColorTransferFunction,
		GetOpacityTransferFunction,
		LegacyVTKReader,
		ResetCamera,
		SaveScreenshot,
		Show,
		Text,
		ColorBy,
		Delete,
		RenameSource,
	)
	return {
		"CreateView": CreateView,
		"GetColorTransferFunction": GetColorTransferFunction,
		"GetOpacityTransferFunction": GetOpacityTransferFunction,
		"LegacyVTKReader": LegacyVTKReader,
		"ResetCamera": ResetCamera,
		"SaveScreenshot": SaveScreenshot,
		"Show": Show,
		"Text": Text,
		"ColorBy": ColorBy,
		"Delete": Delete,
		"RenameSource": RenameSource,
	}


def _has_point_array(src, name: str) -> bool:
	info = src.GetPointDataInformation()
	return info.GetArray(name) is not None


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


def _setup_2d_camera(render_view, src_a, src_b=None) -> None:
	b = src_a.GetDataInformation().GetBounds()
	xmin, xmax, ymin, ymax, zmin, zmax = b
	if src_b is not None:
		b2 = src_b.GetDataInformation().GetBounds()
		xmin = min(xmin, b2[0])
		xmax = max(xmax, b2[1])
		ymin = min(ymin, b2[2])
		ymax = max(ymax, b2[3])
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


def _parse_steps(s: str):
	out = []
	for part in (s or "").split(","):
		part = part.strip()
		if not part:
			continue
		out.append(int(part))
	return out


def main() -> int:
	ap = argparse.ArgumentParser()
	ap.add_argument("--wp-vtk-pattern", required=True, help="Workpiece pattern, e.g. results/.../out_%06d.vtk")
	ap.add_argument("--fe-vtk-pattern", required=True, help="FE tool pattern, e.g. results/.../fe_tool_%06d.vtk")
	ap.add_argument("--steps", default="0", help="Comma-separated print indices (not time step), e.g. 0,1,2")
	ap.add_argument("--out-dir", required=True)
	ap.add_argument("--width", type=int, default=1920)
	ap.add_argument("--height", type=int, default=1080)
	ap.add_argument("--field", default="displacement", help="Workpiece scalar field to color by (e.g. displacement, Svm, temperature)")
	ap.add_argument("--model-label", default="unknown")
	ap.add_argument("--verbose", action="store_true")
	args = ap.parse_args()

	logger = _configure_logging(args.out_dir, args.verbose)

	report = {
		"timestamp": datetime.now(timezone.utc).isoformat(),
		"status": "failed",
		"inputs": {
			"wp_vtk_pattern": args.wp_vtk_pattern,
			"fe_vtk_pattern": args.fe_vtk_pattern,
			"steps": args.steps,
			"field": args.field,
		},
		"outputs": [],
		"errors": [],
	}

	try:
		pv = _try_import_paraview()
		steps = _parse_steps(args.steps)
		if not steps:
			raise ValueError("no steps provided")

		for s in steps:
			wp_vtk = args.wp_vtk_pattern % s
			fe_vtk = args.fe_vtk_pattern % s
			_require_file(wp_vtk, "wp_vtk")
			_require_file(fe_vtk, "fe_vtk")

			wp = pv["LegacyVTKReader"](FileNames=[wp_vtk])
			fe = pv["LegacyVTKReader"](FileNames=[fe_vtk])
			pv["RenameSource"]("workpiece", wp)
			pv["RenameSource"]("fe_tool", fe)

			view = pv["CreateView"]("RenderView")
			view.ViewSize = [args.width, args.height]
			view.Background = [1.0, 1.0, 1.0]

			wp_disp = pv["Show"](wp, view)
			wp_disp.Representation = "Points"
			wp_disp.PointSize = 2.0

			if _has_point_array(wp, args.field):
				pv["ColorBy"](wp_disp, ("POINTS", args.field))
				lut = pv["GetColorTransferFunction"](args.field)
				pwf = pv["GetOpacityTransferFunction"](args.field)
				_apply_diverging_cool_to_warm(lut, pwf)
				wp_disp.SetScalarBarVisibility(view, True)
			else:
				pv["ColorBy"](wp_disp, None)

			fe_disp = pv["Show"](fe, view)
			fe_disp.Representation = "Surface"
			fe_disp.DiffuseColor = [0.6, 0.6, 0.6]
			fe_disp.Opacity = 0.35

			title = pv["Text"](Text=f"Tool Advancement | {args.model_label} | frame={s} | field={args.field}")
			title_disp = pv["Show"](title, view)
			try:
				title_disp.WindowLocation = "Upper Center"
			except Exception:
				pass

			_setup_2d_camera(view, wp, fe)
			pv["ResetCamera"](view)

			out_path = os.path.join(args.out_dir, f"adv_{args.field}_{s:06d}.png")
			logger.info("writing %s", out_path)
			pv["SaveScreenshot"](out_path, view, ImageResolution=[args.width, args.height])
			report["outputs"].append(out_path)

			pv["Delete"](title)
			pv["Delete"](fe_disp)
			pv["Delete"](wp_disp)
			pv["Delete"](fe)
			pv["Delete"](wp)
			pv["Delete"](view)

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

