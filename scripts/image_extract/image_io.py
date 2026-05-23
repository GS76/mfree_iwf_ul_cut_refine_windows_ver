from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageFilter, ImageOps


def load_rgb_image(path: str) -> Image.Image:
    image_path = Path(path)
    if not image_path.is_file():
        raise FileNotFoundError(f"image not found: {path}")

    with Image.open(image_path) as image:
        return image.convert("RGB")


def normalize_grayscale(
    image: Image.Image,
    *,
    autocontrast_cutoff: int = 2,
    blur_radius: float = 0.8,
) -> Image.Image:
    grayscale = ImageOps.grayscale(image)

    cutoff = max(0, min(49, int(autocontrast_cutoff)))
    grayscale = ImageOps.autocontrast(grayscale, cutoff=cutoff)

    if blur_radius > 0:
        grayscale = grayscale.filter(ImageFilter.GaussianBlur(radius=float(blur_radius)))

    return grayscale
