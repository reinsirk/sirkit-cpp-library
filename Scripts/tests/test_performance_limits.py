# SPDX-License-Identifier: CC0-1.0
import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from Scripts import performance_limits as budgets


def configuration():
    return {"format": 1, "baseline": {"run_id": "fixed-fixture"},
            "policy": {"multiplier": 2, "whole_program_minimum_ns": 10_000_000},
            "limits": [
                {"scope": "whole_program", "library": "Graph/A.cpp", "verify_id": "a", "case": "example_00",
                 "metric": "child_cpu_ns", "input_bytes": 2, "baseline_median_ns": 1_000_000,
                 "limit_ns": 10_000_000},
                {"scope": "whole_program", "library": "Graph/A.cpp", "verify_id": "a", "case": "max_00",
                 "metric": "child_cpu_ns", "input_bytes": 100, "baseline_median_ns": 8_000_000,
                 "limit_ns": 16_000_000},
                {"scope": "kernel", "library": "Graph/A.cpp", "kernel_id": "a", "case": "random",
                 "metric": "kernel_elapsed_ns", "size": 1024, "pattern": 0, "rounds": 256,
                 "baseline_median_ns": 100, "limit_ns": 200}]}


class FixedLimitsTests(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.path = Path(self.folder.name) / "limits.json"

    def load(self, value=None):
        self.path.write_text(json.dumps(configuration() if value is None else value))
        return budgets.load(self.path)

    def test_floor_and_no_kernel_floor_match_the_saved_policy(self):
        limits = self.load()
        low = limits.whole("Graph/A.cpp", "a", "example_00", 2)
        high = limits.whole("Graph/A.cpp", "a", "max_00", 100)
        kernel = limits.kernel("Graph/A.cpp", "a", "random", 1024, 0, 256)
        self.assertEqual([low["limit_ns"], high["limit_ns"], kernel["limit_ns"]], [10_000_000, 16_000_000, 200])

    def test_inclusive_boundary_and_median_exclude_single_outlier(self):
        limits = self.load()
        entry = limits.kernel("Graph/A.cpp", "a", "random", 1024, 0, 256)
        self.assertEqual(limits.evaluate(entry, [1, 200, 1000])["status"], "PASS")
        gate = limits.evaluate(entry, [1, 201, 1000])
        self.assertEqual(gate["status"], "FAIL")
        self.assertEqual(gate["observed_median_ns"], 201)
        self.assertEqual(gate["config_sha256"], limits.sha256)
        self.assertEqual(limits.evaluate(entry, [200, 201])["status"], "FAIL")

    def test_measurements_never_rewrite_or_raise_the_budget(self):
        limits = self.load()
        before = self.path.read_bytes()
        entry = limits.kernel("Graph/A.cpp", "a", "random", 1024, 0, 256)
        for sample in (1000, 10_000_000_000, 1):
            limits.evaluate(entry, [sample])
        self.assertEqual(self.path.read_bytes(), before)
        self.assertEqual(limits.kernel("Graph/A.cpp", "a", "random", 1024, 0, 256)["limit_ns"], 200)

    def test_repository_metadata_is_relocatable_but_freeze_uses_the_actual_file(self):
        with patch.object(budgets, "__file__", str(self.path.parent / "Scripts/performance_limits.py")):
            limits = self.load()
        self.assertEqual(limits.metadata["path"], "limits.json")
        self.assertEqual(limits.path, self.path.resolve())
        limits.check_unchanged()

    def test_missing_case_and_changed_input_scale_fail_closed(self):
        limits = self.load()
        for request in [("Graph/A.cpp", "a", "missing", 2), ("Graph/A.cpp", "a", "example_00", 3)]:
            with self.assertRaises(budgets.LimitError): limits.whole(*request)
        for request in [("Graph/A.cpp", "a", "random", 2048, 0, 256),
                        ("Graph/A.cpp", "a", "random", 1024, 0, 1),
                        ("Graph/A.cpp", "a", "random", 1024, 1, 256)]:
            with self.assertRaises(budgets.LimitError): limits.kernel(*request)

    def test_malformed_duplicate_and_unapproved_caps_are_rejected(self):
        for change in (lambda d: d["limits"].append(copy.deepcopy(d["limits"][0])),
                       lambda d: d["limits"][0].update(limit_ns=20_000_000),
                       lambda d: d["limits"][0].update(baseline_median_ns=True),
                       lambda d: d["limits"][0].update(input_bytes=-1),
                       lambda d: d["limits"][0].update(metric="wall_ns"),
                       lambda d: d["limits"][0].update(library="../A.cpp"),
                       lambda d: d["limits"][2].update(pattern=4),
                       lambda d: d["policy"].update(multiplier=3),
                       lambda d: d.update(limits=[])):
            value = configuration(); change(value)
            with self.subTest(value=value), self.assertRaises(budgets.LimitError): self.load(value)

    def test_runtime_budget_mutation_and_invalid_samples_are_rejected(self):
        limits = self.load()
        entry = limits.kernel("Graph/A.cpp", "a", "random", 1024, 0, 256)
        bad = dict(entry, limit_ns=1000)
        with self.assertRaises(budgets.LimitError): limits.evaluate(bad, [1])
        for samples in ([], [True], [-1], [1.5]):
            with self.subTest(samples=samples), self.assertRaises(budgets.LimitError): limits.evaluate(entry, samples)
        self.path.write_text(self.path.read_text() + "\n")
        with self.assertRaisesRegex(budgets.LimitError, "変更"): limits.check_unchanged()


if __name__ == "__main__":
    unittest.main()
