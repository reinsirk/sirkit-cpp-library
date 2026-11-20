# SPDX-License-Identifier: CC0-1.0
"""非LC性能入力の縮小版だけを、ライブラリを使わず照合する。"""
from collections import Counter, deque
from contextlib import ExitStack
import copy
import hashlib
import heapq
import itertools
import math
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from Scripts import prepare_verify
from Verify.ci import performance_cases as cases


class PerformanceCaseTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entries = {e["id"]: e for e in prepare_verify.collect()}

    def test_all_twenty_uncovered_libraries_have_two_meaningful_profiles(self):
        all_libraries = {p for e in self.entries.values() for p in e["libraries"]}
        lc_libraries = {p for e in self.entries.values() if "judge.yosupo.jp/problem/" in (e.get("problem_url") or "") for p in e["libraries"]}
        self.assertEqual(set(cases.LIBRARY_VERIFY_IDS), all_libraries - lc_libraries)
        self.assertEqual(len(cases.LIBRARY_VERIFY_IDS), 20)
        full = [p for library in cases.LIBRARY_VERIFY_IDS for p in cases.plans_for_library(library)]
        self.assertEqual(len(full), 40)
        self.assertEqual(len({p["name"] for p in full}), 40)
        for library, identifier in cases.LIBRARY_VERIFY_IDS.items():
            self.assertEqual(self.entries[identifier]["libraries"], [library])
            for p in cases.plans_for_library(library):
                self.assertEqual(p["scale_claim"], "CI_workload_not_problem_max")
                self.assertTrue(p["oracle"] and p["dimensions"])
                self.assertEqual(cases.case_plans(identifier), cases.plans_for_library(library))

    def test_every_quick_fixture_matches_an_independent_model_and_checker(self):
        with tempfile.TemporaryDirectory() as temporary, patch.object(subprocess, "Popen", side_effect=AssertionError("no compiler or child program")):
            for library in cases.LIBRARY_VERIFY_IDS:
                for plan in cases.plans_for_library(library, True):
                    with self.subTest(name=plan["name"]):
                        result = cases.generate_case(plan, Path(temporary) / plan["name"])
                        inp = Path(result["input_path"]).read_text()
                        expected = Path(result["expected_path"]).read_text()
                        self.assertEqual(result["input_sha256"], hashlib.sha256(inp.encode()).hexdigest())
                        self.assertEqual(result["expected_sha256"], hashlib.sha256(expected.encode()).hexdigest())
                        self.assertTrue(inp and expected)
                        entry = dict(self.entries[plan["verify_id"]], sample_checker=plan["checker"])
                        self.assertTrue(prepare_verify.matches(entry, {"input": inp, "output": expected}, expected))
                        model = self.independent(plan, inp)
                        if model is not None:
                            self.assertEqual(expected.split(), model.split())

    def test_generation_is_reproducible_and_catalog_edits_are_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            plan = cases.plans_for_library("String/Trie.cpp", True)[1]
            a = cases.generate_case(plan, Path(temporary) / "a")
            b = cases.generate_case(plan, Path(temporary) / "b")
            self.assertEqual(a["input_sha256"], b["input_sha256"])
            self.assertEqual(a["expected_sha256"], b["expected_sha256"])
            modified = copy.deepcopy(plan); modified["dimensions"]["patterns"] += 1
            with self.assertRaises(ValueError):cases.generate_case(modified, Path(temporary) / "bad")
            with patch.object(cases, "MAX_BYTES", 1), self.assertRaises(ValueError):
                cases.generate_case(plan, Path(temporary) / "capped")
        with self.assertRaises(ValueError):cases.generate_case(plan, cases._ROOT / "unwanted-performance-input")

    def test_sieve_profile_builds_a_substantial_table_with_known_factors(self):
        a, b = cases.plans_for_library("Math/Zeta/Sieve.cpp")
        self.assertEqual(a["dimensions"], {"value": 10**12, "sieve_bound": 1000000})
        self.assertEqual(b["dimensions"], {"value": 999983**2, "sieve_bound": 999983})
        self.assertTrue(all(999983 % d for d in range(2, math.isqrt(999983) + 1)))

    def test_declared_judge_bounds_and_character_contracts(self):
        for library in cases.LIBRARY_VERIFY_IDS:
            for plan in cases.plans_for_library(library):
                key, d = cases._key(plan["verify_id"]), plan["dimensions"]
                if key == "all_pairs":self.assertLessEqual(d["n"], 100)
                if key == "preferred_trie_dictionary":
                    self.assertLessEqual(d["text_length"], 50000)
                    self.assertLessEqual(d["patterns"], 5000)
                    self.assertLessEqual(d["word_length"], 10)
                if key == "preferred_matrix_rotate_integer":
                    self.assertTrue(all(1 <= d[v] <= 10 for v in ("h", "w", "scale")))
        with tempfile.TemporaryDirectory() as temporary:
            for plan in cases.plans_for_library("String/Trie.cpp", True):
                result = cases.generate_case(plan, Path(temporary) / plan["name"])
                words = Path(result["input_path"]).read_text().split()
                text, count, *patterns = words
                self.assertEqual(len(patterns), int(count))
                self.assertEqual(len(set(patterns)), len(patterns))
                self.assertTrue(all(set(s) <= set("ABCDEFGHIJKLMNOPQRSTUVWXYZ") for s in [text, *patterns]))
                self.assertTrue(all(1 <= len(s) <= 10 for s in patterns))
            for plan in cases.plans_for_library("Algorithm/rotate.cpp", True):
                result = cases.generate_case(plan, Path(temporary) / plan["name"])
                rows = Path(result["input_path"]).read_text().splitlines()[1:]
                self.assertTrue(all(set(row) <= {".", "#"} for row in rows))

    @staticmethod
    def independent(plan, data):
        key = cases._key(plan["verify_id"])
        rows = data.splitlines()
        ints = lambda line: list(map(int, line.split()))
        emit = lambda xs: " ".join(map(str, xs))
        if key == "all_pairs":
            n, m = ints(rows[0]); adj = [[] for _ in range(n)]
            for line in rows[1:]:
                a, b, weight = ints(line); assert weight == 1; adj[a - 1].append(b - 1)
            answers = []
            for start in range(n):
                distance = {start: 0}; pending = deque([start])
                while pending:
                    v = pending.popleft()
                    for w in adj[v]:
                        if w not in distance:distance[w] = distance[v] + 1; pending.append(w)
                answers.append(sum(distance.values()))
            assert len(rows) == m + 1
            return emit(answers)
        if key in {"preferred_topological_yuki", "topological_lexicographical"}:
            n, m = ints(rows[0]); adj = [[] for _ in range(n)]; degree = [0] * n
            for line in rows[1:]:
                a, b = ints(line); adj[a - 1].append(b - 1); degree[b - 1] += 1
            pending = [v for v in range(n) if not degree[v]]; heapq.heapify(pending)
            order = []; length = [1] * n
            while pending:
                v = heapq.heappop(pending); order.append(v + 1)
                for w in adj[v]:
                    length[w] = max(length[w], length[v] + 1); degree[w] -= 1
                    if not degree[w]:heapq.heappush(pending, w)
            return emit(order) if key == "topological_lexicographical" else str(max(length))
        if key == "slope_trick":
            coordinates = []; constant = 0; answers = []
            for line in rows[1:]:
                values = ints(line)
                if values[0] == 1:coordinates.append(values[1]); constant += values[2]
                else:
                    sorted_values = sorted(coordinates); median = sorted_values[(len(sorted_values) - 1) // 2]
                    answers.extend((median, constant + sum(abs(x - median) for x in coordinates)))
            return emit(answers)
        if key == "preferred_trie_dictionary":
            text, count = rows[0].split(); words = rows[1:]
            assert len(words) == int(count)
            return str(sum(sum(text.startswith(word, i) for i in range(len(text))) for word in words))
        if key == "stabbing":
            segments = [ints(line) for line in rows[1:]]
            assert all(min(y1, y2) <= 0 <= max(y1, y2) for x1, y1, x2, y2 in segments)
            return rows[0]
        if key == "coverage_graph_node_cycles_yuki":
            n = int(rows[0]); nxt = [v - 1 for v in ints(rows[1])]; seen = set(); answer = 0
            for start in range(n):
                if start in seen:continue
                path = set(); v = start
                while v not in seen and v not in path:path.add(v); v = nxt[v]
                answer += v in path; seen.update(path)
            return str(answer)
        if key == "zeta_sieve_factorize_split":
            value = int(rows[0]); exponents = []; p = 2
            while p * p <= value:
                count = 0
                while value % p == 0:value //= p; count += 1
                if count:exponents.append(count)
                p += 1
            if value > 1:exponents.append(1)
            return str(sum((math.isqrt(8 * e + 1) - 1) // 2 for e in exponents))
        if key == "preferred_data_hash_set_and_yuki":
            values = set(); answers = []
            for line in rows[1:]:
                tokens = ints(line)
                if tokens[0] == 1:values.add(tokens[1])
                elif tokens[0] == 2:values.discard(tokens[1])
                else:
                    answer = -1
                    if values:
                        answer = (1 << 30) - 1
                        for v in values:answer &= v
                    answers.append(answer)
            return emit(answers)
        if key == "xor_basis":
            values = ints(rows[1]); total = 0
            for v in values:total ^= v
            span = {0}
            for v in values:span |= {x ^ (v & ~total) for x in tuple(span)}
            return str(total + 2 * max(span))
        if key == "functional_graph":
            n, k = ints(rows[0]); nxt = [x - 1 for x in ints(rows[1])]; values = ints(rows[2]); answers = []
            for start in range(n):
                path = []; seen = {}; v = start
                while v not in seen:seen[v] = len(path); path.append(v); v = nxt[v]
                offset = k if k < len(path) else seen[v] + (k - seen[v]) % (len(path) - seen[v])
                answers.append(values[path[offset]])
            return emit(answers)
        if key == "preferred_matrix_rotate_integer":
            rotation, scale, h, w = ints(rows[0]); image = rows[1:]
            transformed = list(zip(*image[::-1])) if rotation == 90 else list(zip(*image))[::-1]
            return "\n".join("".join(c * scale for c in row) for row in transformed for _ in range(scale))
        if key == "preferred_project_selection_yuki":
            h, w = ints(rows[0]); costs = [ints(line) for line in rows[1:1 + h]]
            row_rewards, col_rewards = ints(rows[1 + h]), ints(rows[2 + h]); best = 0
            for mask in range(1 << (h * w)):
                chosen = lambda i, j: bool(mask & (1 << (i * w + j)))
                value = -sum(costs[i][j] for i in range(h) for j in range(w) if chosen(i, j))
                value += sum(row_rewards[i] for i in range(h) if all(chosen(i, j) for j in range(w)))
                value += sum(col_rewards[j] for j in range(w) if all(chosen(i, j) for i in range(h)))
                best = max(best, value)
            return str(best)
        if key == "centroid_decomposition":
            n = int(rows[0]); adj = [[] for _ in range(n)]
            for line in rows[1:]:a, b = ints(line); adj[a - 1].append(b - 1); adj[b - 1].append(a - 1)
            parent = [-1] * n; pending = [(set(range(n)), -1)]
            def components(vertices, removed):
                remaining = vertices - {removed}; output = []
                while remaining:
                    v = next(iter(remaining)); component = {v}; todo = [v]; remaining.remove(v)
                    while todo:
                        for w in adj[todo.pop()]:
                            if w in remaining:remaining.remove(w); component.add(w); todo.append(w)
                    output.append(component)
                return output
            while pending:
                vertices, up = pending.pop()
                scores = {v:max((len(c) for c in components(vertices, v)), default=0) for v in vertices}
                best = min(scores.values()); centers = [v for v in vertices if scores[v] == best]; assert len(centers) == 1
                center = centers[0]; parent[center] = -1 if up == -1 else up + 1
                pending.extend((component, center) for component in components(vertices, center))
            return emit(parent)
        if key == "coverage_geometry_coin_overlap_split":
            kept = []
            for line in rows[1:]:
                x, y = ints(line)
                if all((x - a)**2 + (y - b)**2 >= 400 for a, b in kept):kept.append((x, y))
            return str(len(kept))
        if key == "min_cost_flow_grid":
            n, k = ints(rows[0]); values = [ints(line) for line in rows[1:]]
            score = sum(map(sum, values)) if k == n else max(sum(values[i][p[i]] for i in range(n)) for p in itertools.permutations(range(n)))
            # Full grid feasibility is checked above by the independent registered checker.
            assert score == (n * n if plan["scenario"] == "all_cells" else 2 * n)
            return None
        if key == "pruefer_code":return None
        raise AssertionError(key)


if __name__ == "__main__":
    unittest.main()
