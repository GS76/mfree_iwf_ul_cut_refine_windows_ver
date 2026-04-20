import argparse
import json
import logging
import os
import sys
from datetime import datetime, timezone


def _configure_logging(out_dir: str, verbose: bool) -> logging.Logger:
	os.makedirs(out_dir, exist_ok=True)
	logger = logging.getLogger("pvtooltemp")
	logger.setLevel(logging.DEBUG)
	logger.handlers.clear()
	fmt = logging.Formatter("%(asctime)s %(levelname)s %(message)s")

	ch = logging.StreamHandler(sys.stdout)
	ch.setLevel(logging.DEBUG if verbose else logging.INFO)
	ch.setFormatter(fmt)
	logger.addHandler(ch)

	fh = logging.FileHandler(os.path.join(out_dir, "paraview_batch_fe_tool_temperature.log"), mode="w", encoding="utf-8")
	fh.setLevel(logging.DEBUG)
	fh.setFormatter(fmt)
	logger.addHandler(fh)
	return logger


def _write_report(out_dir: str, report: dict) -> None:
	path = os.path.join(out_dir, "paraview_batch_fe_tool_temperature_report.json")
	with open(path, "w", encoding="utf-8") as f:
		json.dump(report, f, indent=2, sort_keys=True)


def _require_file(path: str, label: str) -> None:
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
	ap.add_argument("--fe-vtk-pattern", required=True, help="FE tool pattern, e.g. results/.../fe_tool_%06d.vtk")
	ap.add_argument("--steps", required=True, help="Comma-separated print indices, e.g. 0,1,2")
	ap.add_argument("--out-dir", required=True)
	ap.add_argument("--width", type=int, default=1920)
	ap.add_argument("--height", type=int, default=1080)
	ap.add_argument("--field", default="temperature")
	ap.add_argument("--model-label", default="unknown")
	ap.add_argument("--verbose", action="store_true")
	args = ap.parse_args()

	logger = _configure_logging(args.out_dir, args.verbose)
	os.makedirs(args.out_dir, exist_ok=True)

	report = {
		"timestamp": datetime.now(timezone.utc).isoformat(),
		"status": "failed",
		"inputs": {"fe_vtk_pattern": args.fe_vtk_pattern, "steps": args.steps, "field": args.field},
		"outputs": [],
		"errors": [],
	}

	try:
		pv = _try_import_paraview()
		steps = _parse_steps(args.steps)
		if not steps:
			raise ValueError("no steps provided")

		for s in steps:
			fe_vtk = args.fe_vtk_pattern % s
			_require_file(fe_vtk, "fe_vtk")

			fe = pv["LegacyVTKReader"](FileNames=[fe_vtk])
			pv["RenameSource"]("fe_tool", fe)

			view = pv["CreateView"]("RenderView")
			view.ViewSize = [args.width, args.height]
			view.Background = [1.0, 1.0, 1.0]

			fe_disp = pv["Show"](fe, view)
			fe_disp.Representation = "Surface"
			fe_disp.Opacity = 1.0

			if _has_point_array(fe, args.field):
				pv["ColorBy"](fe_disp, ("POINTS", args.field))
				lut = pv["GetColorTransferFunction"](args.field)
				pwf = pv["GetOpacityTransferFunction"](args.field)
				_apply_diverging_cool_to_warm(lut, pwf)
				fe_disp.SetScalarBarVisibility(view, True)
			else:
				pv["ColorBy"](fe_disp, None)

			title = pv["Text"](Text=f"FE Tool {args.field} | {args.model_label} | frame={s}")
			title_disp = pv["Show"](title, view)
			try:
				title_disp.WindowLocation = "Upper Center"
			except Exception:
				pass

			_setup_2d_camera(view, fe)
			pv["ResetCamera"](view)

			out_path = os.path.join(args.out_dir, f"fe_tool_{args.field}_{s:06d}.png")
			logger.info("writing %s", out_path)
			pv["SaveScreenshot"](out_path, view, ImageResolution=[args.width, args.height])
			report["outputs"].append(out_path)

			pv["Delete"](title)
			pv["Delete"](fe_disp)
			pv["Delete"](fe)
			pv["Delete"](view)

		report["status"] = "success"
		_write_report(args.out_dir, report)
		logger.info("success")
		return 0
	except Exception as e:
		logger.exception("failure")
		report["errors"].append(str(e))
		_write_report(args.out_dir, report)
		return 2


if __name__ == "__main__":
	raise SystemExit(main())

