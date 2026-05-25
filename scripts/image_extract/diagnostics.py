from __future__ import annotations

from pathlib import Path
from typing import Any, Dict, List, Tuple

from PIL import Image, ImageDraw

Box = Tuple[int, int, int, int]


def _safe_stem(path: str) -> str:
    stem = Path(path).stem.strip()
    if not stem:
        return "image"
    return "".join(ch if ch.isalnum() or ch in ("-", "_") else "_" for ch in stem)


def _mask_to_image(mask: List[bool], width: int, height: int) -> Image.Image:
    out = Image.new("L", (width, height))
    out.putdata([255 if value else 0 for value in mask])
    return out


def write_debug_artifacts(
    *,
    image_path: str,
    source_image: Image.Image,
    roi_box: Box,
    fracture_bbox_roi: List[int] | None,
    fracture_centroid_roi: List[float] | None,
    combined_mask: List[bool],
    debug_dir: str,
) -> Dict[str, str]:
    output_dir = Path(debug_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    stem = _safe_stem(image_path)
    overlay_path = output_dir / f"{stem}_overlay.png"
    roi_path = output_dir / f"{stem}_roi.png"
    mask_path = output_dir / f"{stem}_mask.png"

    x0, y0, x1, y1 = roi_box
    roi_crop = source_image.crop(roi_box)
    roi_crop.save(roi_path)

    overlay = source_image.copy()
    draw = ImageDraw.Draw(overlay)
    draw.rectangle(roi_box, outline=(255, 220, 0), width=3)

    if fracture_bbox_roi:
        fx0, fy0, fx1, fy1 = fracture_bbox_roi
        draw.rectangle((x0 + fx0, y0 + fy0, x0 + fx1, y0 + fy1), outline=(255, 0, 0), width=3)

    if fracture_centroid_roi:
        cx = int(round(x0 + fracture_centroid_roi[0]))
        cy = int(round(y0 + fracture_centroid_roi[1]))
        draw.ellipse((cx - 3, cy - 3, cx + 3, cy + 3), fill=(0, 255, 255), outline=(0, 255, 255))

    overlay.save(overlay_path)

    roi_width = max(1, x1 - x0)
    roi_height = max(1, y1 - y0)
    mask_image = _mask_to_image(combined_mask, roi_width, roi_height)
    mask_image.save(mask_path)

    return {
        "overlay_path": str(overlay_path),
        "roi_path": str(roi_path),
        "mask_path": str(mask_path),
    }
