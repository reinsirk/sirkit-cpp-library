#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""非一意な出力の別解を受理し、壊した解を拒否する。"""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import verify_graph_samples as checks


class GraphSamplesTest(unittest.TestCase):
    def test_modular_square_root(self):
        sample = {"input": "5\n0 5\n1 5\n2 5\n3 5\n4 5\n"}
        for answer in ("0 1 -1 -1 2", "0 4 -1 -1 3"):
            self.assertTrue(checks.check_modular_square_root({}, sample, answer))
        for answer in ("0 1 -1 -1 1", "-1 1 -1 -1 2", "0 -1 -1 -1 2",
                       "0 1 0 -1 2", "0 1 -1 -1 7", "0 1 -1 -1 2 9", ""):
            self.assertFalse(checks.check_modular_square_root({}, sample, answer))
        binary = {"input": "2\n0 2\n1 2\n"}
        self.assertTrue(checks.check_modular_square_root({}, binary, "0 1"))
        self.assertFalse(checks.check_modular_square_root({}, binary, "0 -1"))

    def test_general_matching(self):
        cases = [{'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '1\n0 1\n', 'expected': True},
         {'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '1\n2 0\n', 'expected': True},
         {'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '0\n', 'expected': False},
         {'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '2\n0 1\n1 2\n', 'expected': False},
         {'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '1\n1 1\n', 'expected': False},
         {'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '1\n0 3\n', 'expected': False},
         {'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '-1\n', 'expected': False},
         {'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '1\n0 1\nextra', 'expected': False},
         {'input': '3 3\n0 1\n1 2\n2 0\n', 'output': '', 'expected': False},
         {'input': '4 3\n0 1\n1 2\n2 3\n', 'output': '2\n1 0\n3 2\n', 'expected': True},
         {'input': '4 3\n0 1\n1 2\n2 3\n', 'output': '1\n1 2\n', 'expected': False},
         {'input': '4 3\n0 1\n1 2\n2 3\n', 'output': '2\n0 2\n1 3\n', 'expected': False},
         {'input': '4 3\n0 1\n1 2\n2 3\n', 'output': '2\n0 1\n1 2\n', 'expected': False},
         {'input': '0 0\n', 'output': '0\n', 'expected': True},
         {'input': '2 3\n0 0\n0 1\n1 0\n', 'output': '1\n1 0\n', 'expected': True},
         {'input': '2 1\n0 2\n', 'output': '0\n', 'expected': False}]
        for case in cases:
            with self.subTest(case=case):
                self.assertEqual(checks.check_general_matching({}, case, case["output"]), case["expected"])

    def test_binary_linear_system(self):
        cases = [{'input': '2 3\n101\n011\n10\n', 'output': '1\n100\n111\n', 'expected': True},
         {'input': '2 3\n101\n011\n10\n', 'output': '1\n011\n111\n', 'expected': True},
         {'input': '1 3\n100\n1\n', 'output': '2\n100\n011\n001\n', 'expected': True},
         {'input': '1 3\n100\n1\n', 'output': '2\n100\n010\n010\n', 'expected': False},
         {'input': '1 3\n100\n1\n', 'output': '2\n100\n100\n001\n', 'expected': False},
         {'input': '2 3\n101\n011\n10\n', 'output': '-1\n', 'expected': False},
         {'input': '2 2\n10\n10\n01\n', 'output': '-1\n', 'expected': True},
         {'input': '2 2\n10\n10\n01\n', 'output': '0\n10\n', 'expected': False},
         {'input': '1 1\n1\n1\n', 'output': '0\n1\n', 'expected': True},
         {'input': '1 1\n1\n1\n', 'output': '0\n0\n', 'expected': False},
         {'input': '1 1\n1\n1\n', 'output': '0\n1\n0\n', 'expected': False}]
        for case in cases:
            with self.subTest(case=case):
                self.assertEqual(checks.check_binary_linear_system({}, case, case["output"]), case["expected"])

    def test_longest_common_substring(self):
        sample = {"input": "abcdef\nabcxdef\n"}
        for answer in ("0 3 0 3", "3 6 4 7"):
            self.assertTrue(checks.check_longest_common_substring({}, sample, answer))
        for answer in ("0 2 0 2", "0 3 4 7", "-1 2 0 3", "0 3 0 2", "0 3 0 3 99", ""):
            self.assertFalse(checks.check_longest_common_substring({}, sample, answer))
        self.assertTrue(checks.check_longest_common_substring({}, {"input": "aa\nbbb\n"}, "1 1 2 2"))

    def test_matrix_linear_system(self):
        sample = {"input": "1 2\n1 1\n3\n"}
        for answer in ("1\n3 0\n998244352 1\n", "1\n0 3\n998244351 2\n"):
            self.assertTrue(checks.check_matrix_linear_system({}, sample, answer))
        for answer in ("-1", "0\n3 0", "1\n4 0\n998244352 1", "1\n3 0\n0 0",
                       "1\n3 0\n1 1", "2\n3 0\n998244352 1\n998244351 2", "1\n3 0\n-1 1", ""):
            self.assertFalse(checks.check_matrix_linear_system({}, sample, answer))
        for inp, answer in (("1 0\n1\n", "-1"), ("0 0\n", "0\n"),
                            ("0 2\n", "2\n0 0\n1 0\n0 1\n"),
                            ("2 1\n1\n1\n1 2\n", "-1")):
            self.assertTrue(checks.check_matrix_linear_system({}, {"input": inp}, answer))

    def test_linear_recurrence(self):
        sample = {"input": "3\n1 2 5\n"}
        for answer in ("2\n0 5\n", "2\n1 3\n", "2\n2 1\n"):
            self.assertTrue(checks.check_linear_recurrence({}, sample, answer))
        for answer in ("3\n0 0 0\n", "1\n2\n", "2\n1 2\n", "2\n1 -1\n",
                       "2\n1 998244353\n", "2\n1 3\n99", ""):
            self.assertFalse(checks.check_linear_recurrence({}, sample, answer))
        self.assertTrue(checks.check_linear_recurrence({}, {"input": "0\n"}, "0\n"))
        self.assertTrue(checks.check_linear_recurrence({}, {"input": "3\n0 0 0\n"}, "0\n"))
        self.assertFalse(checks.check_linear_recurrence({}, {"input": "3\n0 0 0\n"}, "1 0\n"))
        self.assertTrue(checks.check_linear_recurrence({}, {"input": "5\n0 0 0 0 1\n"}, "5 0 0 0 0 0\n"))

    def test_bipartite_matching(self):
        sample = {"input": "3 3 6\n0 0\n0 1\n1 0\n1 1\n2 1\n2 2\n"}
        for answer in ("3\n0 0\n1 1\n2 2\n", "3\n2 2\n0 1\n1 0\n"):
            self.assertTrue(checks.check_bipartite_matching({}, sample, answer))
        for answer in ("2\n0 0\n1 1\n", "3\n0 0\n1 0\n2 2\n",
                       "3\n0 0\n0 1\n2 2\n", "3\n0 0\n1 1\n2 0\n",
                       "3\n0 0\n1 1\n3 2\n", "3\n0 0\n1 1\n2 2\n99", ""):
            self.assertFalse(checks.check_bipartite_matching({}, sample, answer))
        for inp in ("0 0 0\n", "3 2 0\n"):
            self.assertTrue(checks.check_bipartite_matching({}, {"input": inp}, "0\n"))

    def test_scc(self):
        sample = {"input": "3 3\n0 1\n1 0\n1 2\n"}
        for answer in ("2\n2 0 1\n1 2\n", "2\n2 1 0\n1 2\n"):
            self.assertTrue(checks.check_scc({}, sample, answer))
        for answer in ("2\n1 2\n2 0 1\n", "1\n3 0 1 2\n",
                       "3\n1 0\n1 1\n1 2\n", "2\n2 0 0\n1 2\n", ""):
            self.assertFalse(checks.check_scc({}, sample, answer))
        self.assertTrue(checks.check_scc({}, {"input": "0 0\n"}, "0\n"))

    def test_two_sat(self):
        sample = {"input": "p cnf 2 2\n1 2 0\n-1 2 0\n",
                  "output": "s SATISFIABLE\nv 1 2 0\n"}
        for answer in ("s SATISFIABLE\nv 1 2 0\n", "s SATISFIABLE\nv -1 2 0\n"):
            self.assertTrue(checks.check_two_sat({}, sample, answer))
        for answer in ("s UNSATISFIABLE", "s SATISFIABLE v 1 -2 0",
                       "s SATISFIABLE v 1 1 0", "s SATISFIABLE v 1 2"):
            self.assertFalse(checks.check_two_sat({}, sample, answer))
        impossible = {"input": "p cnf 1 2\n1 1 0\n-1 -1 0\n",
                      "output": "s UNSATISFIABLE\n"}
        self.assertTrue(checks.check_two_sat({}, impossible, "s UNSATISFIABLE"))
        self.assertFalse(checks.check_two_sat({}, impossible, "s SATISFIABLE v 1 0"))

    def test_grid_matching(self):
        sample = {"input": "2 2\n..\n..\n", "output": "2\n><\n><\n"}
        self.assertTrue(checks.check_max_flow_grid({}, sample, "2\nvv\n^^\n"))
        for answer in ("2\n><\n..\n", "2\n>>\n<<\n", "1\n><\n><\n"):
            self.assertFalse(checks.check_max_flow_grid({}, sample, answer))
        blocked = {"input": "1 2\n.#\n", "output": "0\n.#\n"}
        self.assertFalse(checks.check_max_flow_grid({}, blocked, "1\n><\n"))

    def test_pruefer_completion(self):
        sample = {"input": "3 0\n2 1 1\n"}
        for answer in ("1 2\n1 3\n", "3 1\n2 1\n"):
            self.assertTrue(checks.check_pruefer_completion({}, sample, answer))
        for answer in ("-1", "1 2\n1 2\n", "1 2\n2 3\n", "1 2", "1 4\n1 2"):
            self.assertFalse(checks.check_pruefer_completion({}, sample, answer))
        impossible = {"input": "4 1\n1 1 1 1\n1 2\n"}
        self.assertTrue(checks.check_pruefer_completion({}, impossible, "-1"))
        self.assertFalse(checks.check_pruefer_completion({}, impossible, "1 3\n2 4\n"))
        self.assertTrue(checks.check_pruefer_completion({}, {"input": "1 0\n0\n"}, ""))

    def test_component_sets(self):
        sample = {"input": "3 0\n", "output": "2\n2 0 1\n1 2\n"}
        self.assertTrue(checks.check_component_sets({}, sample, "2\n1 2\n2 1 0\n"))
        for answer in ("1\n3 0 1 2\n", "2\n2 0 0\n1 2\n", "2\n2 0 1\n1 2\n99"):
            self.assertFalse(checks.check_component_sets({}, sample, answer))

    def test_eulerian(self):
        sample = {"input": "1\n2 2\n0 1\n1 0\n", "output": "Yes\n0 1 0\n0 1\n"}
        for checker in (checks.check_eulerian_trail_directed, checks.check_eulerian_trail_undirected):
            self.assertTrue(checker({}, sample, "Yes\n1 0 1\n1 0\n"))
            for answer in ("No", "Yes 0 1 0 0 0", "Yes 0 0 0 0 1", "Yes 0 1 0 1 0 99"):
                self.assertFalse(checker({}, sample, answer))
        sample = {"input": "1\n2 1\n0 1\n", "output": "Yes 0 1 0"}
        self.assertTrue(checks.check_eulerian_trail_undirected({}, sample, "Yes 1 0 0"))
        self.assertFalse(checks.check_eulerian_trail_directed({}, sample, "Yes 1 0 0"))

    def test_grid_min_cost(self):
        sample = {"input": "2 1\n3 1\n1 3\n", "output": "6\nX.\n.X\n"}
        self.assertTrue(checks.check_min_cost_flow_grid({}, sample, "6 X. .X"))
        for answer in ("6 .X X.", "8 XX XX", "6 XX ..", "6 X. ?X"):
            self.assertFalse(checks.check_min_cost_flow_grid({}, sample, answer))
        zero = {"input": "2 1\n0 0\n0 0\n", "output": "0\n..\n..\n"}
        self.assertTrue(checks.check_min_cost_flow_grid({}, zero, "0 .X X."))


if __name__ == "__main__":
    unittest.main()
