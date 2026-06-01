import argparse
import csv
import json
import math
import os
import random
import subprocess
import sys
from copy import deepcopy
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Tuple


def now_iso() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()

def in_warp_session() -> bool:
    term_program = os.environ.get("TERM_PROGRAM", "")
    if "warp" in term_program.lower():
        return True
    if os.environ.get("WARP_IS_LOCAL_SHELL_SESSION", "") == "1":
        return True
    return any(key.startswith("WARP_") for key in os.environ.keys())


def load_json(path: Path) -> Dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def save_json(path: Path, data: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def _validate_parameter(param: Dict[str, Any]) -> None:
    _require(isinstance(param, dict), "Each design parameter must be an object")
    _require("name" in param, "Each design parameter must define 'name'")
    _require("type" in param, f"Parameter '{param.get('name')}' must define 'type'")
    _require(
        param["type"] in {"float", "int"},
        f"Parameter '{param['name']}' type must be one of: float, int",
    )
    _require(
        "bounds" in param and isinstance(param["bounds"], list) and len(param["bounds"]) == 2,
        f"Parameter '{param['name']}' must define 2-element numeric 'bounds'",
    )
    low, high = param["bounds"]
    _require(
        isinstance(low, (int, float)) and isinstance(high, (int, float)) and high > low,
        f"Parameter '{param['name']}' bounds must be numeric and increasing",
    )


def _validate_metric_def(metric: Dict[str, Any], kind: str) -> None:
    _require(isinstance(metric, dict), f"{kind} entries must be objects")
    for key in ("name", "source_file", "column"):
        _require(key in metric, f"{kind} entry missing required field '{key}'")
    reducer = metric.get("reducer", "last")
    _require(
        reducer in {"last", "first", "max", "min", "mean"},
        f"{kind} '{metric['name']}' reducer must be one of: last, first, max, min, mean",
    )
    if kind == "constraint":
        _require(
            "operator" in metric and metric["operator"] in {"<=", ">=", "<", ">", "=="},
            f"constraint '{metric['name']}' must define operator in <=, >=, <, >, ==",
        )
        _require(
            "threshold" in metric and isinstance(metric["threshold"], (int, float)),
            f"constraint '{metric['name']}' must define numeric threshold",
        )


def validate_and_normalize_spec(spec: Dict[str, Any]) -> Dict[str, Any]:
    _require(isinstance(spec, dict), "Campaign spec must be a JSON object")
    for key in ("campaign_id", "script_entrypoint", "design", "objectives"):
        _require(key in spec, f"Campaign spec missing required field '{key}'")

    campaign_id = spec["campaign_id"]
    _require(
        isinstance(campaign_id, str) and campaign_id.strip() != "",
        "campaign_id must be a non-empty string",
    )

    design = spec["design"]
    _require(isinstance(design, dict), "design must be an object")
    _require(
        "parameters" in design and isinstance(design["parameters"], list) and len(design["parameters"]) > 0,
        "design.parameters must be a non-empty array",
    )

    names = set()
    for param in design["parameters"]:
        _validate_parameter(param)
        _require(param["name"] not in names, f"Duplicate parameter name '{param['name']}'")
        names.add(param["name"])

    for metric in spec.get("objectives", []):
        _validate_metric_def(metric, "objective")

    for metric in spec.get("constraints", []):
        _validate_metric_def(metric, "constraint")

    method = design.get("method", "lhs")
    _require(method in {"lhs", "sobol"}, "design.method must be one of: lhs, sobol")

    d = len(design["parameters"])
    defaults_n0 = max(12, 4 * d)
    defaults_nmax = max(40, 12 * d)
    replicated_center_runs = int(design.get("replicated_center_runs", 2))
    _require(replicated_center_runs >= 0, "design.replicated_center_runs must be >= 0")

    budget = spec.get("budget", {})
    _require(isinstance(budget, dict), "budget must be an object when provided")
    n0 = int(design.get("initial_runs", budget.get("n0", defaults_n0)))
    n_max = int(budget.get("n_max", defaults_nmax))
    _require(n0 > 0, "initial run count n0 must be > 0")
    _require(n_max >= n0 + replicated_center_runs, "n_max must be >= n0 + replicated_center_runs")

    normalized = deepcopy(spec)
    normalized.setdefault("results_root", "results/campaigns")
    normalized.setdefault("fixed_args", {})
    normalized.setdefault("constraints", [])
    normalized.setdefault("execution", {})
    normalized.setdefault("stop_rules", {})
    normalized["design"]["method"] = method
    normalized["design"]["replicated_center_runs"] = replicated_center_runs
    normalized["design"]["initial_runs"] = n0
    normalized.setdefault("budget", {})
    normalized["budget"]["n0"] = n0
    normalized["budget"]["n_max"] = n_max
    normalized["budget"]["defaults"] = {
        "n0_formula": "max(12, 4d)",
        "n_max_formula": "max(40, 12d)",
        "d": d,
    }
    normalized["validated_at"] = now_iso()
    return normalized


def _lhs_unit(n: int, d: int, seed: int) -> List[List[float]]:
    rng = random.Random(seed)
    samples = [[0.0 for _ in range(d)] for _ in range(n)]
    for j in range(d):
        strata = list(range(n))
        rng.shuffle(strata)
        for i in range(n):
            u = rng.random()
            samples[i][j] = (strata[i] + u) / n
    return samples


def _sobol_unit(n: int, d: int, seed: int) -> List[List[float]]:
    try:
        from scipy.stats import qmc  # type: ignore
    except Exception as exc:
        raise RuntimeError(
            "Sobol design requested but scipy is unavailable. Install scipy or use design.method='lhs'."
        ) from exc
    sampler = qmc.Sobol(d=d, scramble=True, seed=seed)
    m = int(math.ceil(math.log2(n)))
    points = sampler.random_base2(m=m)
    return points[:n].tolist()


def _scale_value(unit_x: float, param: Dict[str, Any]) -> Any:
    low, high = float(param["bounds"][0]), float(param["bounds"][1])
    raw = low + unit_x * (high - low)
    if param["type"] == "int":
        return int(round(raw))
    return float(raw)


def _generate_design_points(spec: Dict[str, Any]) -> Tuple[List[Dict[str, Any]], str]:
    params = spec["design"]["parameters"]
    d = len(params)
    n0 = int(spec["design"]["initial_runs"])
    seed = int(spec["design"].get("seed", 42))
    method = spec["design"]["method"]
    if method == "sobol":
        unit_points = _sobol_unit(n0, d, seed)
    else:
        unit_points = _lhs_unit(n0, d, seed)

    points: List[Dict[str, Any]] = []
    for unit_row in unit_points:
        points.append({p["name"]: _scale_value(unit_row[idx], p) for idx, p in enumerate(params)})
    return points, method


def _center_point(spec: Dict[str, Any]) -> Dict[str, Any]:
    out: Dict[str, Any] = {}
    for p in spec["design"]["parameters"]:
        low, high = float(p["bounds"][0]), float(p["bounds"][1])
        center = (low + high) / 2.0
        out[p["name"]] = int(round(center)) if p["type"] == "int" else center
    return out


def build_manifest(spec: Dict[str, Any], spec_path: Path, output_path: Path) -> Dict[str, Any]:
    design_points, method_used = _generate_design_points(spec)
    replicated_centers = int(spec["design"]["replicated_center_runs"])
    center = _center_point(spec)
    campaign_id = spec["campaign_id"]

    runs: List[Dict[str, Any]] = []
    idx = 0
    for p in design_points:
        runs.append(
            {
                "run_id": f"{campaign_id}-it00-r{idx:04d}",
                "design_iteration": 0,
                "run_type": method_used,
                "parameter_set": p,
                "status": "pending",
                "exit_code": None,
                "started_at": None,
                "ended_at": None,
                "results_dir": None,
                "quality_flags": [],
                "stdout_log": None,
                "stderr_log": None,
            }
        )
        idx += 1

    for _ in range(replicated_centers):
        runs.append(
            {
                "run_id": f"{campaign_id}-it00-r{idx:04d}",
                "design_iteration": 0,
                "run_type": "center_rep",
                "parameter_set": center,
                "status": "pending",
                "exit_code": None,
                "started_at": None,
                "ended_at": None,
                "results_dir": None,
                "quality_flags": [],
                "stdout_log": None,
                "stderr_log": None,
            }
        )
        idx += 1

    manifest = {
        "campaign_id": campaign_id,
        "spec_path": str(spec_path),
        "script_entrypoint": spec["script_entrypoint"],
        "results_root": spec["results_root"],
        "generated_at": now_iso(),
        "budget": spec["budget"],
        "design": {
            "method": method_used,
            "seed": int(spec["design"].get("seed", 42)),
            "initial_runs": int(spec["design"]["initial_runs"]),
            "replicated_center_runs": replicated_centers,
        },
        "fixed_args": spec.get("fixed_args", {}),
        "objectives": spec.get("objectives", []),
        "constraints": spec.get("constraints", []),
        "stop_rules": spec.get("stop_rules", {}),
        "runs": runs,
    }
    save_json(output_path, manifest)
    return manifest


def _powershell_args_from_mapping(mapping: Dict[str, Any]) -> List[str]:
    args: List[str] = []
    for key, value in mapping.items():
        if isinstance(value, bool):
            if value:
                args.append(f"-{key}")
            continue
        args.extend([f"-{key}", str(value)])
    return args


def execute_manifest(
    manifest_path: Path,
    repo_root: Path,
    dry_run: bool,
    max_runs: int,
    fail_fast: bool,
    allow_in_warp: bool,
) -> Dict[str, Any]:
    if not dry_run and in_warp_session() and not allow_in_warp and os.environ.get("MFREE_ALLOW_WARP_SIM", "") != "1":
        raise RuntimeError(
            "Simulation execution is blocked inside Warp for this repository. Open an external Windows PowerShell session and re-run from repo root. "
            "Override only when intentional via --allow-in-warp or MFREE_ALLOW_WARP_SIM=1."
        )
    manifest = load_json(manifest_path)
    campaign_dir = repo_root / manifest["results_root"] / manifest["campaign_id"]
    runs_dir = campaign_dir / "runs"
    logs_dir = campaign_dir / "artifacts" / "logs"
    runs_dir.mkdir(parents=True, exist_ok=True)
    logs_dir.mkdir(parents=True, exist_ok=True)

    script_path = repo_root / manifest["script_entrypoint"]
    if not script_path.exists():
        raise FileNotFoundError(f"Configured script entrypoint does not exist: {script_path}")

    processed = 0
    for run in manifest["runs"]:
        if max_runs > 0 and processed >= max_runs:
            break
        if run["status"] in {"completed", "failed", "skipped"}:
            continue

        run_id = run["run_id"]
        result_dir = runs_dir / run_id
        result_dir.mkdir(parents=True, exist_ok=True)
        run["results_dir"] = str(result_dir)
        run["started_at"] = now_iso()

        run_args: Dict[str, Any] = {}
        run_args.update(manifest.get("fixed_args", {}))
        run_args.update(run.get("parameter_set", {}))
        run_args["ResultsDir"] = str(result_dir)

        cmd = [
            "pwsh",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-File",
            str(script_path),
        ] + _powershell_args_from_mapping(run_args)

        stdout_log = logs_dir / f"{run_id}.stdout.log"
        stderr_log = logs_dir / f"{run_id}.stderr.log"
        run["stdout_log"] = str(stdout_log)
        run["stderr_log"] = str(stderr_log)

        if dry_run:
            run["status"] = "dry_run"
            run["exit_code"] = 0
            run["ended_at"] = now_iso()
            stdout_log.write_text("DRY RUN\n" + " ".join(cmd) + "\n", encoding="utf-8")
            stderr_log.write_text("", encoding="utf-8")
            processed += 1
            save_json(manifest_path, manifest)
            continue

        proc = subprocess.run(
            cmd,
            cwd=repo_root,
            text=True,
            capture_output=True,
        )
        stdout_log.write_text(proc.stdout or "", encoding="utf-8")
        stderr_log.write_text(proc.stderr or "", encoding="utf-8")
        run["exit_code"] = int(proc.returncode)
        run["ended_at"] = now_iso()
        run["status"] = "completed" if proc.returncode == 0 else "failed"
        if proc.returncode != 0:
            flags = set(run.get("quality_flags", []))
            flags.add("execution_failed")
            run["quality_flags"] = sorted(flags)
            if fail_fast:
                save_json(manifest_path, manifest)
                raise RuntimeError(f"Run failed: {run_id} (exit={proc.returncode})")

        processed += 1
        save_json(manifest_path, manifest)
    return manifest


def _read_csv_numeric_column(path: Path, column: str) -> List[float]:
    values: List[float] = []
    with path.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None or column not in reader.fieldnames:
            raise KeyError(column)
        for row in reader:
            raw = row.get(column, "")
            if raw is None or str(raw).strip() == "":
                continue
            values.append(float(raw))
    return values


def _reduce(values: List[float], reducer: str) -> float:
    if len(values) == 0:
        raise ValueError("no values")
    if reducer == "last":
        return values[-1]
    if reducer == "first":
        return values[0]
    if reducer == "max":
        return max(values)
    if reducer == "min":
        return min(values)
    if reducer == "mean":
        return sum(values) / len(values)
    raise ValueError(f"Unsupported reducer '{reducer}'")


def _constraint_ok(value: float, operator: str, threshold: float) -> bool:
    if operator == "<=":
        return value <= threshold
    if operator == ">=":
        return value >= threshold
    if operator == "<":
        return value < threshold
    if operator == ">":
        return value > threshold
    if operator == "==":
        return value == threshold
    raise ValueError(f"Unsupported constraint operator '{operator}'")


def normalize_results(manifest_path: Path, output_csv: Path) -> Path:
    manifest = load_json(manifest_path)
    rows: List[Dict[str, Any]] = []
    for run in manifest["runs"]:
        quality_flags = set(run.get("quality_flags", []))
        if run.get("status") == "failed":
            quality_flags.add("failed_run")
        if run.get("status") == "dry_run":
            quality_flags.add("dry_run")

        out: Dict[str, Any] = {
            "campaign_id": manifest["campaign_id"],
            "run_id": run["run_id"],
            "design_iteration": run.get("design_iteration"),
            "parameter_set": json.dumps(run.get("parameter_set", {}), sort_keys=True),
            "script_entrypoint": manifest["script_entrypoint"],
            "results_dir": run.get("results_dir"),
            "status": run.get("status"),
            "exit_code": run.get("exit_code"),
            "started_at": run.get("started_at"),
            "ended_at": run.get("ended_at"),
            "stdout_log": run.get("stdout_log"),
            "stderr_log": run.get("stderr_log"),
        }

        results_dir = Path(run["results_dir"]) if run.get("results_dir") else None

        for metric in manifest.get("objectives", []):
            name = metric["name"]
            reducer = metric.get("reducer", "last")
            out_key = f"objective_{name}"
            out[out_key] = None
            if results_dir is None:
                quality_flags.add(f"missing_results_dir:{name}")
                continue
            csv_path = results_dir / metric["source_file"]
            if not csv_path.exists():
                quality_flags.add(f"missing_file:{metric['source_file']}")
                continue
            try:
                values = _read_csv_numeric_column(csv_path, metric["column"])
                out[out_key] = _reduce(values, reducer)
            except KeyError:
                quality_flags.add(f"missing_column:{metric['column']}")
            except Exception:
                quality_flags.add(f"invalid_metric:{name}")

        for metric in manifest.get("constraints", []):
            name = metric["name"]
            reducer = metric.get("reducer", "last")
            val_key = f"constraint_{name}_value"
            ok_key = f"constraint_{name}_satisfied"
            out[val_key] = None
            out[ok_key] = None
            if results_dir is None:
                quality_flags.add(f"missing_results_dir:{name}")
                continue
            csv_path = results_dir / metric["source_file"]
            if not csv_path.exists():
                quality_flags.add(f"missing_file:{metric['source_file']}")
                continue
            try:
                values = _read_csv_numeric_column(csv_path, metric["column"])
                metric_value = _reduce(values, reducer)
                out[val_key] = metric_value
                out[ok_key] = _constraint_ok(
                    metric_value, metric["operator"], float(metric["threshold"])
                )
            except KeyError:
                quality_flags.add(f"missing_column:{metric['column']}")
            except Exception:
                quality_flags.add(f"invalid_constraint:{name}")

        out["quality_flags"] = ";".join(sorted(quality_flags))
        rows.append(out)

    output_csv.parent.mkdir(parents=True, exist_ok=True)
    fieldnames: List[str] = []
    for row in rows:
        for key in row.keys():
            if key not in fieldnames:
                fieldnames.append(key)
    with output_csv.open("w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)
    return output_csv


def default_manifest_path(spec: Dict[str, Any], repo_root: Path) -> Path:
    return (
        repo_root
        / spec["results_root"]
        / spec["campaign_id"]
        / "artifacts"
        / "manifest.json"
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="MVP DOE/ML campaign workflow: validate spec, generate manifest, execute runs, normalize outputs."
    )
    sub = parser.add_subparsers(dest="cmd", required=True)

    p_validate = sub.add_parser("validate-spec", help="Validate and normalize campaign spec")
    p_validate.add_argument("--spec", required=True, help="Path to campaign spec JSON")
    p_validate.add_argument(
        "--out",
        default="",
        help="Optional path for normalized spec JSON (defaults to <campaign>/artifacts/validated_spec.json)",
    )

    p_manifest = sub.add_parser("generate-manifest", help="Generate initial DOE manifest from spec")
    p_manifest.add_argument("--spec", required=True, help="Path to campaign spec JSON")
    p_manifest.add_argument(
        "--out",
        default="",
        help="Optional path for manifest JSON (defaults to <campaign>/artifacts/manifest.json)",
    )

    p_execute = sub.add_parser("execute-manifest", help="Execute manifest rows via PowerShell script adapter")
    p_execute.add_argument("--manifest", required=True, help="Path to manifest JSON")
    p_execute.add_argument("--dry-run", action="store_true", help="Render commands and mark dry_run only")
    p_execute.add_argument("--max-runs", type=int, default=0, help="Limit number of runs processed (0 = all pending)")
    p_execute.add_argument("--fail-fast", action="store_true", help="Stop on first failed run")
    p_execute.add_argument(
        "--allow-in-warp",
        action="store_true",
        help="Override external-only guard and allow non-dry simulation execution inside Warp",
    )

    p_norm = sub.add_parser("normalize-results", help="Build canonical dataset from manifest + run outputs")
    p_norm.add_argument("--manifest", required=True, help="Path to manifest JSON")
    p_norm.add_argument(
        "--out",
        default="",
        help="Optional output CSV path (defaults to <campaign>/artifacts/normalized_results.csv)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parents[1]

    if args.cmd == "validate-spec":
        spec_path = Path(args.spec).resolve()
        spec = validate_and_normalize_spec(load_json(spec_path))
        out = (
            Path(args.out).resolve()
            if args.out
            else (
                repo_root
                / spec["results_root"]
                / spec["campaign_id"]
                / "artifacts"
                / "validated_spec.json"
            )
        )
        save_json(out, spec)
        print(f"validated spec -> {out}")
        return 0

    if args.cmd == "generate-manifest":
        spec_path = Path(args.spec).resolve()
        spec = validate_and_normalize_spec(load_json(spec_path))
        manifest_path = (
            Path(args.out).resolve() if args.out else default_manifest_path(spec, repo_root)
        )
        manifest = build_manifest(spec, spec_path, manifest_path)
        print(
            f"generated manifest -> {manifest_path} "
            f"(initial_runs={manifest['design']['initial_runs']}, "
            f"replicated_center_runs={manifest['design']['replicated_center_runs']}, "
            f"total_runs={len(manifest['runs'])})"
        )
        return 0

    if args.cmd == "execute-manifest":
        manifest_path = Path(args.manifest).resolve()
        execute_manifest(
            manifest_path=manifest_path,
            repo_root=repo_root,
            dry_run=bool(args.dry_run),
            max_runs=int(args.max_runs),
            fail_fast=bool(args.fail_fast),
            allow_in_warp=bool(args.allow_in_warp),
        )
        print(f"execution updated manifest -> {manifest_path}")
        return 0

    if args.cmd == "normalize-results":
        manifest_path = Path(args.manifest).resolve()
        manifest = load_json(manifest_path)
        out_path = (
            Path(args.out).resolve()
            if args.out
            else (
                repo_root
                / manifest["results_root"]
                / manifest["campaign_id"]
                / "artifacts"
                / "normalized_results.csv"
            )
        )
        output_csv = normalize_results(manifest_path=manifest_path, output_csv=out_path)
        print(f"normalized dataset -> {output_csv}")
        return 0

    print("Unknown command", file=sys.stderr)
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
