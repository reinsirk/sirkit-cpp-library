# SPDX-License-Identifier: CC0-1.0
import itertools
import sys
from pathlib import Path
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import prepare_verify
from verify_graph_samples import check_maximum_independent_set


class MaximumIndependentSetCheckerTests(unittest.TestCase):
    def test_accepts_all_maximum_solutions_for_all_small_graphs(self):
        for n in range(1, 5):
            edges = list(itertools.combinations(range(n), 2))
            for mask in range(1 << len(edges)):
                graph = [edge for i, edge in enumerate(edges) if mask >> i & 1]
                valid = [subset for count in range(n + 1)
                         for subset in itertools.combinations(range(n), count)
                         if all(not (u in subset and v in subset) for u, v in graph)]
                optimum = max(map(len, valid))
                sample = {"input": f'{n} {len(graph)}\n' + ''.join(f'{u} {v}\n' for u, v in graph),
                          "output": str(optimum)}
                for subset in valid:
                    output = str(len(subset)) + '\n' + ' '.join(map(str, subset))
                    self.assertEqual(check_maximum_independent_set({}, sample, output), len(subset) == optimum)

    def test_rejects_bad_vertices_edges_counts_and_tokens(self):
        sample = {"input": '3 1\n0 1\n', "output": '2\n0 2\n'}
        for output in ['2\n0 1', '2\n0 0', '2\n0 3', '2\n-1 2', '2\n0',
                       '2\n0 2 1', '1\n2', '2\n0 nope', '2\n0 ２', '2\n0 0_2', '']:
            self.assertFalse(check_maximum_independent_set({}, sample, output), output)
        for malformed in ['3 -1', '3 2\n0 1\n', '3 1\n0 3', '3 1\n0 0']:
            self.assertFalse(check_maximum_independent_set({}, {**sample, "input": malformed}, '2\n0 2'))

    def test_registered_with_prepare_verify(self):
        sample = {"input": '3 1\n0 1\n', "output": '2\n0 2\n'}
        self.assertTrue(prepare_verify.matches({"sample_checker": "maximum_independent_set"}, sample, '2\n1 2'))


if __name__ == '__main__':
    unittest.main()
