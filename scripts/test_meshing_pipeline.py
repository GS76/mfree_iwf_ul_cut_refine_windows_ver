import io
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest import mock


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from Meshing.pipeline.cli import main as cli_main
from Meshing.pipeline.contracts import (
    MeshRefinementConfig,
    MeshSourceConfig,
    MeshSourceType,
    MeshingJobConfig,
)
from Meshing.pipeline.inputs import validate_job_config
from Meshing.pipeline.runner import MeshingPipelineError, run_meshing_job


def _make_tl_job(out_msh: str) -> MeshingJobConfig:
    return MeshingJobConfig(
        source=MeshSourceConfig(
            source_type=MeshSourceType.TL_ANGLES,
            tl_x=0.0,
            tl_y=0.0,
            length=1.0,
            height=1.0,
            rake_deg=5.0,
            clearance_deg=5.0,
            fillet_radius=0.01,
        ),
        refinement=MeshRefinementConfig(),
        out_msh=out_msh,
    )


class ValidateJobConfigTests(unittest.TestCase):
    def test_requires_existing_tool_txt_path(self) -> None:
        job = MeshingJobConfig(
            source=MeshSourceConfig(
                source_type=MeshSourceType.TOOL_TXT,
                tool_txt="C:/definitely_missing_tool_file.txt",
            ),
            refinement=MeshRefinementConfig(),
            out_msh="Meshing/out/tool.msh",
        )
        with self.assertRaisesRegex(ValueError, "tool_txt file not found"):
            validate_job_config(job)

    def test_requires_refine_center_pair(self) -> None:
        job = _make_tl_job("Meshing/out/tool.msh")
        bad_job = MeshingJobConfig(
            source=job.source,
            refinement=MeshRefinementConfig(refine_center_x=1.0, refine_center_y=None),
            out_msh=job.out_msh,
        )
        with self.assertRaisesRegex(ValueError, "refine_center_x and refine_center_y"):
            validate_job_config(bad_job)


class RunnerTests(unittest.TestCase):
    def test_runner_returns_result_payload(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            out_msh = str(Path(td) / "tool.msh")
            out_tool_meta = str(Path(td) / "tool_meta.json")
            job = _make_tl_job(out_msh)
            job = MeshingJobConfig(
                source=job.source,
                refinement=job.refinement,
                out_msh=job.out_msh,
                out_tool_meta=out_tool_meta,
            )

            fake_geom = SimpleNamespace(
                fillet_center=SimpleNamespace(x=0.25, y=0.75),
                vertices=[],
            )
            fake_report = {"mesh": {"nodes": 42, "triangles": 24}}
            fake_meta = {"num_vertices": 5}

            with (
                mock.patch("Meshing.pipeline.runner.validate_job_config") as m_validate,
                mock.patch("Meshing.pipeline.runner._resolve_geometry", return_value=fake_geom),
                mock.patch("Meshing.pipeline.runner._tool_metadata", return_value=fake_meta),
                mock.patch("Meshing.pipeline.runner._gmsh_build_and_mesh", return_value=fake_report) as m_mesh,
            ):
                result = run_meshing_job(job)

            m_validate.assert_called_once()
            self.assertEqual(result["mesh"]["nodes"], 42)
            self.assertEqual(result["tool"]["num_vertices"], 5)
            self.assertTrue(result["outputs"]["msh"].endswith("tool.msh"))
            self.assertIsNotNone(result["refinement_center"])
            self.assertTrue(Path(out_tool_meta).is_file())
            mesh_kwargs = m_mesh.call_args.kwargs
            self.assertAlmostEqual(mesh_kwargs["refine_center"].x, 0.25)
            self.assertAlmostEqual(mesh_kwargs["refine_center"].y, 0.75)

    def test_runner_wraps_internal_exception(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            out_msh = str(Path(td) / "tool.msh")
            job = _make_tl_job(out_msh)
            fake_geom = SimpleNamespace(
                fillet_center=SimpleNamespace(x=0.25, y=0.75),
                vertices=[],
            )
            with (
                mock.patch("Meshing.pipeline.runner.validate_job_config"),
                mock.patch("Meshing.pipeline.runner._resolve_geometry", return_value=fake_geom),
                mock.patch("Meshing.pipeline.runner._tool_metadata", return_value={"ok": True}),
                mock.patch("Meshing.pipeline.runner._gmsh_build_and_mesh", side_effect=RuntimeError("boom")),
            ):
                with self.assertRaisesRegex(MeshingPipelineError, "Meshing pipeline failed"):
                    run_meshing_job(job)


class CliTests(unittest.TestCase):
    def test_cli_success_and_json_output(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            tool_path = Path(td) / "tool.txt"
            tool_path.write_text("dummy", encoding="utf-8")
            out_msh = Path(td) / "tool.msh"
            out_json = Path(td) / "result.json"
            argv = [
                "run_pipeline.py",
                "--tool-txt",
                str(tool_path),
                "--out-msh",
                str(out_msh),
                "--json-output",
                str(out_json),
            ]
            fake_result = {"tool": {"a": 1}, "mesh": {"nodes": 1}}
            with (
                mock.patch.object(sys, "argv", argv),
                mock.patch("Meshing.pipeline.cli.run_meshing_job", return_value=fake_result),
            ):
                rc = cli_main()
            self.assertEqual(rc, 0)
            self.assertTrue(out_json.is_file())
            data = json.loads(out_json.read_text(encoding="utf-8"))
            self.assertEqual(data["mesh"]["nodes"], 1)

    def test_cli_failure_returns_nonzero(self) -> None:
        argv = [
            "run_pipeline.py",
            "--tool-txt",
            "missing.txt",
            "--out-msh",
            "Meshing/out/tool.msh",
        ]
        stderr_capture = io.StringIO()
        with (
            mock.patch.object(sys, "argv", argv),
            mock.patch("Meshing.pipeline.cli.run_meshing_job", side_effect=MeshingPipelineError("fail")),
            mock.patch("sys.stderr", stderr_capture),
        ):
            rc = cli_main()
        self.assertEqual(rc, 2)
        self.assertIn("fail", stderr_capture.getvalue())

    def test_cli_ignores_non_integer_swap_env(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            tool_path = Path(td) / "tool.txt"
            tool_path.write_text("dummy", encoding="utf-8")
            out_msh = Path(td) / "tool.msh"
            argv = [
                "run_pipeline.py",
                "--tool-txt",
                str(tool_path),
                "--out-msh",
                str(out_msh),
            ]
            with (
                mock.patch.object(sys, "argv", argv),
                mock.patch.dict(os.environ, {"MFREE_SWAP_TOOL_RAKE_CLEARANCE": "abc"}, clear=False),
                mock.patch("Meshing.pipeline.cli.run_meshing_job", return_value={"tool": {}, "mesh": {}}),
            ):
                rc = cli_main()
            self.assertEqual(rc, 0)


if __name__ == "__main__":
    raise SystemExit(0 if unittest.main(verbosity=2).result.wasSuccessful() else 1)
