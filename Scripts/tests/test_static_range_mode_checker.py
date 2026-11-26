# SPDX-License-Identifier: CC0-1.0
import itertools
import sys
from pathlib import Path
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import prepare_verify
from verify_graph_samples import check_static_range_mode


def sample_for(values, ranges):
    lines = [f"{len(values)} {len(ranges)}", " ".join(map(str, values))]
    lines += [f"{left} {right}" for left, right in ranges]
    answers = []
    for left, right in ranges:
        part = values[left:right]
        best = max(part.count(value) for value in part)
        answers.append(f"{min(value for value in part if part.count(value) == best)} {best}")
    return {"input": "\n".join(lines) + "\n", "output": "\n".join(answers) + ("\n" if answers else "")}


class StaticRangeModeCheckerTests(unittest.TestCase):
    def test_accepts_every_mode_and_rejects_other_values(self):
        for n in range(1, 5):
            for values in itertools.product(range(3), repeat=n):
                values = list(values)
                for left in range(n):
                    for right in range(left + 1, n + 1):
                        sample = sample_for(values, [(left, right)])
                        part = values[left:right]
                        best = max(part.count(value) for value in part)
                        for value in range(4):
                            for times in range(best + 2):
                                valid = times == best and part.count(value) == best
                                self.assertEqual(check_static_range_mode({}, sample, f"{value} {times}"), valid)

    def test_checks_every_query_and_counts(self):
        sample = sample_for([3, 7, 1, 2, 1], [(1, 5), (3, 4), (0, 4)])
        self.assertTrue(check_static_range_mode({}, sample, "1 2\n2 1\n7 1\n"))
        for output in ["1 2\n2 1\n", "1 2\n2 1\n7 1\n0 0", "1 2\n2 1\n7 2", "1 2\n1 1\n7 1", "1 2\n2 1\nx 1", ""]:
            self.assertFalse(check_static_range_mode({}, sample, output), output)
        empty = sample_for([5, 5, 5], [])
        self.assertTrue(check_static_range_mode({}, empty, ""))

    def test_rejects_inputs_outside_constraints(self):
        for text in ["0 0\n\n", "2 1\n1 2\n1 1\n", "2 1\n1 2\n0 3\n", "2 1\n1 -1\n0 1\n", "2 1\n1 2\n0\n"]:
            self.assertFalse(check_static_range_mode({}, {"input": text, "output": "1 1\n"}, "1 1"), text)

    def test_registered_with_prepare_verify(self):
        sample = sample_for([3, 7, 1, 2, 1], [(0, 4)])
        self.assertTrue(prepare_verify.matches({"sample_checker": "static_range_mode"}, sample, "2 1"))
        self.assertFalse(prepare_verify.matches({"sample_checker": "static_range_mode"}, sample, "4 1"))


if __name__ == '__main__':
    unittest.main()
