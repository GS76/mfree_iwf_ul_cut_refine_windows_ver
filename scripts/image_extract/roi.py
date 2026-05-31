from __future__ import annotations

from typing import Any, Dict, Tuple

Box = Tuple[int, int, int, int]
ImageSize = Tuple[int, int]

PRESET_ROIS: Dict[str, Tuple[float, float, float, float]] = {
    "full": (0.0, 0.0, 1.0, 1.0),
    "center": (0.2, 0.2, 0.6, 0.6),
    "top_half": (0.0, 0.0, 1.0, 0.5),
    "bottom_half": (0.0, 0.5, 1.0, 0.5),
    "chip_zone": (0.1, 0.2, 0.8, 0.6),
}

LAYOUT_PROFILES: Dict[str, Dict[str, Any]] = {
    "legacy": {
        "enabled": False,
    },
    "paraview_vtk_dual_bar": {
        "enabled": True,
        "top_exclusion_fraction": 0.16,
        "bottom_exclusion_fraction": 0.14,
        "padding_fraction": 0.02,
        "dark_threshold": 188,
        "color_delta_threshold": 18,
        "background_delta_threshold": 28,
        "min_foreground_ratio": 0.001,
    },
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


def _median_channel(values: list[int]) -> int:
    if not values:
        return 210
    sorted_values = sorted(values)
    return int(sorted_values[len(sorted_values) // 2])


def _estimate_background_rgb(image) -> Tuple[int, int, int]:
    width, height = image.size
    if width <= 0 or height <= 0:
        return 210, 210, 210

    pixels = image.load()
    x_band = max(1, width // 100)
    y_start = max(0, int(round(0.2 * height)))
    y_stop = min(height, int(round(0.8 * height)))
    if y_stop <= y_start:
        y_start = 0
        y_stop = height

    sample_r: list[int] = []
    sample_g: list[int] = []
    sample_b: list[int] = []
    for y in range(y_start, y_stop):
        for x in range(x_band):
            r, g, b = pixels[x, y]
            sample_r.append(int(r))
            sample_g.append(int(g))
            sample_b.append(int(b))

            rx = width - 1 - x
            r, g, b = pixels[rx, y]
            sample_r.append(int(r))
            sample_g.append(int(g))
            sample_b.append(int(b))

    return _median_channel(sample_r), _median_channel(sample_g), _median_channel(sample_b)


def _detect_scene_bbox_from_crop(image, profile: Dict[str, Any]) -> Tuple[Box, Dict[str, Any]]:
    width, height = image.size
    if width <= 2 or height <= 2:
        return (0, 0, width, height), {
            "scene_detected": False,
            "foreground_ratio": 0.0,
            "candidate_crop_box_px": [0, 0, width, height],
            "reason": "crop_too_small",
        }

    top_exclusion = int(round(height * float(profile.get("top_exclusion_fraction", 0.16))))
    bottom_exclusion = int(round(height * float(profile.get("bottom_exclusion_fraction", 0.14))))
    y0 = max(0, min(height - 2, top_exclusion))
    y1 = max(y0 + 1, min(height, height - bottom_exclusion))
    if y1 <= y0:
        y0, y1 = 0, height

    pixels = image.load()
    bg_r, bg_g, bg_b = _estimate_background_rgb(image)
    dark_threshold = int(profile.get("dark_threshold", 188))
    color_delta_threshold = int(profile.get("color_delta_threshold", 18))
    background_delta_threshold = int(profile.get("background_delta_threshold", 28))
    min_foreground_ratio = float(profile.get("min_foreground_ratio", 0.001))

    fg_count = 0
    min_x = width
    min_y = y1
    max_x = -1
    max_y = -1

    for y in range(y0, y1):
        for x in range(width):
            r, g, b = pixels[x, y]
            luma = (int(r) + int(g) + int(b)) / 3.0
            chroma = max(int(r), int(g), int(b)) - min(int(r), int(g), int(b))
            background_delta = max(abs(int(r) - bg_r), abs(int(g) - bg_g), abs(int(b) - bg_b))
            is_foreground = (
                luma <= dark_threshold
                or chroma >= color_delta_threshold
                or background_delta >= background_delta_threshold
            )
            if not is_foreground:
                continue

            fg_count += 1
            min_x = min(min_x, x)
            min_y = min(min_y, y)
            max_x = max(max_x, x)
            max_y = max(max_y, y)

    candidate_area = max(1, width * max(1, y1 - y0))
    foreground_ratio = fg_count / candidate_area
    if fg_count == 0 or foreground_ratio < min_foreground_ratio:
        return (0, 0, width, height), {
            "scene_detected": False,
            "foreground_ratio": round(foreground_ratio, 6),
            "candidate_crop_box_px": [0, y0, width, y1],
            "reason": "foreground_not_detected",
        }

    padding = max(1, int(round(float(profile.get("padding_fraction", 0.02)) * max(width, height))))
    box = (
        max(0, min_x - padding),
        max(0, min_y - padding),
        min(width, max_x + 1 + padding),
        min(height, max_y + 1 + padding),
    )
    return _clip_box(box, (width, height)), {
        "scene_detected": True,
        "foreground_ratio": round(foreground_ratio, 6),
        "candidate_crop_box_px": [0, y0, width, y1],
        "reason": "ok",
    }


def apply_layout_profile(image, *, base_roi: Box, profile_name: str) -> Tuple[Box, str, Dict[str, Any]]:
    if profile_name not in LAYOUT_PROFILES:
        available = ", ".join(sorted(LAYOUT_PROFILES.keys()))
        raise ValueError(f"unknown layout profile '{profile_name}'. Available: {available}")

    profile = LAYOUT_PROFILES[profile_name]
    if not bool(profile.get("enabled", False)):
        return base_roi, "layout:legacy", {"scene_detected": False, "reason": "layout_disabled"}

    x0, y0, x1, y1 = base_roi
    crop = image.crop(base_roi)
    scene_box_local, metadata = _detect_scene_bbox_from_crop(crop, profile)
    sx0, sy0, sx1, sy1 = scene_box_local
    scene_box_global = (x0 + sx0, y0 + sy0, x0 + sx1, y0 + sy1)
    scene_box_global = _clip_box(scene_box_global, image.size)

    return scene_box_global, f"layout:{profile_name}:auto_scene", metadata
