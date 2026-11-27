# SPDX-License-Identifier: CC0-1.0
"""生成器・独立参照・欠落検出の軽い検査。コンパイラやMax入力は起動しない。"""
from contextlib import redirect_stdout
from collections import deque
import copy
import hashlib
import io
import json
from pathlib import Path
import shutil
import signal
import subprocess
import tempfile
import unittest
from unittest import mock

from Scripts import ci_stress
from Verify.ci import stress_cases


def cuboid_oracle(data):
    rows = [list(map(int, row.split())) for row in data.splitlines()]
    n = rows[0][0]
    grid = rows[1:n * n + 1]
    answers = []
    for lx, rx, ly, ry, lz, rz in rows[n * n + 2:]:
        answers.append(sum(grid[x * n + y][z] for x in range(lx - 1, rx)
                           for y in range(ly - 1, ry) for z in range(lz - 1, rz)))
    return answers


class StressCatalogTests(unittest.TestCase):
    def test_maxima_are_pinned_actual_problem_constraints(self):
        plans = stress_cases.case_plans()
        self.assertEqual(len({p["name"] for p in plans}), len(plans))
        self.assertEqual({p["verify_id"] for p in plans},
                         {"union_find", "persistent_union_find_union_path_query", "prefix_sum_2d", "prefix_sum_3d"})
        union = next(p for p in plans if p["name"] == "union_find_max")
        self.assertEqual(union["dimensions"], {"n": 200000, "q": 200000})
        self.assertEqual(union["maximum_dimensions"], ["n", "q"])
        self.assertIn(stress_cases.LIBRARY_CHECKER_REVISION, union["constraint_source"])
        balanced = next(p for p in plans if p["name"] == "union_find_balanced")
        self.assertEqual(balanced["dimensions"], {"n": 131072, "q": 200000})
        self.assertEqual(balanced["maximum_dimensions"], ["q"])
        for p in plans:
            self.assertLess(p["planned_build_operations"], p["dimensions"]["q"])
            self.assertLessEqual(p["dimensions"]["n"], p["problem_limits"]["n"])
            self.assertLessEqual(p["dimensions"]["q"], p["problem_limits"]["q"])
        two_stars = next(p for p in plans if p["name"].endswith("two_stars"))
        self.assertEqual(two_stars["dimensions"], {"n": 200000, "q": 300000})
        self.assertGreater(10**9 * (two_stars["dimensions"]["n"] // 2)**2, (1 << 63) - 1)
        self.assertEqual(two_stars["planned_build_operations"], 200001)
        query = next(p for p in plans if p["name"].endswith("query_heavy"))
        self.assertEqual(query["planned_build_operations"], 131069)
        self.assertEqual(query["dimensions"]["q"] - query["planned_build_operations"], 168931)
        trains = next(p for p in plans if p["verify_id"] == "prefix_sum_2d")
        self.assertEqual(trains["dimensions"], {"n": 500, "m": 200000, "q": 100000})
        self.assertEqual(set(trains["maximum_dimensions"]), {"n", "m", "q"})
        cube = next(p for p in plans if p["verify_id"] == "prefix_sum_3d")
        self.assertEqual(cube["dimensions"], {"n": 100, "q": 200000})
        self.assertEqual(cube["maximum_dimensions"], ["n", "q"])

    def test_quick_is_never_reported_as_max(self):
        for plan in stress_cases.case_plans(True):
            self.assertEqual(plan["maximum_dimensions"], [])
            self.assertEqual(plan["size_profile"], "quick_smoke")
            self.assertLessEqual(plan["dimensions"]["n"], 64)

    def test_plan_has_no_compilation_execution_or_generation(self):
        with mock.patch.object(subprocess, "Popen", side_effect=AssertionError("no child")), \
                mock.patch.object(stress_cases, "generate_case", side_effect=AssertionError("no input")), \
                redirect_stdout(io.StringIO()) as output:
            self.assertEqual(ci_stress.main(["--plan", "--quick"]), 0)
        report = json.loads(output.getvalue())
        self.assertFalse(report["executed"])
        self.assertEqual(report["cases"], stress_cases.case_plans(True))

    def test_every_quick_generator_has_exact_q_and_nonempty_oracle(self):
        with tempfile.TemporaryDirectory() as temporary:
            for plan in stress_cases.case_plans(True):
                folder = Path(temporary) / plan["name"]
                result = stress_cases.generate_case(plan, folder)
                actual = (folder / "input.txt").read_bytes()
                expected = (folder / "expected.txt").read_bytes()
                if plan["verify_id"] == "prefix_sum_2d":
                    n, m, q = map(int, actual.splitlines()[0].split())
                    self.assertEqual((n, m, q), tuple(plan["dimensions"][key] for key in ("n", "m", "q")))
                    self.assertEqual(len(actual.splitlines()), m + q + 1)
                elif plan["verify_id"] == "prefix_sum_3d":
                    n = plan["dimensions"]["n"]
                    self.assertEqual(len(actual.splitlines()), n * n + plan["dimensions"]["q"] + 2)
                else:
                    self.assertEqual(len(actual.splitlines()) - 1, plan["dimensions"]["q"])
                self.assertEqual(result["generated_operations"], plan["dimensions"]["q"])
                self.assertGreater(result["expected_answer_count"], 0)
                self.assertEqual(result["expected_answer_count"], len(expected.splitlines()))
                self.assertEqual(result["input_sha256"], hashlib.sha256(actual).hexdigest())
                self.assertEqual(result["expected_sha256"], hashlib.sha256(expected).hexdigest())
                if plan["verify_id"] == "union_find":
                    self.assertEqual(expected.split(), self.union_oracle(actual))
                elif plan["verify_id"] == "prefix_sum_2d":
                    rows = [tuple(map(int, row.split())) for row in actual.splitlines()]
                    _, m, _ = rows[0]
                    trains = rows[1:m + 1]
                    answers = [sum(left <= lo <= hi <= right for lo, hi in trains)
                               for left, right in rows[m + 1:]]
                    self.assertEqual(list(map(int, expected.split())), answers)
                elif plan["verify_id"] == "prefix_sum_3d":
                    self.assertEqual(list(map(int, expected.split())), cuboid_oracle(actual))
                else:
                    self.assertEqual(expected.split(), self.yuki_oracle(actual))

    @staticmethod
    def union_oracle(data):
        rows = [list(map(int, line.split())) for line in data.splitlines()]
        n, q = rows[0]
        assert len(rows) == q + 1
        labels, output = list(range(n)), []
        for kind, u, v in rows[1:]:
            if kind == 0:
                old, new = labels[v], labels[u]
                labels = [new if label == old else label for label in labels]
            else:
                output.append(str(int(labels[u] == labels[v])).encode())
        return output

    @staticmethod
    def yuki_oracle(data):
        # 永続UFや生成器のbit式を使わず、既知の森をBFSしてパス最大値を求める。
        rows = [list(map(int, line.split())) for line in data.splitlines()]
        n, x, q = rows[0]
        assert len(rows) == q + 1
        adjacency, output = [[] for _ in range(n)], []
        last_weight = 0
        def distances(start):
            distance, pending = {start: 0}, deque([start])
            while pending:
                vertex = pending.popleft()
                for other, weight in adjacency[vertex]:
                    if other not in distance:
                        distance[other] = max(distance[vertex], weight)
                        pending.append(other)
            return distance
        for operation in rows[1:]:
            kind = operation[0]
            if kind == 1:
                _, v, weight = operation
                assert 0 <= x < n and 0 <= v < n and last_weight <= weight <= 10**9
                assert v not in distances(x)
                adjacency[x].append((v, weight))
                adjacency[v].append((x, weight))
                last_weight = weight
            elif kind == 2:
                _, u, v = operation
                answer = distances(u).get(v, -1)
                output.append(str(answer).encode())
                if answer >= 0:
                    x = (x + answer) % n
            elif kind == 3:
                _, v = operation
                component = distances(v)
                total = sum(sum(distances(vertex).values()) for vertex in component) // 2
                output.append(str(total % 998244353).encode())
            else:
                _, value = operation
                assert 0 <= value < n
                x = (x + value) % n
        return output

    def test_generation_is_deterministic(self):
        with tempfile.TemporaryDirectory() as temporary:
            for plan in [stress_cases.case_plans(True)[0], stress_cases.case_plans(True)[4]]:
                a, b = Path(temporary) / "a", Path(temporary) / "b"
                first, second = stress_cases.generate_case(plan, a), stress_cases.generate_case(plan, b)
                self.assertEqual(first, second)
                self.assertEqual((a / "input.txt").read_bytes(), (b / "input.txt").read_bytes())

    def test_modified_or_out_of_constraint_plan_fails(self):
        with tempfile.TemporaryDirectory() as temporary:
            plan = stress_cases.case_plans(True)[0]
            plan["dimensions"]["q"] = 500001
            with self.assertRaises(stress_cases.GenerationError):
                stress_cases.generate_case(plan, Path(temporary))
            plan = stress_cases.case_plans(True)[0]
            plan["seed"] += 1
            with self.assertRaises(stress_cases.GenerationError):
                stress_cases.generate_case(plan, Path(temporary))

    def test_input_cap_and_missing_operations_fail(self):
        with tempfile.TemporaryDirectory() as temporary:
            writer = stress_cases.CaseWriter(Path(temporary), 2, 60)
            try:
                writer.line(1, 0, 0, operation=True)
                writer.answer(0)
                with self.assertRaises(stress_cases.GenerationError):
                    writer.finish()
                with mock.patch.object(stress_cases, "MAX_INPUT_BYTES", 1):
                    with self.assertRaises(stress_cases.GenerationError):
                        writer.line(1)
            finally:
                writer.close()


class StressResultTests(unittest.TestCase):
    def compare(self, actual, expected=b"1\n0\n", count=2):
        with tempfile.TemporaryDirectory() as temporary:
            a, e = Path(temporary) / "actual", Path(temporary) / "expected"
            if actual is not None:
                a.write_bytes(actual)
            e.write_bytes(expected)
            return ci_stress.compare_output(a, e, count)

    def test_full_integer_output_match(self):
        self.assertTrue(self.compare(b"  +01\n0 \n")["passed"])

    def test_missing_extra_wrong_noninteger_and_empty_outputs_fail(self):
        for actual in [None, b"", b"1\n", b"1\n0\n0\n", b"0\n0\n", b"1\nx\n", b"9" * 33]:
            with self.subTest(actual=actual):
                self.assertFalse(self.compare(actual)["passed"])
        self.assertFalse(self.compare(b"", b"", 0)["passed"])
        self.assertFalse(self.compare(b"1\n0\n", count=3)["passed"])

    def test_output_size_cap(self):
        with mock.patch.object(stress_cases, "MAX_OUTPUT_BYTES", 2):
            self.assertFalse(self.compare(b"1\n0\n")["passed"])

    def test_all_cases_builds_and_source_freeze_are_required(self):
        case = {"name": "one", "verify_id": "id", "status": "PASS", "expected_answer_count": 1,
                "answer_count": 1, "dimensions": {"q": 1}, "generated_operations": 1}
        good = {"selected_case_ids": ["one"], "results": [case],
                "builds": [{"verify_id": "id", "status": "PASS"}], "source_freeze_passed": True}
        self.assertTrue(ci_stress.complete(good))
        changes = [lambda r: r.update(results=[]), lambda r: r.update(builds=[]),
                   lambda r: r.update(selected_case_ids=[]), lambda r: r.update(source_freeze_passed=False),
                   lambda r: r["results"][0].update(status="NOT_RUN"),
                   lambda r: r["results"][0].update(answer_count=0),
                   lambda r: r["results"][0].update(expected_answer_count=0),
                   lambda r: r["results"][0].update(generated_operations=0),
                   lambda r: r["results"][0].pop("dimensions"),
                   lambda r: r["results"][0].update(answer_count=True),
                   lambda r: r.update(results=None), lambda r: r.update(builds=None)]
        for change in changes:
            altered = copy.deepcopy(good)
            change(altered)
            self.assertFalse(ci_stress.complete(altered))

    def test_source_mutation_is_detected(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for name in ["Scripts/ci_stress.py", "Scripts/prepare_verify.py", "Verify/ci/stress_cases.py", "Verify/ci/library_checker.json",
                         "DataStructure/Test.cpp", "Verify/current/drivers/test.cpp"]:
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("// fixture\n")
            entry = {"libraries": ["DataStructure/Test.cpp"], "driver": "Verify/current/drivers/test.cpp"}
            frozen = ci_stress.frozen_sources([entry], root)
            ci_stress.assert_sources_unchanged(frozen, root)
            (root / entry["driver"]).write_text("// changed\n")
            with self.assertRaises(ci_stress.StressError):
                ci_stress.assert_sources_unchanged(frozen, root)

    def test_timeout_kills_only_dedicated_child_process_group(self):
        with tempfile.TemporaryDirectory() as temporary:
            process = mock.Mock(pid=12345)
            process.wait.side_effect = [subprocess.TimeoutExpired(["fake"], 1), -9]
            with mock.patch.object(subprocess, "Popen", return_value=process) as popen, \
                    mock.patch.object(os := ci_stress.os, "killpg") as kill:
                row = ci_stress.execute(["fake"], stdout=Path(temporary) / "out",
                                        stderr=Path(temporary) / "err", timeout=1, memory_mib=512)
            self.assertTrue(row["timed_out"])
            self.assertEqual(row["returncode"], -9)
            kill.assert_called_once_with(12345, signal.SIGKILL)
            self.assertTrue(popen.call_args.kwargs["start_new_session"])

    def test_run_final_source_change_cannot_return_success(self):
        # 実C++コンパイルや実プログラム起動をせず、runnerの結果伝播だけ確認。
        def fake_execute(command, *, stdout, stderr, timeout, memory_mib, stdin=None):
            stderr.write_bytes(b"")
            if "--version" in command:
                stdout.write_text("mock-gcc 1\n")
            elif "-o" in command:
                Path(command[command.index("-o") + 1]).write_bytes(b"fake executable")
                stdout.write_bytes(b"")
            else:
                shutil.copyfile(stdin.parent / "expected.txt", stdout)
            return {"returncode": 0, "timed_out": False, "elapsed_ns": 1}
        with tempfile.TemporaryDirectory() as temporary:
            with mock.patch.object(ci_stress, "execute", side_effect=fake_execute), \
                    mock.patch.object(ci_stress.shutil, "which", return_value="/bin/true"), \
                    mock.patch.object(ci_stress, "assert_sources_unchanged",
                                      side_effect=[None, ci_stress.StressError("changed after run")]):
                code = ci_stress.main(["--quick", "--case", "union_find_max", "--output", temporary])
            self.assertEqual(code, 1)
            report = json.loads((Path(temporary) / "result.json").read_text())
            self.assertEqual(report["status"], "FAIL")
            self.assertFalse(report["source_freeze_passed"])


if __name__ == "__main__":
    unittest.main()
