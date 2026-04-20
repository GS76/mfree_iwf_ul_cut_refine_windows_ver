import argparse
import csv
import json
import re
from pathlib import Path


PATH_RE = re.compile(r"(Meshing/[^\s]+|src/[^\s]+|scripts/[^\s]+|docs/[^\s]+|snapshots/[^\s]+)")
TIME_RE = re.compile(r"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d+Z)")


def decode_log(path: Path) -> str:
    b = path.read_bytes()
    if b[:2] in (b"\xff\xfe", b"\xfe\xff"):
        return b.decode("utf-16", errors="replace")
    return b.decode("utf-8", errors="replace")


def parse_run_list_json(path: Path):
    runs = json.loads(path.read_text(encoding="utf-8-sig"))
    out = []
    for r in runs:
        out.append(
            {
                "run_id": int(r["databaseId"]),
                "commit": r["headSha"],
                "title": r["displayTitle"],
                "url": r["url"],
                "event": r.get("event", ""),
                "createdAt": r.get("createdAt", ""),
                "conclusion": r.get("conclusion", ""),
            }
        )
    return out


def extract_failures_from_log_text(s: str):
    failures = []

    m = re.search(
        r"clang-format check failed for:\s*(?:\r?\n)(.*?)(?:\r?\n\s*\r?\n|\Z)",
        s,
        re.DOTALL | re.IGNORECASE,
    )
    if m:
        block = m.group(1)
        rows = []
        for line in block.splitlines():
            t = line.strip("\r\n")
            if not t:
                continue
            ts = TIME_RE.search(t)
            ts = ts.group(1) if ts else ""
            for p in PATH_RE.findall(t):
                rows.append({"file_path": p.strip(), "timestamp": ts, "error": "not clang-formatted"})

        seen = {}
        for r in rows:
            if r["file_path"] not in seen:
                seen[r["file_path"]] = r
        failures.append({"step": "clang-format", "rows": list(seen.values())})

    m = re.search(
        r"basic \.editorconfig check failed:\s*(?:\r?\n)(.*?)(?:\r?\n\s*\r?\n|\Z)",
        s,
        re.DOTALL | re.IGNORECASE,
    )
    if m:
        block = m.group(1)
        rows = []
        for line in block.splitlines():
            t = line.strip("\r\n")
            if not t:
                continue
            mm = re.match(r"\s*([^:]+):\s*(.+)$", t)
            if not mm:
                continue
            rows.append({"file_path": mm.group(1).strip(), "timestamp": "", "error": mm.group(2).strip()})
        failures.append({"step": "EditorConfig (basic)", "rows": rows})

    return failures


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--runs-json", required=True, help="Path to gh run list --json output")
    ap.add_argument(
        "--log-dir",
        default=".",
        help="Directory containing ci_run_<run_id>.log files (downloaded via gh run view --log)",
    )
    ap.add_argument("--out-csv", default="docs/ci_failure_report_runs_1_3.csv")
    ap.add_argument("--out-md", default="docs/ci_failure_report_runs_1_3.md")
    args = ap.parse_args()

    runs = parse_run_list_json(Path(args.runs_json))
    log_dir = Path(args.log_dir)

    rows = []
    for r in runs:
        run_id = r["run_id"]
        log_path = log_dir / f"ci_run_{run_id}.log"
        if not log_path.exists():
            continue
        text = decode_log(log_path)
        failures = extract_failures_from_log_text(text)
        for f in failures:
            for item in f["rows"]:
                rows.append(
                    {
                        "run_id": run_id,
                        "commit": r["commit"],
                        "title": r["title"],
                        "workflow": "quality",
                        "job": "formatting",
                        "step": f["step"],
                        "file_path": item["file_path"],
                        "timestamp": item["timestamp"],
                        "error": item["error"],
                        "run_url": r["url"],
                        "event": r.get("event", ""),
                        "createdAt": r.get("createdAt", ""),
                        "conclusion": r.get("conclusion", ""),
                    }
                )

    out_csv = Path(args.out_csv)
    out_csv.parent.mkdir(parents=True, exist_ok=True)
    if rows:
        with out_csv.open("w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
            w.writeheader()
            w.writerows(rows)
    else:
        out_csv.write_text("", encoding="utf-8")

    counts = {}
    for row in rows:
        key = (row["run_id"], row["step"])
        counts[key] = counts.get(key, 0) + 1

    out_md = Path(args.out_md)
    out_md.parent.mkdir(parents=True, exist_ok=True)
    lines = []
    lines.append("# CI Failure Report (Runs 1–3)")
    lines.append("")
    lines.append("Generated from downloaded GitHub Actions logs for the `quality` workflow.")
    lines.append("")
    lines.append("## Runs")
    lines.append("")
    for r in runs:
        lines.append(f"- Run {r['run_id']}: {r['title']} ({r['commit'][:7]})")
        lines.append(f"  - URL: {r['url']}")
        lines.append(f"  - Conclusion: {r.get('conclusion','')}")
        lines.append(f"  - Created: {r.get('createdAt','')}")
    lines.append("")
    lines.append("## Failure Counts")
    lines.append("")
    lines.append("| Run ID | Step | Failing files |")
    lines.append("|---:|---|---:|")
    for (run_id, step), n in sorted(counts.items()):
        lines.append(f"| {run_id} | {step} | {n} |")
    lines.append("")
    lines.append("## Full Mapping")
    lines.append("")
    lines.append(f"- CSV: {out_csv.as_posix()}")
    lines.append("")
    out_md.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    raise SystemExit(main())
