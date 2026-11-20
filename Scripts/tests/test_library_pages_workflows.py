# SPDX-License-Identifier: CC0-1.0
from copy import deepcopy
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

import yaml

ROOT = Path(__file__).resolve().parents[2]


def workflow(name):
    return yaml.safe_load((ROOT / ".github/workflows" / name).read_text())


def events(value):
    # PyYAMLのYAML1.1ではonがboolとして読まれる。Actionsのキー自体はon。
    return value.get("on", value.get(True))


def python_block(script, marker):
    beginning = "python3 - <<'" + marker + "'\n"
    if beginning not in script:
        raise AssertionError("expected inline Python helper")
    return script.split(beginning, 1)[1].split("\n" + marker, 1)[0] + "\n"


def run_record(identifier=100, number=9, conclusion="failure"):
    return {"id": identifier, "run_number": number, "run_attempt": 2,
            "event": "pull_request", "status": "completed", "head_sha": "a" * 40,
            "repository": {"full_name": "example/library"}, "conclusion": conclusion,
            "pull_requests": [{"number": 17, "head": {"sha": "a" * 40}, "base": {"ref": "main"}}]}


def artifact(identifier=200, name="contracts-g++-native", *, expired=False, run=100, head=None):
    return {"id": identifier, "name": name, "expired": expired,
            "workflow_run": {"id": run, "head_sha": head or "a" * 40},
            "digest": "sha256:" + "c" * 64}


class StructureTests(unittest.TestCase):
    def test_workflows_only_run_for_main_pull_requests(self):
        verify = events(workflow("verify.yml"))
        pages = events(workflow("library-pages.yml"))
        self.assertEqual(set(verify), {"pull_request"})
        self.assertEqual(set(pages), {"pull_request"})
        self.assertEqual(verify["pull_request"]["branches"], ["main"])
        self.assertEqual(pages["pull_request"], {"branches": ["main"], "types": ["closed"]})
        self.assertNotIn("closed", verify["pull_request"].get("types", []))

    def test_unmerged_preview_has_no_pages_write_or_deploy_action(self):
        value = workflow("verify.yml")
        self.assertEqual(value["permissions"], {"contents": "read"})
        for job in value["jobs"].values():
            self.assertNotIn("write", job.get("permissions", {}).values())
            for step in job.get("steps", []):
                self.assertNotIn(step.get("uses", "").split("@")[0], {
                    "actions/deploy-pages", "actions/upload-pages-artifact", "actions/configure-pages"})

    def test_publication_checks_merged_main_and_uses_reviewed_merge_commit(self):
        value = workflow("library-pages.yml")
        build = value["jobs"]["build"]
        for name in ("build", "deploy"):
            guard = value["jobs"][name]["if"]
            self.assertIn("pull_request.merged == true", guard)
            self.assertIn("pull_request.base.ref == 'main'", guard)
            self.assertIn("pull_request.base.repo.full_name == github.repository", guard)
        checkout = next(step for step in build["steps"] if step.get("uses", "").startswith("actions/checkout@"))
        self.assertEqual(checkout["with"]["ref"], "${{ github.event.pull_request.merge_commit_sha }}")
        self.assertFalse(checkout["with"]["persist-credentials"])
        self.assertEqual(build["permissions"], {"contents": "read", "actions": "read", "pages": "read"})
        deploy = value["jobs"]["deploy"]
        self.assertIn("needs.build.result == 'success'", deploy["if"])
        self.assertEqual(deploy["environment"]["name"], "github-pages")
        self.assertEqual(deploy["permissions"]["pages"], "write")
        self.assertEqual(deploy["permissions"]["id-token"], "write")
        self.assertFalse(any(step.get("uses", "").startswith("actions/checkout@") for step in deploy["steps"]))
        configure = next(step for step in build["steps"] if step.get("uses", "").startswith("actions/configure-pages@"))
        self.assertIs(configure["with"]["enablement"], False)
        self.assertNotEqual(value["concurrency"]["group"], workflow("verify.yml")["concurrency"]["group"])

    def test_preview_waits_for_evidence_even_when_individual_checks_fail(self):
        value = workflow("verify.yml")
        docs = value["jobs"]["documentation"]
        self.assertEqual(set(docs["needs"]), {"maintenance", "examples", "instrumented", "performance", "all_performance", "contracts", "library_checker", "stress"})
        self.assertIn("!cancelled()", docs["if"])
        self.assertIn("needs.maintenance.result == 'success'", docs["if"])
        self.assertNotIn("needs.examples.result == 'success'", docs["if"])
        downloads = [step for step in docs["steps"] if step.get("uses", "").startswith("actions/download-artifact@")]
        self.assertEqual({step["with"]["pattern"] for step in downloads}, {
            "local-verify-*", "instrumented-*", "representative-performance", "all-library-performance-*", "contracts-*", "library-checker-*"})
        for step in downloads:
            self.assertNotIn("run-id", step["with"])
            self.assertFalse(step["with"]["merge-multiple"])
            self.assertEqual(step["with"]["path"], "${{ runner.temp }}/pages-input")
        command = next(step["run"] for step in docs["steps"] if step.get("name") == "Build preview with source-bound reports")
        self.assertIn("Scripts/build_library_pages.py", command)
        self.assertIn('--report "$RUNNER_TEMP/pages-input"', command)
        self.assertIn("--render", command)
        self.assertNotIn("--production", command)

    def test_contract_matrix_and_official_checker_shards_are_explicit(self):
        jobs = workflow("verify.yml")["jobs"]
        self.assertEqual(jobs["contracts"]["strategy"]["matrix"], {"compiler": ["g++", "clang++"], "arithmetic": ["native", "portable"]})
        self.assertEqual(jobs["library_checker"]["strategy"]["matrix"]["shard"], list(range(8)))
        command = next(step["run"] for step in jobs["library_checker"]["steps"] if step.get("name") == "Run all official full cases")
        self.assertIn('--base "$VERIFY_PR_BASE"', command)
        self.assertIn("--shard-count 8", command)
        self.assertIn("--all", command)
        self.assertIn("--discard-test-data", command)
        self.assertIn("--compiler g++-15", command)
        self.assertIn('--cache "$RUNNER_TEMP/library-checker-cache"', command)
        upload = next(step for step in jobs["library_checker"]["steps"] if step.get("name") == "Upload metadata only")
        self.assertEqual(upload["with"]["path"], "${{ runner.temp }}/library-checker-report/result.json")
        self.assertNotIn("cache", upload["with"]["path"])

    def test_final_gate_includes_contracts_official_cases_and_documentation(self):
        gate = workflow("verify.yml")["jobs"]["result"]
        self.assertEqual(set(gate["needs"]), {"maintenance", "examples", "variant_builds", "instrumented", "guidelines", "performance", "all_performance", "contracts", "library_checker", "stress", "documentation"})
        command = gate["steps"][0]["run"]
        for variable in ("CONTRACT_RESULT", "LIBRARY_CHECKER_RESULT", "STRESS_RESULT", "DOCUMENTATION_RESULT",
                         "ALL_PERFORMANCE_RESULT", "VARIANT_BUILDS_RESULT"):
            self.assertIn('"$' + variable + '" == success', command)
        self.assertEqual(gate["steps"][0]["env"]["ALL_PERFORMANCE_RESULT"], "${{ needs.all_performance.result }}")

    def test_atcoder_verify_is_an_additional_gcc_configuration(self):
        job = workflow("verify.yml")["jobs"]["examples"]
        self.assertEqual(job["strategy"]["matrix"], {
            "compiler": ["g++", "clang++"], "arithmetic": ["native", "portable"],
            "compile_profile": ["standard"],
            "include": [{"compiler": "g++", "arithmetic": "native", "compile_profile": "atcoder"}],
        })
        run = next(step for step in job["steps"] if step.get("name") == "Select and run local Verify")
        self.assertEqual(run["env"]["VERIFY_COMPILE_PROFILE"], "${{ matrix.compile_profile }}")
        self.assertIn('--compile-profile "$VERIFY_COMPILE_PROFILE"', run["run"])
        upload = next(step for step in job["steps"] if step.get("name") == "Upload local Verify report")
        self.assertIn("'atcoder-local-verify' || 'local-verify'", upload["with"]["name"])

    def test_variant_build_matrix_runs_native_msvc_on_windows(self):
        job = workflow("verify.yml")["jobs"]["variant_builds"]
        self.assertEqual(job["needs"], "maintenance")
        self.assertEqual(job["strategy"]["matrix"]["include"], [
            {"os": "ubuntu-26.04", "compiler": "GCC"}, {"os": "ubuntu-26.04", "compiler": "Clang"},
            {"os": "windows-2025", "compiler": "MSVC"},
        ])
        windows = next(step for step in job["steps"] if "on Windows" in step.get("name", ""))
        self.assertEqual(windows["shell"], "cmd")
        self.assertIn("SIRKIT_VS_DEVELOPER_COMMAND", windows["run"])
        self.assertIn("--compiler MSVC --syntax-only --run-tests", windows["run"])
        linux = next(step for step in job["steps"] if "on Linux" in step.get("name", ""))
        self.assertIn("--gcc g++-15 --clang clang++-18", linux["run"])
        self.assertIn("--run-tests", linux["run"])
        self.assertIn("--summary", linux["run"])
        self.assertIn("--summary", windows["run"])

    def test_all_library_performance_shards_compare_full_scope_and_upload_only_metadata(self):
        job = workflow("verify.yml")["jobs"]["all_performance"]
        self.assertEqual(job["needs"], "maintenance")
        self.assertEqual(job["timeout-minutes"], 90)
        self.assertFalse(job["strategy"]["fail-fast"])
        self.assertEqual(job["strategy"]["matrix"], {"shard": list(range(8)), "compile_profile": ["standard", "atcoder"]})
        steps = job["steps"]
        install = next(step["run"] for step in steps if step.get("name") == "Install compiler and headers")
        self.assertIn("g++-15 libboost-dev", install)
        comparison = next(step for step in steps if step.get("name") == "Compare all Verify programs against the PR base")
        self.assertEqual(comparison["env"]["VERIFY_PR_BASE"], "${{ github.event.pull_request.base.sha }}")
        command = comparison["run"]
        self.assertIn("Scripts/ci_performance_all.py", command)
        self.assertIn("--compiler g++-15", command)
        self.assertIn('--compile-profile "$VERIFY_COMPILE_PROFILE"', command)
        self.assertIn('--shard-index "$VERIFY_SHARD" --shard-count 8', command)
        self.assertIn('--cache "$RUNNER_TEMP/all-library-performance-cache"', command)
        self.assertIn('--output "$RUNNER_TEMP/all-library-performance"', command)
        self.assertIn('--base "$VERIFY_PR_BASE"', command)
        for option in ("--limits", "--quick", "--library", "--plan"):
            self.assertNotIn(option, command)
        upload = next(step for step in steps if step.get("uses", "").startswith("actions/upload-artifact@"))
        self.assertEqual(upload["if"], "always()")
        self.assertIn("'atcoder-all-library-performance' || 'all-library-performance'", upload["with"]["name"])
        self.assertTrue(upload["with"]["name"].endswith("-${{ matrix.shard }}"))
        self.assertEqual(upload["with"]["path"].splitlines(), [
            "${{ runner.temp }}/all-library-performance/result.json",
            "${{ runner.temp }}/all-library-performance/summary.md"])
        provenance = next(step for step in steps if step.get("name") == "Attach report provenance")
        self.assertEqual(provenance["if"], "always()")
        self.assertEqual(provenance["env"]["REPORT_FILE"], "${{ runner.temp }}/all-library-performance/result.json")
        self.assertIn("pull_request_head_sha", python_block(provenance["run"], "PYPROVENANCE"))
        summary = next(step for step in steps if step.get("name") == "Summarize all-library performance")
        self.assertEqual(summary["if"], "always()")
        self.assertIn('>> "$GITHUB_STEP_SUMMARY"', summary["run"])

    def test_kernel_performance_regression_gate_rejects_incomplete_reports(self):
        job = workflow("verify.yml")["jobs"]["performance"]
        self.assertEqual(job["strategy"]["matrix"], {"compile_profile": ["standard", "atcoder"]})
        comparison = next(step for step in job["steps"] if step.get("name") == "Compare library kernels against the PR base")
        self.assertEqual(comparison["env"]["VERIFY_PR_BASE"], "${{ github.event.pull_request.base.sha }}")
        command = comparison["run"]
        self.assertIn('--base "$VERIFY_PR_BASE"', command)
        self.assertIn("--compiler g++-15", command)
        self.assertIn('--compile-profile "$VERIFY_COMPILE_PROFILE"', command)
        self.assertNotIn("--limits", command)
        code = python_block(command, "PYREGRESSION")
        baseline = "a" * 40
        good = {"status": "PASS", "compile_profile": "atcoder", "regression_policy": "library_worst_case",
                "current_measurements_passed": True, "performance_regression_passed": True,
                "base_revision": baseline, "expected_cases": 2, "compared_cases": 2, "results": [{}, {}],
                "sources": {"fenwick": {}},
                "worst_case_regressions": {"fenwick": {"status": "PASS", "compared_cases": 2}}}
        fixtures = [(good, 0)]
        for mutation in ({"status": "PARTIAL"}, {"status": "NO_BASELINE"},
                         {"status": "CURRENT_ONLY"}, {"performance_regression_passed": False},
                         {"regression_policy": None}, {"worst_case_regressions": {}},
                         {"worst_case_regressions": {"fenwick": {"status": "FAIL", "compared_cases": 2}}},
                         {"worst_case_regressions": {"fenwick": {"status": "PASS", "compared_cases": 1}}},
                         {"compile_profile": "standard"},
                         {"base_revision": "b" * 40}, {"compared_cases": 1},
                         {"current_measurements_passed": False}, {"expected_cases": 3},
                         {"expected_cases": True}, {"expected_cases": 0, "results": []},
                         {"results": None}):
            fixtures.append(({**good, **mutation}, 1))
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "performance/result.json"
            path.parent.mkdir()
            env = {**os.environ, "RUNNER_TEMP": directory, "VERIFY_PR_BASE": baseline, "VERIFY_COMPILE_PROFILE": "atcoder"}
            for report, expected in fixtures:
                with self.subTest(report=report):
                    path.write_text(json.dumps(report))
                    actual = subprocess.run([sys.executable, "-c", code], env=env, capture_output=True)
                    self.assertEqual(actual.returncode, expected, actual.stderr)
        upload = next(step for step in job["steps"] if step.get("uses", "").startswith("actions/upload-artifact@"))
        self.assertEqual(upload["with"]["path"].splitlines(), [
            "${{ runner.temp }}/performance/result.json", "${{ runner.temp }}/performance/summary.md"])

    def test_max_cases_run_once_outside_matrices_and_quick_is_not_used_in_ci(self):
        jobs = workflow("verify.yml")["jobs"]
        stress = jobs["stress"]
        self.assertNotIn("strategy", stress)
        command = next(step["run"] for step in stress["steps"] if step.get("name") == "Run deterministic full-size cases")
        self.assertIn("Scripts/ci_stress.py", command)
        self.assertNotIn("--quick", command)
        self.assertNotIn("--case ", command)
        performance = next(step["run"] for step in jobs["performance"]["steps"] if step.get("name") == "Compare library kernels against the PR base")
        self.assertNotIn("--quick", performance)
        self.assertNotIn("--id ", performance)
        upload = next(step for step in stress["steps"] if step.get("uses", "").startswith("actions/upload-artifact@"))
        self.assertNotIn(".in", upload["with"]["path"])
        self.assertNotIn(".out", upload["with"]["path"])

    def test_documentation_tools_are_pinned_and_reports_are_outside_checkout(self):
        self.assertEqual((ROOT / ".competitive-verifier/requirements.txt").read_text().strip(), "competitive-verifier==4.2.2")
        for steps in [workflow("verify.yml")["jobs"]["documentation"]["steps"], workflow("library-pages.yml")["jobs"]["build"]["steps"]]:
            python = next(step for step in steps if step.get("uses", "").startswith("actions/setup-python@"))
            ruby = next(step for step in steps if step.get("uses", "").startswith("ruby/setup-ruby@"))
            self.assertEqual(python["with"]["python-version"], "3.12")
            self.assertEqual(ruby["with"], {"ruby-version": "3.3", "bundler": "2.6.9"})
            self.assertTrue(any("pip install -r .competitive-verifier/requirements.txt" in step.get("run", "") for step in steps))
        production = next(step["run"] for step in workflow("library-pages.yml")["jobs"]["build"]["steps"] if step.get("name") == "Build the merged main site")
        self.assertIn("--production", production)
        self.assertIn('--output "$RUNNER_TEMP/library-pages"', production)


class ProvenanceTests(unittest.TestCase):
    def helper(self):
        step = next(step for step in workflow("verify.yml")["jobs"]["examples"]["steps"] if step.get("name") == "Attach report provenance")
        return python_block(step["run"], "PYPROVENANCE")

    def test_append_execution_origin_preserves_source_sha_results_and_status(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "result.json"
            original = {"status": "FAIL", "checked": True, "source_hashes": {"Graph/Low_Link.cpp": "a" * 64}, "results": [], "provenance": {"existing": "retained"}}
            path.write_text(json.dumps(original))
            environment = dict(os.environ, REPORT_FILE=str(path), GITHUB_SHA="b" * 40,
                               GITHUB_RUN_ID="123", GITHUB_RUN_ATTEMPT="2", GITHUB_REPOSITORY="example/library",
                               GITHUB_JOB="examples", SIRKIT_PR_HEAD_SHA="c" * 40)
            subprocess.run([sys.executable, "-c", self.helper()], env=environment, check=True)
            actual = json.loads(path.read_text())
            self.assertEqual({key: value for key, value in actual.items() if key != "provenance"}, {key: value for key, value in original.items() if key != "provenance"})
            self.assertEqual(actual["provenance"]["existing"], "retained")
            self.assertEqual(actual["provenance"]["github_run_id"], "123")
            self.assertEqual(actual["provenance"]["pull_request_head_sha"], "c" * 40)

    def test_missing_report_is_not_fabricated(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "missing.json"
            subprocess.run([sys.executable, "-c", self.helper()], env=dict(os.environ, REPORT_FILE=str(path)), check=True)
            self.assertFalse(path.exists())


class ArtifactSelectionTests(unittest.TestCase):
    def helper(self):
        step = next(step for step in workflow("library-pages.yml")["jobs"]["build"]["steps"] if step.get("id") == "reports")
        return python_block(step["run"], "PYREPORTS")

    def execute(self, runs, artifacts, *, failure=False, overrides=None):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "bin").mkdir()
            (root / "runs.json").write_text(json.dumps([{"workflow_runs": runs}]))
            (root / "artifacts.json").write_text(json.dumps([{"artifacts": artifacts}]))
            gh = root / "bin/gh"
            gh.write_text("#!/usr/bin/env python3\nimport json,os,sys\nfrom pathlib import Path\nroot=Path(os.environ['FAKE_GH_ROOT'])\nwith(root/'calls.jsonl').open('a')as out:out.write(json.dumps(sys.argv[1:])+'\\n')\nif os.environ.get('FAKE_GH_FAILURE')=='1':sys.exit(1)\nname='runs.json'if any('verify.yml/runs'in arg for arg in sys.argv)else'artifacts.json'\nprint((root/name).read_text())\n")
            gh.chmod(0o700)
            environment = dict(os.environ, PATH=str(root / "bin") + os.pathsep + os.environ["PATH"],
                               FAKE_GH_ROOT=str(root), FAKE_GH_FAILURE="1" if failure else "0",
                               PAGES_REPOSITORY="example/library", PAGES_PR_HEAD="a" * 40,
                               PAGES_PR_NUMBER="17", PAGES_MERGE_COMMIT="b" * 40,
                               RUNNER_TEMP=str(root), GITHUB_OUTPUT=str(root / "output"), GH_TOKEN="not-a-real-token")
            environment.update(overrides or {})
            process = subprocess.run([sys.executable, "-c", self.helper()], env=environment, capture_output=True, text=True)
            metadata = json.loads((root / "pages-download-provenance.json").read_text()) if (root / "pages-download-provenance.json").exists() else None
            output = (root / "output").read_text() if (root / "output").exists() else ""
            calls = [json.loads(line) for line in (root / "calls.jsonl").read_text().splitlines()] if (root / "calls.jsonl").exists() else []
            return process, metadata, output, calls

    def test_exact_head_and_pr_choose_latest_completed_run_without_assuming_pass(self):
        wrong_head = run_record(101, 100)
        wrong_head["head_sha"] = "d" * 40
        active = run_record(102, 101)
        active["status"] = "in_progress"
        other_pr = run_record(103, 102)
        other_pr["pull_requests"][0]["number"] = 18
        runs = [run_record(50, 8, "success"), run_record(), wrong_head, active, other_pr]
        artifacts = [artifact(200), artifact(201), artifact(202, expired=True),
                     artifact(203, name="library-pages-preview"), artifact(204, run=999),
                     artifact(205, name="instrumented-coverage")]
        process, metadata, output, calls = self.execute(runs, artifacts)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertEqual(metadata["selection_status"], "AVAILABLE")
        self.assertEqual(metadata["run"]["id"], 100)
        self.assertEqual(metadata["run"]["conclusion"], "failure")
        self.assertEqual(metadata["requested_head_sha"], "a" * 40)
        self.assertEqual({row["id"] for row in metadata["artifacts"]}, {201, 205})
        self.assertIn("run_id=100", output)
        self.assertIn("artifact_ids=201,205", output)
        self.assertIn("head_sha=" + "a" * 40, calls[0])
        self.assertTrue(all("--method" in call and "GET" in call for call in calls))
        self.assertTrue(all("not-a-real-token" not in arg for call in calls for arg in call))

    def test_no_matching_run_is_unavailable_with_an_empty_existing_report_directory(self):
        wrong = run_record()
        wrong["head_sha"] = "c" * 40
        process, metadata, output, calls = self.execute([wrong], [artifact()])
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertEqual(metadata["selection_status"], "UNAVAILABLE")
        self.assertIsNone(metadata["run"])
        self.assertIn("artifact_ids=\n", output)
        self.assertIn("::warning::", process.stdout)
        self.assertEqual(len(calls), 1)

    def test_all_library_performance_artifacts_allow_only_eight_exact_shards(self):
        valid = [artifact(300 + i, name="all-library-performance-" + str(i)) for i in range(8)]
        invalid = [artifact(400 + i, name=name) for i, name in enumerate((
            "all-library-performance-8", "all-library-performance--1",
            "all-library-performance-00", "all-library-performance-7-cache",
            "all-library-performance-0/../../outside"))]
        process, metadata, output, calls = self.execute([run_record()], valid + invalid)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertEqual({row["name"] for row in metadata["artifacts"]},
                         {"all-library-performance-" + str(i) for i in range(8)})
        self.assertEqual({row["id"] for row in metadata["artifacts"]}, set(range(300, 308)))

    def test_api_failure_is_recorded_without_inventing_results(self):
        process, metadata, output, calls = self.execute([run_record()], [artifact()], failure=True)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertEqual(metadata["selection_status"], "UNAVAILABLE")
        self.assertEqual(metadata["error_kind"], "CalledProcessError")
        self.assertEqual(metadata["artifacts"], [])
        self.assertIn("artifact_ids=\n", output)
        self.assertNotIn("not-a-real-token", json.dumps(metadata))

    def test_expired_wrong_origin_unrecognized_and_invalid_artifact_ids_are_not_downloaded(self):
        artifacts = [artifact(1, expired=True), artifact(2, head="d" * 40),
                     artifact(3, name="../../outside"), artifact(True), artifact(5, run=999)]
        process, metadata, output, calls = self.execute([run_record()], artifacts)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertEqual(metadata["artifacts"], [])
        self.assertEqual(metadata["selection_status"], "UNAVAILABLE")
        self.assertIn("artifact_ids=\n", output)

    def test_untrusted_api_rows_or_event_fields_cannot_select_a_run(self):
        wrong = run_record()
        wrong["repository"] = {"full_name": "other/library"}
        process, metadata, output, calls = self.execute([wrong], [artifact()])
        self.assertEqual(process.returncode, 0)
        self.assertIsNone(metadata["run"])
        for overrides in ({"PAGES_REPOSITORY": "x;touch injected"}, {"PAGES_PR_HEAD": "$(touch injected)"}):
            with self.subTest(overrides=overrides):
                process, metadata, output, calls = self.execute([run_record()], [artifact()], overrides=overrides)
                self.assertNotEqual(process.returncode, 0)
                self.assertIsNone(metadata)
                self.assertEqual(calls, [])


if __name__ == "__main__":
    unittest.main()
