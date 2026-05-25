#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, List

DEFAULT_REQUIREMENTS_PATH = "config/image_extraction_requirements.json"
DEFAULT_ROI_PRESET_NAMES = ["bottom_half", "center", "chip_zone", "full", "top_half"]
_MISSING = object()


def _split_patterns(text: str) -> List[str]:
    return [chunk.strip() for chunk in text.split(",") if chunk.strip()]


def _discover_requirements_path(argv: List[str]) -> str:
    mini_parser = argparse.ArgumentParser(add_help=False)
    mini_parser.add_argument("--requirements-file", default=DEFAULT_REQUIREMENTS_PATH)
    known, _ = mini_parser.parse_known_args(argv)
    return str(known.requirements_file)


def _load_requirements(path: str) -> Dict[str, Any]:
    req_path = Path(path)
    if not req_path.is_file():
        raise FileNotFoundError(f"requirements file not found: {path}")

    with req_path.open("r", encoding="utf-8") as handle:
        payload = json.load(handle)

    if not isinstance(payload, dict):
        raise ValueError("requirements payload must be a JSON object")
    return payload


def _coerce_bool(value: Any, default: bool) -> bool:
    if value is None:
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    if isinstance(value, str):
        lowered = value.strip().lower()
        if lowered in ("1", "true", "yes", "y", "on"):
            return True
        if lowered in ("0", "false", "no", "n", "off"):
            return False
    return default


def _coerce_int(value: Any, default: int) -> int:
    try:
        return int(value)
    except Exception:
        return int(default)


def _coerce_float(value: Any, default: float) -> float:
    try:
        return float(value)
    except Exception:
        return float(default)


def _resolve_defaults(requirements: Dict[str, Any]) -> Dict[str, Any]:
    defaults = requirements.get("defaults", {})
    if not isinstance(defaults, dict):
        return {}
    return defaults


def _nested_get(mapping: Dict[str, Any], path: str) -> Any:
    value: Any = mapping
    for key in path.split("."):
        if not isinstance(value, dict) or key not in value:
            return _MISSING
        value = value[key]
    return value


def _find_missing_required_fields(result: Dict[str, Any], required_fields: List[str]) -> List[str]:
    missing: List[str] = []
    for field_path in required_fields:
        if _nested_get(result, field_path) is _MISSING:
            missing.append(field_path)
    return missing


def _collect_images(inputs: Iterable[str], *, patterns: List[str], recursive: bool) -> List[str]:
    found: List[str] = []
    for item in inputs:
        path = Path(item)
        if path.is_file():
            found.append(str(path))
            continue

        if path.is_dir():
            for pattern in patterns:
                iterator = path.rglob(pattern) if recursive else path.glob(pattern)
                for matched in iterator:
                    if matched.is_file():
                        found.append(str(matched))
            continue

        raise FileNotFoundError(f"input path not found: {item}")

    unique_sorted = sorted(set(found))
    return unique_sorted


def _flatten_record(result: Dict[str, Any]) -> Dict[str, Any]:
    row: Dict[str, Any] = {
        "status": result.get("status"),
        "image_path": result.get("image_path"),
    }

    if result.get("status") != "ok":
        row["error"] = result.get("error", "")
        return row

    fracture = result.get("fracture", {})
    text_info = result.get("text", {})
    debug = result.get("debug", {})
    requirements_missing = result.get("requirements_missing_fields", [])

    row.update(
        {
            "image_width_px": result.get("image_width_px"),
            "image_height_px": result.get("image_height_px"),
            "roi_box_px": json.dumps(result.get("roi_box_px")),
            "roi_width_px": result.get("roi_width_px"),
            "roi_height_px": result.get("roi_height_px"),
            "roi_source": result.get("roi_source"),
            "fracture_present": fracture.get("fracture_present"),
            "fracture_area_px": fracture.get("fracture_area_px"),
            "fracture_length_px": fracture.get("fracture_length_px"),
            "fracture_component_count": fracture.get("fracture_component_count"),
            "largest_component_area_px": fracture.get("largest_component_area_px"),
            "fracture_bbox_roi_px": json.dumps(fracture.get("fracture_bbox_roi_px")),
            "fracture_bbox_image_px": json.dumps(fracture.get("fracture_bbox_image_px")),
            "fracture_centroid_roi_px": json.dumps(fracture.get("fracture_centroid_roi_px")),
            "fracture_centroid_image_px": json.dumps(fracture.get("fracture_centroid_image_px")),
            "dark_pixel_ratio": fracture.get("dark_pixel_ratio"),
            "edge_pixel_ratio": fracture.get("edge_pixel_ratio"),
            "combined_mask_true_px": fracture.get("combined_mask_true_px"),
            "fracture_confidence": fracture.get("confidence"),
            "confidence_breakdown": json.dumps(fracture.get("confidence_breakdown")),
            "ocr_enabled": text_info.get("enabled"),
            "ocr_available": text_info.get("available"),
            "ocr_engine": text_info.get("engine"),
            "ocr_language": text_info.get("language"),
            "ocr_line_count": text_info.get("line_count"),
            "ocr_char_count": text_info.get("char_count"),
            "ocr_error": text_info.get("error"),
            "ocr_text": text_info.get("text"),
            "debug_overlay_path": (debug or {}).get("overlay_path"),
            "debug_roi_path": (debug or {}).get("roi_path"),
            "debug_mask_path": (debug or {}).get("mask_path"),
            "requirements_missing_fields": json.dumps(requirements_missing),
        }
    )
    return row


def _write_json(path: Path, payload: Dict[str, Any], *, pretty: bool) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        if pretty:
            json.dump(payload, handle, indent=2, sort_keys=True)
        else:
            json.dump(payload, handle, separators=(",", ":"), sort_keys=True)
        handle.write("\n")


def _write_csv(path: Path, rows: List[Dict[str, Any]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)

    fieldnames: List[str] = []
    for row in rows:
        for key in row.keys():
            if key not in fieldnames:
                fieldnames.append(key)

    with path.open("w", encoding="utf-8", newline="\n") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)


def build_parser(*, requirements: Dict[str, Any], requirements_path: str) -> argparse.ArgumentParser:
    defaults = _resolve_defaults(requirements)
    strict_default = _coerce_bool(requirements.get("strict_requirements_default"), False)
    parser = argparse.ArgumentParser(
        description="Extract structured fracture and metadata information from simulation result images."
    )
    parser.add_argument(
        "--requirements-file",
        default=requirements_path,
        help="Path to the image extraction requirements contract JSON.",
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        help="One or more image files or directories containing result images.",
    )
    parser.add_argument(
        "--patterns",
        default=str(defaults.get("input_patterns", "*.png,*.jpg,*.jpeg,*.bmp,*.tif,*.tiff,*.webp")),
        help="Comma-separated file glob patterns used for directory inputs.",
    )
    parser.add_argument(
        "--recursive",
        dest="recursive",
        action="store_true",
        default=_coerce_bool(defaults.get("recursive"), False),
        help="Recursively scan directory inputs for matching image files.",
    )
    parser.add_argument(
        "--no-recursive",
        dest="recursive",
        action="store_false",
        help="Disable recursive scan for directory inputs.",
    )
    parser.add_argument(
        "--roi",
        default=None,
        help="Custom ROI as x,y,width,height (pixels by default; fractions with --roi-relative).",
    )
    parser.add_argument(
        "--roi-relative",
        action="store_true",
        help="Interpret --roi coordinates as relative fractions in [0,1].",
    )
    parser.add_argument(
        "--roi-preset",
        default=str(defaults.get("roi_preset", "full")),
        choices=DEFAULT_ROI_PRESET_NAMES,
        help="Named ROI preset used when --roi is not provided.",
    )
    parser.add_argument(
        "--dark-threshold",
        type=int,
        default=_coerce_int(defaults.get("dark_threshold"), 38),
        help="Dark-pixel threshold for crack-like regions.",
    )
    parser.add_argument(
        "--edge-threshold",
        type=int,
        default=_coerce_int(defaults.get("edge_threshold"), 42),
        help="Edge threshold for discontinuity detection.",
    )
    parser.add_argument(
        "--min-component-area",
        type=int,
        default=_coerce_int(defaults.get("min_component_area"), 30),
        help="Minimum connected-component area in pixels for fracture candidate regions.",
    )
    parser.add_argument(
        "--presence-area-ratio-threshold",
        type=float,
        default=_coerce_float(defaults.get("presence_area_ratio_threshold"), 0.0015),
        help="Minimum fracture-area ratio needed for fracture-present classification.",
    )
    parser.add_argument(
        "--presence-edge-ratio-threshold",
        type=float,
        default=_coerce_float(defaults.get("presence_edge_ratio_threshold"), 0.0060),
        help="Minimum edge-ratio needed for fracture-present classification.",
    )
    parser.add_argument(
        "--ocr",
        dest="enable_ocr",
        action="store_true",
        default=_coerce_bool(defaults.get("enable_ocr"), True),
        help="Enable OCR extraction via pytesseract (default: enabled).",
    )
    parser.add_argument(
        "--no-ocr",
        dest="enable_ocr",
        action="store_false",
        help="Disable OCR extraction.",
    )
    parser.add_argument(
        "--ocr-language",
        default=str(defaults.get("ocr_language", "eng")),
        help="OCR language code passed to pytesseract.",
    )
    parser.add_argument(
        "--debug-dir",
        default=defaults.get("debug_dir"),
        help="Optional directory for per-image debug artifacts (ROI, mask, overlay).",
    )
    parser.add_argument(
        "--output-json",
        default=str(defaults.get("output_json", "results/image_extract/image_extract_summary.json")),
        help="Output JSON summary path.",
    )
    parser.add_argument(
        "--output-csv",
        default=str(defaults.get("output_csv", "results/image_extract/image_extract_summary.csv")),
        help="Output CSV summary path.",
    )
    parser.add_argument(
        "--strict-requirements",
        dest="strict_requirements",
        action="store_true",
        default=strict_default,
        help="Fail if any required field from the requirements contract is missing in an OK result.",
    )
    parser.add_argument(
        "--no-strict-requirements",
        dest="strict_requirements",
        action="store_false",
        help="Do not fail the run when required fields are missing.",
    )
    parser.add_argument("--pretty-json", action="store_true", help="Pretty-print the JSON summary.")
    return parser


def main() -> int:
    discovered_requirements_path = _discover_requirements_path(sys.argv[1:])
    help_requested = any(arg in ("-h", "--help") for arg in sys.argv[1:])

    requirements: Dict[str, Any]
    try:
        requirements = _load_requirements(discovered_requirements_path)
    except Exception as exc:
        if help_requested:
            requirements = {}
        else:
            raise SystemExit(f"Failed to load requirements contract: {exc}")

    parser = build_parser(requirements=requirements, requirements_path=discovered_requirements_path)
    args = parser.parse_args()

    try:
        from image_extract.pipeline import analyze_images
    except Exception as exc:
        raise SystemExit(
            "Failed to import image extraction toolkit dependencies.\n"
            "Install required packages first, for example:\n"
            "  python -m pip install pillow pytesseract\n"
            f"Import error: {exc}"
        )

    patterns = _split_patterns(args.patterns)
    if not patterns:
        raise SystemExit("No valid file patterns supplied to --patterns")

    image_paths = _collect_images(args.inputs, patterns=patterns, recursive=args.recursive)
    if not image_paths:
        raise SystemExit("No images found from provided inputs.")

    results = analyze_images(
        image_paths,
        roi_text=args.roi,
        roi_relative=bool(args.roi_relative),
        roi_preset=args.roi_preset,
        dark_threshold=int(args.dark_threshold),
        edge_threshold=int(args.edge_threshold),
        min_component_area=int(args.min_component_area),
        presence_area_ratio_threshold=float(args.presence_area_ratio_threshold),
        presence_edge_ratio_threshold=float(args.presence_edge_ratio_threshold),
        enable_ocr=bool(args.enable_ocr),
        ocr_language=args.ocr_language,
        debug_dir=args.debug_dir,
    )

    required_fields_raw = requirements.get("required_output_fields", []) if isinstance(requirements, dict) else []
    required_fields = [str(value).strip() for value in required_fields_raw if str(value).strip()]
    images_with_missing_fields = 0

    for result in results:
        if result.get("status") != "ok":
            result["requirements_missing_fields"] = []
            continue
        missing_fields = _find_missing_required_fields(result, required_fields)
        result["requirements_missing_fields"] = missing_fields
        if missing_fields:
            images_with_missing_fields += 1

    payload = {
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "requirements": {
            "file": str(Path(args.requirements_file)),
            "schema_version": requirements.get("schema_version") if isinstance(requirements, dict) else None,
            "strict_requirements": bool(args.strict_requirements),
            "required_fields": required_fields,
            "images_with_missing_fields": images_with_missing_fields,
        },
        "input_count": len(image_paths),
        "ok_count": sum(1 for result in results if result.get("status") == "ok"),
        "error_count": sum(1 for result in results if result.get("status") != "ok"),
        "results": results,
    }

    json_path = Path(args.output_json)
    csv_path = Path(args.output_csv)
    _write_json(json_path, payload, pretty=bool(args.pretty_json))
    _write_csv(csv_path, [_flatten_record(result) for result in results])

    print(f"Processed {len(results)} image(s).")
    print(f"JSON summary: {json_path}")
    print(f"CSV summary:  {csv_path}")

    if bool(args.strict_requirements) and images_with_missing_fields > 0:
        print(
            f"Requirements validation failed for {images_with_missing_fields} image(s): missing required output fields.",
            file=sys.stderr,
        )
        return 3
    if payload["error_count"] > 0:
        print(f"Completed with {payload['error_count']} image-level error(s).", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())