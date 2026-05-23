#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, List
DEFAULT_ROI_PRESET_NAMES = ["bottom_half", "center", "chip_zone", "full", "top_half"]


def _split_patterns(text: str) -> List[str]:
    return [chunk.strip() for chunk in text.split(",") if chunk.strip()]


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


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Extract structured fracture and metadata information from simulation result images."
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        help="One or more image files or directories containing result images.",
    )
    parser.add_argument(
        "--patterns",
        default="*.png,*.jpg,*.jpeg,*.bmp,*.tif,*.tiff,*.webp",
        help="Comma-separated file glob patterns used for directory inputs.",
    )
    parser.add_argument(
        "--recursive",
        action="store_true",
        help="Recursively scan directory inputs for matching image files.",
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
        default="full",
        choices=DEFAULT_ROI_PRESET_NAMES,
        help="Named ROI preset used when --roi is not provided.",
    )
    parser.add_argument("--dark-threshold", type=int, default=38, help="Dark-pixel threshold for crack-like regions.")
    parser.add_argument("--edge-threshold", type=int, default=42, help="Edge threshold for discontinuity detection.")
    parser.add_argument(
        "--min-component-area",
        type=int,
        default=30,
        help="Minimum connected-component area in pixels for fracture candidate regions.",
    )
    parser.add_argument(
        "--ocr",
        dest="enable_ocr",
        action="store_true",
        default=True,
        help="Enable OCR extraction via pytesseract (default: enabled).",
    )
    parser.add_argument(
        "--no-ocr",
        dest="enable_ocr",
        action="store_false",
        help="Disable OCR extraction.",
    )
    parser.add_argument("--ocr-language", default="eng", help="OCR language code passed to pytesseract.")
    parser.add_argument(
        "--debug-dir",
        default=None,
        help="Optional directory for per-image debug artifacts (ROI, mask, overlay).",
    )
    parser.add_argument(
        "--output-json",
        default="results/image_extract/image_extract_summary.json",
        help="Output JSON summary path.",
    )
    parser.add_argument(
        "--output-csv",
        default="results/image_extract/image_extract_summary.csv",
        help="Output CSV summary path.",
    )
    parser.add_argument("--pretty-json", action="store_true", help="Pretty-print the JSON summary.")
    return parser


def main() -> int:
    parser = build_parser()
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
        enable_ocr=bool(args.enable_ocr),
        ocr_language=args.ocr_language,
        debug_dir=args.debug_dir,
    )

    payload = {
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
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

    if payload["error_count"] > 0:
        print(f"Completed with {payload['error_count']} image-level error(s).", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
