# SPDX-License-Identifier: CC0-1.0
"""非LCライブラリの全体時間用入力。規模はCI workloadで、問題Maxとは呼ばない。"""
from __future__ import annotations

import hashlib
from pathlib import Path

from . import stress_cases

LIBRARY_VERIFY_IDS = {
    "Algorithm/Prefix_Sum_ND.cpp": "prefix_sum_3d",
    "Graph/Topological.cpp": "preferred_topological_yuki",
    "Graph/Bellman-Ford_SFPA.cpp": "coverage_graph_spfa_all_pairs_yuki",
    "DataStructure/Slope_Trick.cpp": "slope_trick",
    "String/Trie.cpp": "preferred_trie_dictionary",
    "Graph/Topological_sort.cpp": "topological_lexicographical",
    "Geometry/Point.cpp": "coverage_geometry_orientation_stabbing_split",
    "DataStructure/Node.cpp": "coverage_graph_node_cycles_yuki",
    "Graph/Bellman-Ford.cpp": "coverage_graph_bellman_ford_all_pairs_yuki",
    "Math/Zeta/Sieve.cpp": "zeta_sieve_factorize_split",
    "DataStructure/Hash_set.cpp": "preferred_data_hash_set_and_yuki",
    "Math/Xor_Basis.cpp": "xor_basis",
    "Graph/Floyd-Warshall.cpp": "coverage_graph_floyd_warshall_all_pairs_yuki",
    "Graph/Functional_Graph.cpp": "functional_graph",
    "Algorithm/rotate.cpp": "preferred_matrix_rotate_integer",
    "Graph/Project_Selection.cpp": "preferred_project_selection_yuki",
    "Graph/Centroid_Decomposition.cpp": "centroid_decomposition",
    "Geometry/Circle.cpp": "coverage_geometry_coin_overlap_split",
    "Graph/Pruefer_Code.cpp": "pruefer_code",
    "Graph/Min_Cost_Flow.cpp": "min_cost_flow_grid",
    "Geometry/Line_Segment.cpp": "coverage_geometry_segment_stabbing_split",
    "Algorithm/Knapsack.cpp": "knapsack_bounded_exact",
    "Algorithm/Subset_Sum.cpp": "subset_sum_cards",
    "Geometry/Minkowski_Sum.cpp": "minkowski_sum",
    "Graph/Dynamic_Bipartiteness.cpp": "dynamic_bipartiteness_operations",
    "Graph/Global_Min_Cut.cpp": "global_min_cut_operations",
    "Graph/Tree_B_Matching.cpp": "tree_b_matching",
    "Graph/Zero_One_BFS.cpp": "zero_one_bfs_wizard",
    "Math/Range_Xor_Basis.cpp": "range_xor_basis",
    "String/Edit_Distance.cpp": "bounded_edit_distance",
    "String/Longest_Common_Subsequence.cpp": "lcs_sequence",
    "String/Palindromic_Factorization.cpp": "palindromic_factorization_local",
    "String/Prefix_Function.cpp": "kmp_string_matching",
}
# name, full dimensions, quick dimensions, independent answer basis
_PROFILES = {
    "prefix_sum_3d": (
        ("affine", {"n": 100, "q": 200000}, {"n": 5, "q": 48}, "Sum affine cell values via independent integer intervals."),
        ("constant", {"n": 100, "q": 200000}, {"n": 5, "q": 48}, "Each answer is 999 times the query volume.")),
    "preferred_topological_yuki": (
        ("chain", {"n": 100000}, {"n": 16}, "Directed chain has longest vertex-count n."),
        ("many_sources", {"n": 100000}, {"n": 16}, "Edges from first half to disjoint second half give longest count2.")),
    "topological_lexicographical": (
        ("chain", {"n": 200000}, {"n": 16}, "Unique topological ordering1..n."),
        ("many_sources", {"n": 200000}, {"n": 16}, "Increasing labels are lexicographically first for disjoint forward edges.")),
    "all_pairs": (
        ("forward_chain", {"n": 100}, {"n": 8}, "Row i distances are0,1,..,n-i-1."),
        ("reverse_chain", {"n": 100}, {"n": 8}, "Row i distances are0,1,..,i; reverse propagation stresses Bellman-Ford.")),
    "slope_trick": (
        ("monotone", {"q": 200000}, {"q": 32}, "Median of0..t-1; absolute deviation sum floor(t²/4)."),
        ("alternating_extremes", {"q": 200000}, {"q": 32}, "Median -1e9; min2e9*floor(t/2), plus alternating constants.")),
    "preferred_trie_dictionary": (
        ("shared_prefixes", {"text_length": 50000, "patterns": 5000, "word_length": 10}, {"text_length": 64, "patterns": 32, "word_length": 8}, "Distinct A^1..A^L match n-k+1 times each; C..Z filler never matches."),
        ("periodic", {"text_length": 50000, "patterns": 5000, "word_length": 10}, {"text_length": 64, "patterns": 32, "word_length": 8}, "Both alternating words at each length partition all windows; C..Z filler never matches.")),
    "stabbing": (
        ("parallel", {"n": 100}, {"n": 8}, "All vertical segments intersect y=0, so answer n."),
        ("concurrent", {"n": 100}, {"n": 8}, "All segments contain origin, so answer n.")),
    "coverage_graph_node_cycles_yuki": (
        ("one_cycle", {"n": 200000}, {"n": 32}, "One directed cycle."),
        ("two_cycles", {"n": 200000}, {"n": 32}, "Disjoint2-cycles: n/2 cycles.")),
    "zeta_sieve_factorize_split": (
        ("large_smooth", {"value": 10**12, "sieve_bound": 1000000}, {"value": 10000, "sieve_bound": 100}, "2^12*5^12:4+4 removals; quick2^4*5^4:2+2."),
        ("prime_square", {"value": 999983**2, "sieve_bound": 999983}, {"value": 101**2, "sieve_bound": 101}, "Known prime square: one exponent removal.")),
    "preferred_data_hash_set_and_yuki": (
        ("growth_and_hits", {"q": 200000}, {"q": 32}, "Set0..q/2-1 has bitwise AND0."),
        ("erase_reuse", {"q": 200000}, {"q": 32}, "Singleton AND is its key; empty AND is-1.")),
    "xor_basis": (
        ("unit_basis", {"n": 100000, "bits": 60}, {"n": 64, "bits": 8}, "Paired vectors XOR to0 and span all chosen bits."),
        ("dense_basis", {"n": 100000, "bits": 60}, {"n": 64, "bits": 8}, "Paired prefix masks span all chosen bits and XOR to0.")),
    "functional_graph": (
        ("cycle_large_jump", {"n": 131071, "k": 10**18}, {"n": 31, "k": 10**18}, "Successor index is(i+k)mod n."),
        ("tail", {"n": 131071, "k": 43690}, {"n": 31, "k": 10}, "Successor index is min(i+k,n-1).")),
    "preferred_matrix_rotate_integer": (
        ("clockwise_rectangular", {"h": 10, "w": 9, "scale": 10, "rotation": 90}, {"h": 3, "w": 4, "scale": 2, "rotation": 90}, "Coordinate permutation then pixel replication."),
        ("counterclockwise_rectangular", {"h": 9, "w": 10, "scale": 10, "rotation": 270}, {"h": 4, "w": 3, "scale": 2, "rotation": 270}, "Independent inverse coordinate permutation then replication.")),
    "preferred_project_selection_yuki": (
        ("all_profitable", {"h": 40, "w": 40}, {"h": 3, "w": 4}, "Unit cell costs; each complete-row/column reward exceeds its entire cost, so choose all."),
        ("all_unprofitable", {"h": 40, "w": 40}, {"h": 3, "w": 4}, "Cell cost1e6 dominates every possible row/column reward, so choose none.")),
    "centroid_decomposition": (
        ("chain", {"n": 131071}, {"n": 31}, "2^k-1 chain has unique median in every recursive odd interval."),
        ("star", {"n": 131071}, {"n": 31}, "Unique center0; every leaf is a singleton child.")),
    "coverage_geometry_coin_overlap_split": (
        ("tangent_grid", {"n": 50000}, {"n": 32}, "Radius10 centers20 apart never overlap strictly, so retain all."),
        ("coincident", {"n": 50000}, {"n": 32}, "Only first coincident circle survives.")),
    "pruefer_code": (
        ("partial_chain", {"n": 200000}, {"n": 32}, "Complete known path degree sequence from alternating existing edges."),
        ("partial_star", {"n": 200000}, {"n": 32}, "Complete star degree sequence after half its spokes already exist.")),
    "min_cost_flow_grid": (
        ("all_cells", {"n": 50, "k": 50}, {"n": 4, "k": 4}, "Every weight positive and k=n, so choose all cells."),
        ("unique_diagonal", {"n": 50, "k": 1}, {"n": 4, "k": 1}, "Diagonal2/off-diagonal1 with k=1 has unique diagonal optimum2n.")),
    "knapsack_bounded_exact": (
        ("all_stocks_needed", {"n": 100, "target": 30000, "stock": 200}, {"n": 8, "target": 60, "stock": 5},
         "Each item weighs two. Pick the required number in descending order of value, respecting stock."),
        ("unreachable_odd_target", {"n": 100, "target": 29999, "stock": 200}, {"n": 8, "target": 59, "stock": 5},
         "All weights are even and the target is odd.")),
    "subset_sum_cards": (
        ("unique_all_back", {"n": 100, "back": 100}, {"n": 8, "back": 20},
         "Only turning every card to its larger side reaches the total upper bound."),
        ("near_max_unreachable", {"n": 100, "back": 100}, {"n": 8, "back": 20},
         "Every attainable sum is even; the target is one less than the total upper bound.")),
    "minkowski_sum": (
        ("box_with_duplicate_corners", {"n": 100000, "width": 5000000, "height": 8000000},
         {"n": 24, "width": 5, "height": 8}, "Every original corner is a division point of two identical points; all other points lie inside the box."),
        ("wide_collinear", {"n": 100000, "width": 5000000, "height": 0}, {"n": 24, "width": 5, "height": 0},
         "All division points belong to the same line.")),
    "dynamic_bipartiteness_operations": (
        ("long_odd_cycle_toggle", {"n": 200000, "rounds": 100000}, {"n": 12, "rounds": 14},
         "An even-length subpath and one temporary edge give an odd cycle until the edge is removed."),
        ("parallel_edges", {"n": 200000, "rounds": 100000}, {"n": 16, "rounds": 10},
         "Every active edge joins opposite parities as duplicates are added and original edges removed.")),
    "tree_b_matching": (
        ("high_degree_star", {"n": 300000}, {"n": 12}, "Only the center binds; take the largest n//3 edge weights."),
        ("alternating_chain", {"n": 300000}, {"n": 12}, "Every positive edge can be selected because they are vertex-disjoint.")),
    "global_min_cut_operations": (
        ("complete_unit", {"n": 450}, {"n": 8}, "Every cut of the complete graph has |S|*(n-|S|) unit edges."),
        ("disjoint_cliques", {"n": 450}, {"n": 8}, "A partition between the two complete components cuts no edges.")),
    "palindromic_factorization_local": (
        ("zimin_then_separator", {"n": (1 << 19) - 1}, {"n": 31},
         "A complete Zimin word is a palindrome; its distinct final separator requires another part."),
        ("no_long_palindrome", {"n": 500000}, {"n": 36},
         "The abc period has no length-two or length-three palindrome, so every part has length one.")),
    "zero_one_bfs_wizard": (
        ("all_open", {"side": 1000}, {"side": 12}, "An open path using only orthogonal steps costs no warps."),
        ("isolated_landings", {"side": 999}, {"side": 11},
         "Landing cells have both coordinates even. Each warp advances at most two rows or columns; diagonal warps meet the lower bound.")),
    "range_xor_basis": (
        ("unit_vectors", {"n": 400000, "q": 200000, "bits": 59}, {"n": 64, "q": 48, "bits": 7},
         "Each even query requests an element in the interval. Odd queries request a bit missing from every element."),
        ("dense_vectors", {"n": 400000, "q": 200000, "bits": 59}, {"n": 64, "q": 48, "bits": 7},
         "Dense low-bit prefixes still omit the top bit used by the negative queries.")),
    "bounded_edit_distance": (
        ("band_edge", {"n": 500000, "k": 20}, {"n": 55, "k": 4},
         "Inserting the extra prefix of length k is sufficient and the length difference gives a lower bound."),
        ("one_past_limit", {"n": 500000, "k": 20}, {"n": 55, "k": 4},
         "At least k+1 changes are needed to introduce k+1 copies of a missing letter.")),
    "lcs_sequence": (
        ("ordered_runs", {"n": 3000}, {"n": 24},
         "The only common letters are a and b in that order, so take the minimum count of each."),
        ("opposite_order", {"n": 3000}, {"n": 24},
         "The a and b runs occur in opposite order. The unique longest choice uses only the longer common run.")),
    "kmp_string_matching": (
        ("all_overlapping", {"text": 1000000, "pattern": 10000}, {"text": 120, "pattern": 16},
         "Equal-character strings match at every feasible starting index."),
        ("long_borders", {"text": 1000000, "pattern": 10000}, {"text": 120, "pattern": 16},
         "The only b is at the final text index and the end of the pattern, fixing one match.")),
}
MAX_BYTES = 32 * 1024 * 1024
_ROOT = Path(__file__).resolve().parents[2]
_VALIDITY = {
    "prefix_sum_3d": "N<=100, Q<=200000; values lie in[0,999], endpoints in[1,N].",
    "preferred_topological_yuki": "Forward edges on distinct vertices form a DAG; every edge has implicit unit multiplicity.",
    "topological_lexicographical": "Simple DAG with vertex labels1..n and at most n-1 edges.",
    "all_pairs": "N<=100, M=N-1; weights1 and acyclic edges exclude negative cycles.",
    "slope_trick": "Q<=200000; insertion coordinates lie in[-1e9,1e9], constants in[-7,7]; each query follows an insertion.",
    "preferred_trie_dictionary": "Uppercase text<=50000; <=5000 distinct nonempty uppercase patterns of length<=10.",
    "stabbing": "100 nonzero integer-coordinate segments at most; coordinates lie in[-100,100].",
    "coverage_graph_node_cycles_yuki": "Every vertex has exactly one valid successor in1..n.",
    "zeta_sieve_factorize_split": "1<=N<=10^12; required sieve bound is floor(sqrt(N)).",
    "preferred_data_hash_set_and_yuki": "Q<=400000; every key is nonnegative and less than2^30; erases remove registered keys.",
    "xor_basis": "N<=100000; every value is nonnegative and less than2^60.",
    "functional_graph": "N<=200000, 0<=K<=10^18; every successor and value lies in1..N.",
    "preferred_matrix_rotate_integer": "1<=H,W,scale<=10; pixels are ./# and rotation is90 or270 degrees.",
    "preferred_project_selection_yuki": "Positive integer cell costs and row/column rewards; rectangular matrix and reward lengths agree.",
    "centroid_decomposition": "N<=200000; simple connected tree has N-1 edges; odd chain components have unique centroids.",
    "coverage_geometry_coin_overlap_split": "Radius10; integer nonnegative centers, with tangent or coincident circle positions.",
    "pruefer_code": "N<=200000; existing edges form a forest contained in the explicit target tree; target degrees sum2(N-1).",
    "min_cost_flow_grid": "N<=50, 1<=K<=N; positive cell weights in[1,2].",
    "knapsack_bounded_exact": "N<=100, S<=30000, with positive weights, values, and stock inside AWC0103 D bounds.",
    "subset_sum_cards": "N<=100; card values<=100; target in[1,10000].",
    "minkowski_sum": "2<=N<=100000, p=1, q=2; all coordinate magnitudes are at most 10^7.",
    "dynamic_bipartiteness_operations": "Local driver: add valid endpoints, remove an active edge by addition ID, or query.",
    "tree_b_matching": "2<=N<=300000; vertex caps 1 or n//3 and weights within [-5,299999].",
    "global_min_cut_operations": "Local driver: valid endpoints and nonnegative uint64 edge weights.",
    "palindromic_factorization_local": "Local driver: one nonempty lowercase token; its length is less than INT_MAX.",
    "zero_one_bfs_wizard": "H,W<=1000; distinct start and end both on open cells.",
    "range_xor_basis": "N<=400000, Q<=200000; values and query targets in[1,2^60); nonempty one-based intervals.",
    "bounded_edit_distance": "1<=K<=20; lowercase nonempty strings of length<=500000.",
    "lcs_sequence": "Lowercase nonempty strings of length<=3000.",
    "kmp_string_matching": "Nonempty alphanumeric text<=1000000 and pattern<=10000.",
}
_UNREVERIFIED_LIMITS = {
    "preferred_topological_yuki", "coverage_graph_node_cycles_yuki",
    "preferred_project_selection_yuki", "coverage_geometry_coin_overlap_split",
}
_LOCAL_DRIVERS = {
    "dynamic_bipartiteness_operations", "global_min_cut_operations", "palindromic_factorization_local",
}


def _key(identifier: str) -> str:
    if identifier in {"coverage_graph_bellman_ford_all_pairs_yuki", "coverage_graph_spfa_all_pairs_yuki", "coverage_graph_floyd_warshall_all_pairs_yuki"}:
        return "all_pairs"
    if identifier in {"coverage_geometry_orientation_stabbing_split", "coverage_geometry_segment_stabbing_split"}:
        return "stabbing"
    return identifier


def plans_for_library(library: str, quick: bool = False) -> list[dict]:
    identifier = LIBRARY_VERIFY_IDS[library]
    checker = {"pruefer_code": "pruefer_completion", "min_cost_flow_grid": "min_cost_flow_grid"}.get(identifier, "tokens")
    return [{"name": identifier + "_" + name, "library": library, "verify_id": identifier,
             "scenario": name, "checker": checker, "dimensions": dict(small if quick else full),
             "seed": 20261116, "size_profile": "quick_smoke" if quick else "full",
             "scale_claim": "CI_workload_not_problem_max", "oracle": oracle,
             "scope": "whole_program_including_io",
             "constraints_status": ("local_operation_constraints" if identifier in _LOCAL_DRIVERS
                                    else "numerical_judge_bounds_not_reverified" if identifier in _UNREVERIFIED_LIMITS
                                    else "known_problem_constraints"),
             "constraint_note": _VALIDITY[_key(identifier)] + (
                 " Numerical judge bounds were not reverified; this is an existing-driver API workload."
                 if identifier in _UNREVERIFIED_LIMITS else "")}
            for name, full, small, oracle in _PROFILES[_key(identifier)]]


def representative_entries() -> dict[str, str]:
    return dict(LIBRARY_VERIFY_IDS)


def case_plans(verify_id: str, quick: bool = False) -> list[dict]:
    libraries = [library for library, identifier in LIBRARY_VERIFY_IDS.items() if identifier == verify_id]
    if len(libraries) != 1:
        raise ValueError("unknown or ambiguous performance Verify ID")
    return plans_for_library(libraries[0], quick)


class _Writer:
    def __init__(self, path: Path):
        self.path = path
        self.stream = path.open("wb")
        self.hash = hashlib.sha256()
        self.bytes = 0
    def line(self, *values):
        data = (" ".join(map(str, values)) + "\n").encode("ascii")
        self.bytes += len(data)
        if self.bytes > MAX_BYTES:
            raise ValueError("generated performance fixture exceeds size budget")
        self.stream.write(data); self.hash.update(data)
    def close(self):
        self.stream.close()


def _generate(plan: dict, inp: _Writer, out: _Writer) -> None:
    name, identifier, d = plan["scenario"], plan["verify_id"], plan["dimensions"]
    key = _key(identifier)
    if key == "prefix_sum_3d":
        for fields, answer in stress_cases.cuboid_rows(d["n"], d["q"], constant=name == "constant"):
            inp.line(*fields)
            if answer is not None:
                out.line(answer)
    elif key == "all_pairs":
        n = d["n"]; inp.line(n, n - 1)
        for i in range(n - 1): inp.line(i + 1, i + 2, 1) if name == "forward_chain" else inp.line(i + 2, i + 1, 1)
        for i in range(n):
            length = n - 1 - i if name == "forward_chain" else i
            out.line(length * (length + 1) // 2)
    elif key in {"preferred_topological_yuki", "topological_lexicographical"}:
        n = d["n"]
        edges = [(i, i + 1) for i in range(n - 1)] if name == "chain" else [(i, i + n // 2) for i in range(n // 2)]
        inp.line(n, len(edges))
        for a, b in edges: inp.line(a + 1, b + 1)
        if key == "preferred_topological_yuki": out.line(n if name == "chain" else 2)
        else: out.line(*range(1, n + 1))
    elif key == "slope_trick":
        q = d["q"]; inp.line(q)
        for t in range(1, q // 2 + 1):
            if name == "monotone":
                inp.line(1, t - 1, 0); inp.line(2); out.line((t - 1) // 2, t * t // 4)
            else:
                inp.line(1, -10**9 if t & 1 else 10**9, 7 if t & 1 else -7)
                inp.line(2); out.line(-10**9, 2 * 10**9 * (t // 2) + (7 if t & 1 else 0))
    elif key == "preferred_trie_dictionary":
        n, count, length = d["text_length"], d["patterns"], d["word_length"]
        text = "A" * n if name == "shared_prefixes" else ("AB" * ((n + 1) // 2))[:n]
        inp.line(text, count)
        matching = ["A" * k for k in range(1, length + 1)] if name == "shared_prefixes" else [
            (pair * ((k + 1) // 2))[:k] for k in range(1, length + 1) for pair in ("AB", "BA")]
        for word in matching: inp.line(word)
        for i in range(count - len(matching)):
            value = i; letters = []
            for _ in range(length):
                letters.append(chr(ord("C") + value % 24)); value //= 24
            inp.line("".join(letters))
        out.line(sum(n - k + 1 for k in range(1, length + 1)))
    elif key == "stabbing":
        n = d["n"]; inp.line(n)
        for i in range(n): inp.line(i, -100, i, 100) if name == "parallel" else inp.line(-i, -100, i, 100)
        out.line(n)
    elif key == "coverage_graph_node_cycles_yuki":
        n = d["n"]; inp.line(n)
        inp.line(*(i + 2 if i + 1 < n else 1 for i in range(n))) if name == "one_cycle" else inp.line(*(i + 2 if i % 2 == 0 else i for i in range(n)))
        out.line(1 if name == "one_cycle" else n // 2)
    elif key == "zeta_sieve_factorize_split":
        value = d["value"]; inp.line(value)
        out.line((8 if value == 10**12 else 4) if name == "large_smooth" else 1)
    elif key == "preferred_data_hash_set_and_yuki":
        q = d["q"]; inp.line(q)
        if name == "growth_and_hits":
            for i in range(q // 2): inp.line(1, i)
            for _ in range(q // 2): inp.line(3); out.line(0)
        else:
            for i in range(q // 4):
                value = i * 4096
                inp.line(1, value); inp.line(3); out.line(value)
                inp.line(2, value); inp.line(3); out.line(-1)
    elif key == "xor_basis":
        n, bits = d["n"], d["bits"]; inp.line(n)
        values = []
        for i in range(n // 2):
            b = i % bits
            value = 1 << b if name == "unit_basis" else (1 << (b + 1)) - 1
            values.extend((value, value))
        inp.line(*values); out.line(2 * ((1 << bits) - 1))
    elif key == "functional_graph":
        n, k = d["n"], d["k"]; inp.line(n, k)
        successor = [(i + 1) % n for i in range(n)] if name == "cycle_large_jump" else [min(i + 1, n - 1) for i in range(n)]
        inp.line(*(i + 1 for i in successor)); inp.line(*range(1, n + 1))
        out.line(*(((i + k) % n) + 1 if name == "cycle_large_jump" else min(i + k, n - 1) + 1 for i in range(n)))
    elif key == "preferred_matrix_rotate_integer":
        h, w, scale, rotation = d["h"], d["w"], d["scale"], d["rotation"]
        image = ["".join(".#"[(i * 3 + j) % 2] for j in range(w)) for i in range(h)]
        inp.line(rotation, scale, h, w)
        for row in image: inp.line(row)
        rotated = [["."] * h for _ in range(w)]
        for i in range(h):
            for j in range(w):
                if rotation == 90: rotated[j][h - 1 - i] = image[i][j]
                else: rotated[w - 1 - j][i] = image[i][j]
        for row in rotated:
            line = "".join(c * scale for c in row)
            for _ in range(scale): out.line(line)
    elif key == "preferred_project_selection_yuki":
        h, w = d["h"], d["w"]; inp.line(h, w)
        for _ in range(h): inp.line(*([1 if name == "all_profitable" else 10**6] * w))
        inp.line(*([w + 1] * h if name == "all_profitable" else range(1, h + 1)))
        inp.line(*([h + 1] * w if name == "all_profitable" else range(1, w + 1)))
        out.line(h * w + h + w if name == "all_profitable" else 0)
    elif key == "centroid_decomposition":
        n = d["n"]; inp.line(n)
        for i in range(1, n): inp.line(i, i + 1) if name == "chain" else inp.line(1, i + 1)
        parents = [1] * n; parents[0] = -1
        if name == "chain":
            todo = [(0, n, -1)]
            while todo:
                left, right, parent = todo.pop()
                if left == right: continue
                middle = (left + right) // 2; parents[middle] = -1 if parent == -1 else parent + 1
                todo.extend(((left, middle, middle), (middle + 1, right, middle)))
        out.line(*parents)
    elif key == "coverage_geometry_coin_overlap_split":
        n = d["n"]; inp.line(n)
        for i in range(n): inp.line(20 * (i % 500), 20 * (i // 500)) if name == "tangent_grid" else inp.line(0, 0)
        out.line(n if name == "tangent_grid" else 1)
    elif key == "pruefer_code":
        n = d["n"]
        if name == "partial_chain":
            edges = [(i, i + 1) for i in range(0, n - 1, 2)]
            remaining = [(i, i + 1) for i in range(1, n - 1, 2)]
            degrees = [1] + [2] * (n - 2) + [1]
        else:
            edges = [(0, i) for i in range(1, n // 2 + 1)]
            remaining = [(0, i) for i in range(n // 2 + 1, n)]
            degrees = [n - 1] + [1] * (n - 1)
        inp.line(n, len(edges)); inp.line(*degrees)
        for a, b in edges: inp.line(a + 1, b + 1)
        for a, b in remaining: out.line(a + 1, b + 1)
    elif key == "min_cost_flow_grid":
        n, k = d["n"], d["k"]; inp.line(n, k)
        for i in range(n): inp.line(*(1 if name == "all_cells" else (2 if i == j else 1) for j in range(n)))
        out.line(n * n if name == "all_cells" else 2 * n)
        for i in range(n): out.line("X" * n if name == "all_cells" else "." * i + "X" + "." * (n - i - 1))
    elif key == "knapsack_bounded_exact":
        n, target, stock = d["n"], d["target"], d["stock"]
        inp.line(n, target)
        for value in range(1, n + 1):
            inp.line(2, value, stock)
        if target % 2:
            out.line(-1)
        else:
            complete, remainder = divmod(target // 2, stock)
            out.line(stock * sum(range(n - complete + 1, n + 1)) + remainder * (n - complete))
    elif key == "subset_sum_cards":
        n, back = d["n"], d["back"]
        reachable = name == "unique_all_back"
        inp.line(n, n * back if reachable else n * back - 1)
        for _ in range(n):
            inp.line(back - (1 if reachable else 2), back)
        out.line("Yes" if reachable else "No")
        if reachable:
            out.line("T" * n)
    elif key == "minkowski_sum":
        n, width, height = d["n"], d["width"], d["height"]
        corners = ((0, 0), (width, height), (0, height), (width, 0))
        inp.line(n, 1, 2)
        for i in range(n):
            inp.line(*corners[i % len(corners)])
        out.line(width * height, 1)
    elif key == "dynamic_bipartiteness_operations":
        n, rounds = d["n"], d["rounds"]
        inp.line(n, n - 1 + 4 * rounds)
        for i in range(n - 1):
            inp.line(0, i, i + 1)
        if name == "long_odd_cycle_toggle":
            for i in range(rounds):
                inp.line(0, 0, n - 2)
                inp.line(2)
                inp.line(1, n - 1 + i)
                inp.line(2)
                out.line("No")
                out.line("Yes")
        else:
            for i in range(rounds):
                inp.line(0, i, i + 1)
                inp.line(2)
                out.line("Yes")
            for i in range(rounds):
                inp.line(1, i)
                inp.line(2)
                out.line("Yes")
    elif key == "tree_b_matching":
        n = d["n"]
        inp.line(n)
        if name == "high_degree_star":
            cap = n // 3
            inp.line(cap, *([1] * (n - 1)))
            for i in range(1, n):
                inp.line(1, i + 1, i)
            out.line(cap * (2 * n - cap - 1) // 2)
        else:
            inp.line(*([1] * n))
            for i in range(1, n):
                inp.line(i, i + 1, 3 if i % 2 else -5)
            out.line(3 * (n // 2))
    elif key == "global_min_cut_operations":
        n = d["n"]
        split = name == "disjoint_cliques"
        half = n // 2
        groups = [(0, half), (half, n)] if split else [(0, n)]
        count = sum((last - first) * (last - first - 1) // 2 for first, last in groups)
        inp.line(n, count)
        for first, last in groups:
            for u in range(first, last):
                for v in range(u + 1, last):
                    inp.line(u, v, 1)
        out.line(0 if split else n - 1)
    elif key == "palindromic_factorization_local":
        n = d["n"]
        if name == "zimin_then_separator":
            letters = "abcdefghijklmnopqrstuvwxy"
            inp.line("".join(letters[(index & -index).bit_length() - 1] for index in range(1, n + 1)) + "z")
            out.line(2)
        else:
            inp.line(("abc" * ((n + 2) // 3))[:n])
            out.line(n)
    elif key == "zero_one_bfs_wizard":
        side = d["side"]
        inp.line(side, side)
        inp.line(1, 1)
        inp.line(side, side)
        if name == "all_open":
            for _ in range(side):
                inp.line("." * side)
            out.line(0)
        else:
            landings = (".#" * ((side + 1) // 2))[:side]
            wall = "#" * side
            for i in range(side):
                inp.line(wall if i % 2 else landings)
            out.line((side - 1) // 2)
    elif key == "range_xor_basis":
        n, count, bits = d["n"], d["q"], d["bits"]
        unit = name == "unit_vectors"
        values = [(1 << (i % bits)) if unit else (1 << (i % bits + 1)) - 1 for i in range(n)]
        inp.line(n, count)
        inp.line(*values)
        for i in range(count):
            left = i * 65537 % n
            width = min(4 * bits, n - left)
            right = left + 1 + i * 43 % width
            inp.line(left + 1, right, (1 << bits) if i % 2 else values[right - 1])
            out.line("No" if i % 2 else "Yes")
    elif key == "bounded_edit_distance":
        n, limit = d["n"], d["k"]
        inp.line(limit)
        if name == "band_edge":
            inp.line("a" * (n - limit))
            inp.line("b" * limit + "a" * (n - limit))
            out.line("Yes")
        else:
            inp.line("a" * n)
            inp.line("b" * (limit + 1) + "a" * (n - limit - 1))
            out.line("No")
    elif key == "lcs_sequence":
        n = d["n"]
        a = "a" * (n // 3)
        b = "b" * (n - len(a))
        inp.line(a + b)
        if name == "ordered_runs":
            inp.line(a[:-1] + "c" + b)
            out.line(a[:-1] + b)
        else:
            inp.line(b + a)
            out.line(b)
    elif key == "kmp_string_matching":
        text, pattern = d["text"], d["pattern"]
        if name == "all_overlapping":
            inp.line("a" * text)
            inp.line("a" * pattern)
            for i in range(text - pattern + 1):
                out.line(i)
        else:
            inp.line("a" * (text - 1) + "b")
            inp.line("a" * (pattern - 1) + "b")
            out.line(text - pattern)
    else:
        raise ValueError("unimplemented performance case")


def generate_case(plan: dict, directory: Path) -> dict:
    library = plan.get("library")
    if library not in LIBRARY_VERIFY_IDS or plan not in plans_for_library(library) + plans_for_library(library, True):
        raise ValueError("case differs from reviewed catalog")
    directory = Path(directory).resolve()
    if directory.is_relative_to(_ROOT):
        raise ValueError("performance inputs must stay outside repository")
    directory.mkdir(parents=True, exist_ok=True)
    inp, out = _Writer(directory / "input.txt"), _Writer(directory / "expected.txt")
    try:
        _generate(plan, inp, out)
    finally:
        inp.close(); out.close()
    if not inp.bytes or not out.bytes:
        raise ValueError("empty input or expected output")
    return {"input_path": str(inp.path), "expected_path": str(out.path),
            "input_sha256": inp.hash.hexdigest(), "expected_sha256": out.hash.hexdigest(),
            "input_bytes": inp.bytes, "expected_bytes": out.bytes, "checker": plan["checker"],
            "dimensions": plan["dimensions"], "seed": plan["seed"],
            "oracle": plan["oracle"], "scale_claim": plan["scale_claim"]}
