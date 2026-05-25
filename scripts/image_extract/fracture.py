from __future__ import annotations

from collections import deque
from typing import Any, Dict, List, Sequence

from PIL import Image, ImageFilter


def _threshold_mask(image: Image.Image, *, threshold: int, keep_below: bool) -> List[bool]:
    pixels = list(image.getdata())
    if keep_below:
        return [value <= threshold for value in pixels]
    return [value >= threshold for value in pixels]


def _connected_components(
    mask: Sequence[bool],
    *,
    width: int,
    height: int,
    min_component_area: int,
) -> List[Dict[str, Any]]:
    components: List[Dict[str, Any]] = []
    visited = bytearray(width * height)

    def neighbors(index: int) -> List[int]:
        x = index % width
        y = index // width
        out: List[int] = []
        for ny in range(max(0, y - 1), min(height, y + 2)):
            for nx in range(max(0, x - 1), min(width, x + 2)):
                if nx == x and ny == y:
                    continue
                out.append(ny * width + nx)
        return out

    for start_index, is_true in enumerate(mask):
        if not is_true or visited[start_index]:
            continue

        queue: deque[int] = deque([start_index])
        visited[start_index] = 1
        points: List[int] = []

        while queue:
            index = queue.popleft()
            points.append(index)
            for nxt in neighbors(index):
                if visited[nxt] or not mask[nxt]:
                    continue
                visited[nxt] = 1
                queue.append(nxt)

        area = len(points)
        if area < min_component_area:
            continue

        min_x = width
        min_y = height
        max_x = -1
        max_y = -1
        sum_x = 0
        sum_y = 0

        for index in points:
            x = index % width
            y = index // width
            min_x = min(min_x, x)
            min_y = min(min_y, y)
            max_x = max(max_x, x)
            max_y = max(max_y, y)
            sum_x += x
            sum_y += y

        bbox = [min_x, min_y, max_x + 1, max_y + 1]
        centroid = [sum_x / area, sum_y / area]
        span_px = max(max_x - min_x + 1, max_y - min_y + 1)
        components.append(
            {
                "area_px": area,
                "bbox_roi_px": bbox,
                "centroid_roi_px": centroid,
                "span_px": span_px,
            }
        )

    components.sort(key=lambda component: component["area_px"], reverse=True)
    return components


def detect_fracture_features(
    grayscale_roi: Image.Image,
    *,
    dark_threshold: int,
    edge_threshold: int,
    min_component_area: int,
    presence_area_ratio_threshold: float = 0.0015,
    presence_edge_ratio_threshold: float = 0.0060,
) -> Dict[str, Any]:
    width, height = grayscale_roi.size
    pixel_count = width * height

    if pixel_count <= 0:
        raise ValueError("ROI has zero pixels")

    dark_mask = _threshold_mask(
        grayscale_roi,
        threshold=max(0, min(255, int(dark_threshold))),
        keep_below=True,
    )
    edge_image = grayscale_roi.filter(ImageFilter.FIND_EDGES)
    edge_mask = _threshold_mask(
        edge_image,
        threshold=max(0, min(255, int(edge_threshold))),
        keep_below=False,
    )

    combined_mask = [dark_mask[i] or edge_mask[i] for i in range(pixel_count)]
    dark_true_px = sum(dark_mask)
    edge_true_px = sum(edge_mask)

    components = _connected_components(
        combined_mask,
        width=width,
        height=height,
        min_component_area=max(1, int(min_component_area)),
    )

    largest_component = components[0] if components else None
    fracture_area_px = sum(component["area_px"] for component in components)
    area_ratio = fracture_area_px / pixel_count
    dark_ratio = dark_true_px / pixel_count
    edge_ratio = edge_true_px / pixel_count

    fracture_present = bool(
        (largest_component is not None and area_ratio >= presence_area_ratio_threshold)
        or edge_ratio >= presence_edge_ratio_threshold
    )

    return {
        "roi_width_px": width,
        "roi_height_px": height,
        "roi_area_px": pixel_count,
        "dark_pixel_ratio": round(dark_ratio, 6),
        "edge_pixel_ratio": round(edge_ratio, 6),
        "combined_mask_true_px": int(sum(combined_mask)),
        "fracture_present": fracture_present,
        "fracture_component_count": len(components),
        "fracture_area_px": int(fracture_area_px),
        "largest_component_area_px": int(largest_component["area_px"]) if largest_component else 0,
        "fracture_length_px": int(largest_component["span_px"]) if largest_component else 0,
        "fracture_bbox_roi_px": largest_component["bbox_roi_px"] if largest_component else None,
        "fracture_centroid_roi_px": (
            [round(largest_component["centroid_roi_px"][0], 3), round(largest_component["centroid_roi_px"][1], 3)]
            if largest_component
            else None
        ),
        "_combined_mask": combined_mask,
    }
