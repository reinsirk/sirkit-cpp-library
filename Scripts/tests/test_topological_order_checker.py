# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import itertools
import random
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import prepare_verify as prepare
import verify_graph_samples as checks


def input_for(n, edges):
    return f"{n} {len(edges)}\n" + "".join(f"{u} {v}\n" for u, v in edges)


def kahn_orders(n, edges):
    # 各段階で入次数0の未使用頂点を選ぶ独立列挙。
    incoming = [set() for _ in range(n)]
    for u, v in edges:
        incoming[v].add(u)
    found = set()

    def visit(order, used):
        if len(order) == n:
            found.add(tuple(order))
            return
        for vertex in range(n):
            if vertex not in used and incoming[vertex] <= used:
                visit(order + [vertex], used | {vertex})

    visit([], set())
    return found


class TopologicalOrderCheckerTests(unittest.TestCase):
    def accepts(self, n, edges, output):
        sample = {"input": input_for(n, edges), "output": "intentionally unused official order"}
        expected = checks.check_topological_order({}, sample, output)
        for mode in ["topological_order", "topological"]:
            self.assertEqual(prepare.matches({"sample_checker": mode}, sample, output), expected)
        return expected

    def test_multiple_correct_orders_and_whitespace(self):
        edges = [(0, 2), (1, 2), (2, 3)]
        self.assertTrue(self.accepts(4, edges, "0\n1\n2\n3\n"))
        self.assertTrue(self.accepts(4, edges, "1 0 2 3"))
        self.assertTrue(self.accepts(4, edges, " +1  -0  02 3 \n"))
        self.assertTrue(self.accepts(1, [], "0"))
        self.assertTrue(self.accepts(0, [], " \n"))

    def test_missing_duplicate_out_of_range_and_extra_output(self):
        edges = [(0, 2), (1, 2), (2, 3)]
        for output in ["", "0 1 2", "0 1 2 2", "0 1 2 4", "-1 0 2 3", "0 1 2 3 4",
                       "2 0 1 3", "0 1 3 2", "0 1 2 3 DONE", "4 0 1 2 3"]:
            with self.subTest(output=output):
                self.assertFalse(self.accepts(4, edges, output))

    def test_invalid_tokens_are_false_not_exceptions(self):
        for output in ["0 1.0", "0 x", "0 1_0", "0 ١", "0 1e0", None, 1, "0 NaN"]:
            with self.subTest(output=output):
                self.assertFalse(self.accepts(2, [], output))

    def test_malformed_graph_inputs_and_cycles(self):
        for text in ["", "1", "-1 0", "2 -1", "2 2 0 1", "2 0 0 1",
                     "2 1 0 2", "2 1 -1 0", "2 1 0 x", "2 1_0"]:
            self.assertFalse(checks.check_topological_order({}, {"input": text}, "0 1"))
        self.assertFalse(self.accepts(2, [(0, 1), (1, 0)], "0 1"))
        self.assertFalse(self.accepts(1, [(0, 0)], "0"))
        self.assertTrue(self.accepts(2, [(0, 1), (0, 1)], "0 1"))

    def test_all_small_dags_against_independent_kahn_enumeration(self):
        for n in range(5):
            candidates = [(u, v) for u in range(n) for v in range(u + 1, n)]
            for mask in range(1 << len(candidates)):
                edges = [e for i, e in enumerate(candidates) if (mask >> i) & 1]
                orders = kahn_orders(n, edges)
                sample = {"input": input_for(n, edges)}
                for order in itertools.permutations(range(n)):
                    output = " ".join(map(str, order))
                    self.assertEqual(checks.check_topological_order({}, sample, output), order in orders)
        rng = random.Random(20261115)
        for _ in range(20):
            n = 5
            label = list(range(n))
            rng.shuffle(label)
            edges = [(label[u], label[v]) for u in range(n) for v in range(u + 1, n) if rng.randrange(2)]
            orders = kahn_orders(n, edges)
            for order in itertools.permutations(range(n)):
                self.assertEqual(checks.check_topological_order({}, {"input": input_for(n, edges)},
                                                          " ".join(map(str, order))), order in orders)


if __name__ == "__main__":
    unittest.main()
