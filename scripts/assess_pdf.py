import argparse
import json
import re
from collections import Counter
from pathlib import Path
from typing import Dict, List, Tuple


def load_pdf_reader(pdf_path: Path):
    try:
        from pypdf import PdfReader  # type: ignore
    except Exception:
        try:
            from PyPDF2 import PdfReader  # type: ignore
        except Exception as exc:
            raise RuntimeError(
                "No supported PDF parser found. Install `pypdf` (recommended) or `PyPDF2`."
            ) from exc
    return PdfReader(str(pdf_path))


def normalize_space(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def infer_title(first_page_text: str) -> str:
    candidates = []
    for raw in first_page_text.splitlines():
        line = normalize_space(raw)
        if not line:
            continue
        if len(line) < 8:
            continue
        if line.lower().startswith(("https://", "http://", "doi")):
            continue
        if len(line.split()) <= 2:
            continue
        candidates.append(line)
    if not candidates:
        return ""
    candidates.sort(key=len, reverse=True)
    return candidates[0]


def infer_sections(full_text: str) -> List[str]:
    section_patterns = [
        r"\babstract\b",
        r"\bintroduction\b",
        r"\bmethod(?:ology|s)?\b",
        r"\bresults?\b",
        r"\bdiscussion\b",
        r"\bconclusion(?:s)?\b",
        r"\breferences\b",
    ]
    found = []
    lc = full_text.lower()
    for pattern in section_patterns:
        m = re.search(pattern, lc)
        if m:
            found.append(re.sub(r"\\b|\(\?:|\)|\?", "", pattern).replace("|", "/"))
    return found


def keyword_frequencies(full_text: str, keywords: List[str]) -> Dict[str, int]:
    lc = full_text.lower()
    freq = {}
    for kw in keywords:
        k = kw.lower().strip()
        if not k:
            continue
        freq[k] = len(re.findall(rf"\b{re.escape(k)}\b", lc))
    return freq


def top_terms(full_text: str, limit: int = 25) -> List[Tuple[str, int]]:
    stop_words = {
        "the",
        "and",
        "for",
        "with",
        "that",
        "this",
        "from",
        "are",
        "was",
        "were",
        "been",
        "have",
        "has",
        "had",
        "into",
        "their",
        "than",
        "then",
        "they",
        "them",
        "which",
        "while",
        "using",
        "used",
        "also",
        "can",
        "may",
        "such",
        "these",
        "those",
        "not",
        "but",
        "our",
        "its",
        "via",
    }
    words = re.findall(r"[A-Za-z][A-Za-z0-9\-]+", full_text.lower())
    filtered = [w for w in words if len(w) >= 4 and w not in stop_words]
    counts = Counter(filtered)
    return counts.most_common(limit)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Assess a PDF and persist reusable assessment artifacts."
    )
    parser.add_argument("--pdf", required=True, help="Path to the input PDF")
    parser.add_argument(
        "--out-dir",
        default="results/pdf_assess",
        help="Directory to store assessment artifacts",
    )
    parser.add_argument(
        "--keywords",
        default="meshfree,SPH,thermal,contact,plasticity,refinement,tool,workpiece",
        help="Comma-separated keywords for frequency reporting",
    )
    args = parser.parse_args()

    pdf_path = Path(args.pdf).resolve()
    if not pdf_path.exists():
        raise FileNotFoundError(f"PDF not found: {pdf_path}")

    out_dir = Path(args.out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    reader = load_pdf_reader(pdf_path)
    page_count = len(reader.pages)

    page_texts: List[str] = []
    for i, page in enumerate(reader.pages):
        text = page.extract_text() or ""
        page_texts.append(text)
        page_file = out_dir / f"page_{i+1:03d}.txt"
        page_file.write_text(text, encoding="utf-8")

    full_text = "\n\n".join(page_texts)
    full_text_norm = normalize_space(full_text)

    first_page_text = page_texts[0] if page_texts else ""
    title = infer_title(first_page_text)

    metadata = {}
    if hasattr(reader, "metadata") and reader.metadata:
        for k, v in dict(reader.metadata).items():
            key = str(k).lstrip("/")
            metadata[key] = str(v)

    keywords = [k.strip() for k in args.keywords.split(",")]
    summary = {
        "pdf_path": str(pdf_path),
        "page_count": page_count,
        "title_inferred": title,
        "metadata": metadata,
        "sections_detected": infer_sections(full_text),
        "keyword_frequencies": keyword_frequencies(full_text, keywords),
        "top_terms": [{"term": t, "count": c} for t, c in top_terms(full_text)],
        "character_count": len(full_text),
        "word_count": len(full_text_norm.split()) if full_text_norm else 0,
    }

    summary_path = out_dir / "assessment_summary.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    text_path = out_dir / "document_text.txt"
    text_path.write_text(full_text, encoding="utf-8")

    preview_path = out_dir / "assessment_preview.txt"
    preview_lines = [
        f"PDF: {pdf_path}",
        f"Pages: {page_count}",
        f"Inferred title: {title}",
        f"Word count: {summary['word_count']}",
        f"Sections: {', '.join(summary['sections_detected']) if summary['sections_detected'] else 'none'}",
        "Top terms:",
    ]
    for item in summary["top_terms"][:15]:
        preview_lines.append(f"  - {item['term']}: {item['count']}")
    preview_path.write_text("\n".join(preview_lines) + "\n", encoding="utf-8")

    print(f"assessment_summary: {summary_path}")
    print(f"document_text: {text_path}")
    print(f"assessment_preview: {preview_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
