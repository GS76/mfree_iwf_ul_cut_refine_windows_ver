from __future__ import annotations

from typing import Any, Dict

from PIL import Image, ImageOps


def _normalize_text(text: str) -> str:
    lines = [line.strip() for line in text.splitlines()]
    cleaned_lines = [line for line in lines if line]
    return "\n".join(cleaned_lines)


def extract_text(image: Image.Image, *, enabled: bool, language: str) -> Dict[str, Any]:
    output: Dict[str, Any] = {
        "enabled": bool(enabled),
        "available": False,
        "engine": None,
        "language": language,
        "text": "",
        "line_count": 0,
        "char_count": 0,
        "error": None,
    }

    if not enabled:
        return output

    try:
        import pytesseract  # type: ignore
    except Exception as exc:
        output["error"] = f"pytesseract unavailable: {exc}"
        return output

    try:
        preprocessed = ImageOps.autocontrast(ImageOps.grayscale(image), cutoff=2)
        text = pytesseract.image_to_string(preprocessed, lang=language)
        normalized = _normalize_text(text)

        output["available"] = True
        output["engine"] = "pytesseract"
        output["text"] = normalized
        output["line_count"] = normalized.count("\n") + (1 if normalized else 0)
        output["char_count"] = len(normalized)
        return output
    except Exception as exc:
        output["available"] = True
        output["engine"] = "pytesseract"
        output["error"] = f"OCR failed: {exc}"
        return output
