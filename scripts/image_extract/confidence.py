from __future__ import annotations

from typing import Any, Dict, Tuple


def _clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def score_fracture_confidence(features: Dict[str, Any]) -> Tuple[float, Dict[str, float]]:
    roi_area = float(max(1, int(features.get("roi_area_px", 1))))
    fracture_present = bool(features.get("fracture_present", False))
    fracture_area_ratio = float(features.get("fracture_area_px", 0)) / roi_area
    largest_component_ratio = float(features.get("largest_component_area_px", 0)) / roi_area
    edge_ratio = float(features.get("edge_pixel_ratio", 0.0))
    dark_ratio = float(features.get("dark_pixel_ratio", 0.0))
    component_count = float(features.get("fracture_component_count", 0))

    if fracture_present:
        score = (
            0.25
            + min(0.35, largest_component_ratio * 18.0)
            + min(0.20, fracture_area_ratio * 9.0)
            + min(0.12, edge_ratio * 5.0)
            + min(0.08, component_count / 40.0)
        )
    else:
        score = min(0.35, fracture_area_ratio * 4.0 + edge_ratio * 2.5 + dark_ratio * 1.25)

    score = _clamp(score, 0.0, 1.0)
    breakdown = {
        "fracture_area_ratio": round(fracture_area_ratio, 6),
        "largest_component_ratio": round(largest_component_ratio, 6),
        "edge_ratio": round(edge_ratio, 6),
        "dark_ratio": round(dark_ratio, 6),
        "component_count": round(component_count, 3),
    }
    return round(score, 4), breakdown
