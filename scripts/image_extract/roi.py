from __future__ import annotations

from typing import Dict, Tuple

Box = Tuple[int, int, int, int]
ImageSize = Tuple[int, int]

PRESET_ROIS: Dict[str, Tuple[float, float, float, float]] = {
    "full": (0.0, 0.0, 1.0, 1.0),
    "center": (0.2, 0.2, 0.6, 0.6),
    "top_half": (0.0, 0.0, 1.0, 0.5),
    "bottom_half": (0.0, 0.5, 1.0, 0.5),
    "chip_zone": (0.1, 0.2, 0.8, 0.6),
}


def _clip_box(box: Box, image_size: ImageSize) -> Box:
    width, height = image_size
    x0, y0, x1, y1 = box

    x0 = max(0, min(x0, width - 1))
    y0 = max(0, min(y0, height - 1))
    x1 = max(x0 + 1, min(x1, width))
    y1 = max(y0 + 1, min(y1, height))
    return x0, y0, x1, y1


def _parse_roi_text(roi_text: str) -> Tuple[float, float, float, float]:
    parts = [part.strip() for part in roi_text.split(",")]
    if len(parts) != 4:
        raise ValueError("ROI must have four values: x,y,width,height")
    values = tuple(float(part) for part in parts)
    return values  # type: ignore[return-value]


def _box_from_relative(relative: Tuple[float, float, float, float], image_size: ImageSize) -> Box:
    width, height = image_size
    x, y, w, h = relative

    x0 = int(round(x * width))
    y0 = int(round(y * height))
    x1 = int(round((x + w) * width))
    y1 = int(round((y + h) * height))
    return _clip_box((x0, y0, x1, y1), image_size)


def _box_from_absolute(absolute: Tuple[float, float, float, float], image_size: ImageSize) -> Box:
    x, y, w, h = absolute
    x0 = int(round(x))
    y0 = int(round(y))
    x1 = int(round(x + w))
    y1 = int(round(y + h))
    return _clip_box((x0, y0, x1, y1), image_size)


def resolve_roi(
    image_size: ImageSize,
    *,
    roi_text: str | None,
    roi_relative: bool,
    preset_name: str,
) -> Tuple[Box, str]:
    if roi_text:
        parsed = _parse_roi_text(roi_text)
        if roi_relative:
            return _box_from_relative(parsed, image_size), "custom:relative"
        return _box_from_absolute(parsed, image_size), "custom:absolute"

    if preset_name not in PRESET_ROIS:
        available = ", ".join(sorted(PRESET_ROIS.keys()))
        raise ValueError(f"unknown ROI preset '{preset_name}'. Available: {available}")

    return _box_from_relative(PRESET_ROIS[preset_name], image_size), f"preset:{preset_name}"
