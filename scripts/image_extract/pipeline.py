from __future__ import annotations

from typing import Any, Dict, Iterable, List

from .confidence import score_fracture_confidence
from .diagnostics import write_debug_artifacts
from .fracture import detect_fracture_features
from .image_io import load_rgb_image, normalize_grayscale
from .roi import resolve_roi
from .text_extract import extract_text


def _to_image_coordinates(
    roi_box: List[int],
    *,
    roi_bbox: List[int] | None,
    roi_centroid: List[float] | None,
) -> Dict[str, Any]:
    x0, y0, _, _ = roi_box
    out: Dict[str, Any] = {
        "fracture_bbox_image_px": None,
        "fracture_centroid_image_px": None,
    }

    if roi_bbox:
        out["fracture_bbox_image_px"] = [x0 + roi_bbox[0], y0 + roi_bbox[1], x0 + roi_bbox[2], y0 + roi_bbox[3]]

    if roi_centroid:
        out["fracture_centroid_image_px"] = [round(x0 + roi_centroid[0], 3), round(y0 + roi_centroid[1], 3)]

    return out


def analyze_image(
    image_path: str,
    *,
    roi_text: str | None = None,
    roi_relative: bool = False,
    roi_preset: str = "full",
    dark_threshold: int = 38,
    edge_threshold: int = 42,
    min_component_area: int = 30,
    presence_area_ratio_threshold: float = 0.0015,
    presence_edge_ratio_threshold: float = 0.0060,
    enable_ocr: bool = True,
    ocr_language: str = "eng",
    debug_dir: str | None = None,
) -> Dict[str, Any]:
    source_image = load_rgb_image(image_path)
    normalized_grayscale = normalize_grayscale(source_image)
    roi_box, roi_source = resolve_roi(
        source_image.size,
        roi_text=roi_text,
        roi_relative=roi_relative,
        preset_name=roi_preset,
    )

    roi_rgb = source_image.crop(roi_box)
    roi_gray = normalized_grayscale.crop(roi_box)

    ocr_output = extract_text(roi_rgb, enabled=enable_ocr, language=ocr_language)
    fracture_features = detect_fracture_features(
        roi_gray,
        dark_threshold=dark_threshold,
        edge_threshold=edge_threshold,
        min_component_area=min_component_area,
        presence_area_ratio_threshold=presence_area_ratio_threshold,
        presence_edge_ratio_threshold=presence_edge_ratio_threshold,
    )
    confidence, confidence_breakdown = score_fracture_confidence(fracture_features)

    image_width, image_height = source_image.size
    roi_width = roi_box[2] - roi_box[0]
    roi_height = roi_box[3] - roi_box[1]
    coordinate_mapping = _to_image_coordinates(
        list(roi_box),
        roi_bbox=fracture_features.get("fracture_bbox_roi_px"),
        roi_centroid=fracture_features.get("fracture_centroid_roi_px"),
    )

    combined_mask = fracture_features.pop("_combined_mask")
    debug = None
    if debug_dir:
        debug = write_debug_artifacts(
            image_path=image_path,
            source_image=source_image,
            roi_box=roi_box,
            fracture_bbox_roi=fracture_features.get("fracture_bbox_roi_px"),
            fracture_centroid_roi=fracture_features.get("fracture_centroid_roi_px"),
            combined_mask=combined_mask,
            debug_dir=debug_dir,
        )

    return {
        "status": "ok",
        "image_path": image_path,
        "image_width_px": image_width,
        "image_height_px": image_height,
        "roi_box_px": list(roi_box),
        "roi_width_px": roi_width,
        "roi_height_px": roi_height,
        "roi_source": roi_source,
        "fracture": {
            **fracture_features,
            **coordinate_mapping,
            "confidence": confidence,
            "confidence_breakdown": confidence_breakdown,
        },
        "text": ocr_output,
        "debug": debug,
    }


def analyze_images(
    image_paths: Iterable[str],
    *,
    roi_text: str | None = None,
    roi_relative: bool = False,
    roi_preset: str = "full",
    dark_threshold: int = 38,
    edge_threshold: int = 42,
    min_component_area: int = 30,
    presence_area_ratio_threshold: float = 0.0015,
    presence_edge_ratio_threshold: float = 0.0060,
    enable_ocr: bool = True,
    ocr_language: str = "eng",
    debug_dir: str | None = None,
) -> List[Dict[str, Any]]:
    results: List[Dict[str, Any]] = []
    for image_path in image_paths:
        try:
            results.append(
                analyze_image(
                    image_path,
                    roi_text=roi_text,
                    roi_relative=roi_relative,
                    roi_preset=roi_preset,
                    dark_threshold=dark_threshold,
                    edge_threshold=edge_threshold,
                    min_component_area=min_component_area,
                    presence_area_ratio_threshold=presence_area_ratio_threshold,
                    presence_edge_ratio_threshold=presence_edge_ratio_threshold,
                    enable_ocr=enable_ocr,
                    ocr_language=ocr_language,
                    debug_dir=debug_dir,
                )
            )
        except Exception as exc:
            results.append(
                {
                    "status": "error",
                    "image_path": image_path,
                    "error": str(exc),
                }
            )
    return results
