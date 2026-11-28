# SPDX-License-Identifier: CC0-1.0
"""Deterministic workloads for structures and numeric order queries without LC cases."""

from collections import Counter
from functools import cache
from itertools import combinations
from math import fsum, gcd, hypot

LIBRARY_VERIFY_IDS = {
    "Geometry/Delaunay_Triangulation.cpp": "delaunay_triangulation_reference",
    "String/DFA_Minimization.cpp": "dfa_minimization_reference",
    "Math/Labeled_Tree_Matching_Counts.cpp": "labeled_tree_matching_counts",
    "Geometry/Trapezoid_Count.cpp": "trapezoid_count",
    "Math/Todd_Coxeter.cpp": "todd_coxeter_reference",
    "String/Lexicographic_Concatenation.cpp": "lexicographic_concatenation_second",
    "String/Lexicographic_String_Shuffle.cpp": "lexicographic_string_shuffle",
    "String/Bitmask_NFA.cpp": "bitmask_nfa_wildcard",
    "String/Regex_Language_Edit_Distance.cpp": "regex_language_edit_distance",
    "Geometry/Maximum_Weight_Window.cpp": "maximum_weight_window",
    "Geometry/Integer_Geometry_3D.cpp": "point_plane_distance_sum",
    "DataStructure/Weighted_L1_Statistics.cpp": "weighted_l1_statistics_absolute_minima",
    "Geometry/L1_Origin_Line_Fit.cpp": "l1_origin_line_fit",
    "DataStructure/Weighted_Slope_Trick.cpp": "weighted_slope_trick_selling_cars",
    "Algorithm/Narrow_Grid_Domino_Packings.cpp": "narrow_grid_domino_packings",
    "DataStructure/Grid_Connectivity_Frontier.cpp": "grid_corner_connected_colorings",
    "Graph/Tournament_Path.cpp": "tournament_path_reference",
    "DataStructure/Successor_Set.cpp": "successor_set_linear_probing",
    "DataStructure/Range_Add_Weighted_Minimum.cpp": "range_add_weighted_minimum",
    "Algorithm/Eytzinger_Search.cpp": "eytzinger_search",
    "DataStructure/Range_Add_State_Ids.cpp": "range_add_state_ids",
    "DataStructure/Range_Add_Range_Gcd.cpp": "range_add_range_gcd",
    "Algorithm/Disjoint_Subarray_Max_Sum.cpp": "disjoint_subarray_max_sum",
    "Algorithm/Permutation_Order.cpp": "permutation_order_roundtrip",
    "Algorithm/Pair_Xor_Order.cpp": "pair_xor_order",
    "DataStructure/Binary_Range_Statistics.cpp": "binary_range_statistics",
    "DataStructure/Interval_Set.cpp": "interval_set_range_mex",
    "DataStructure/Monotone_Line_Container.cpp": "monotone_line_container_frog",
    "DataStructure/Meldable_Heap.cpp": "meldable_heap_tree_game",
    "DataStructure/Offline_Li_Chao_Tree.cpp": "offline_li_chao_tree_coins",
    "DataStructure/Top_K_Sum.cpp": "top_k_sum_sliding",
    "DataStructure/Persistent_Stack.cpp": "persistent_stack_notebook",
    "DataStructure/Persistent_Weighted_Union_Find.cpp": "persistent_weighted_union_find_branching",
    "DataStructure/Rollback_Lazy_Segment_Tree.cpp": "rollback_lazy_segment_tree",
    "DataStructure/Sparse_Table_2D.cpp": "sparse_table_2d_stamps",
    "DataStructure/Segment_Tree_2D.cpp": "segment_tree_2d_rectangle_minimum",
    "DataStructure/Range_Fenwick_Tree.cpp": "range_fenwick_tree",
    "DataStructure/Union_Find_Order_Statistics.cpp": "union_find_order_statistics_rank",
}

PROFILES = {
    "delaunay_triangulation_reference": (
        ("rectangular_grid_cocircular", {"width": 500, "height": 400}, {"width": 5, "height": 4},
         "A grid has 3n-3-h edges with h boundary points, including four-point circles in every unit square."),
        ("repeated_collinear_points", {"n": 200000, "distinct": 25001}, {"n": 30, "distinct": 9},
         "Only consecutive distinct points have edges, even when each coordinate repeats.")),
    "dfa_minimization_reference": (
        ("distinct_residues_on_cycle", {"n": 200000}, {"n": 18},
         "Only state 0 is accepting. Different distances to 0 distinguish all residues."),
        ("paired_equivalent_cycle_states", {"n": 200000}, {"n": 20},
         "The second symbol flips an irrelevant low bit, so each pair has the same language.")),
    "labeled_tree_matching_counts": (
        ("fixed_root_newton_doublings", {"n": 4096}, {"n": 6},
         "Independent quadratic exponential differentiation computes every fixed-root coefficient."),
        ("all_labeled_sizes_up_to_limit", {"n": 250000}, {"n": 6},
         "Cayley's formula minus the closed count from pairing labels and contracting the matching.")),
    "trapezoid_count": (
        ("single_integer_parabola", {"n": 2000}, {"n": 8},
         "Chords have slope equal to the sum of their endpoints' parameters, and all their midpoints are distinct."),
        ("centrally_symmetric_raised_parabolas", {"n": 2000}, {"n": 10},
         "Opposite half-curves have disjoint chord slope ranges from cross-half chords and share n/2 diameters.")),
    "todd_coxeter_reference": (
        ("long_cycle_with_length_wrap", {"period": 4096}, {"period": 9},
         "The monoid is addition modulo the length of the only relation, so length differences determine equality."),
        ("many_temporary_vertices_all_identified", {"period": 200000}, {"period": 20},
         "A long initial relation creates temporarily distinct vertices, then all generators collapse to the identity.")),
    "lexicographic_concatenation_second": (
        ("one_long_word_many_equal_short_words", {"n": 300000, "total": 1000000}, {"n": 5, "total": 28},
         "All the single a words go first and admit a tie; the only b ends the result."),
        ("largest_number_of_two_word_tests", {"n": 300000, "total": 1000000}, {"n": 8, "total": 22},
         "The larger permutation of each pair puts its b-prefixed word before the a word.")),
    "lexicographic_string_shuffle": (
        ("all_equal_long_prefixes", {"n": 50, "length": 50}, {"n": 3, "length": 3},
         "Every unfinished word must produce all its leading z before its first a, so finish one word at a time."),
        ("sorted_overlapping_words", {"n": 50, "length": 50}, {"n": 4, "length": 4},
         "Each input word is sorted; the sorted union is reachable and is a lower bound on every ordering.")),
    "bitmask_nfa_wildcard": (
        ("ambiguous_word_boundaries", {"n": 2000, "block": 8}, {"n": 12, "block": 4},
         "Every binary word is accepted by the all-X decomposition, including words with more than one valid decomposition."),
        ("overlapping_block_languages", {"n": 2000, "block": 8}, {"n": 12, "block": 4},
         "Equal-length blocks choose X when possible, otherwise Y. A pair of Y-only blocks is forbidden.")),
    "regex_language_edit_distance": (
        ("fixed_mismatches", {"n": 2000}, {"n": 8},
         "Two equally long words use disjoint letters, so every letter needs a unit-cost edit."),
        ("two_star_runs_with_mandatory_tail", {"n": 2000}, {"n": 12},
         "Every added c costs one edit, and choosing no repeated letters attains this bound.")),
    "maximum_weight_window": (
        ("wide_window_negative_fences", {"n": 100000}, {"n": 18},
         "A window centered at x=0 takes every positive point and avoids the negative fences at x=±1e9."),
        ("disjoint_signed_squares", {"n": 100000}, {"n": 20},
         "Between consecutive blocks is a gap wider than the square side; one bottom row has two positive points.")),
    "point_plane_distance_sum": (
        ("unit_moment_curve", {"n": 300}, {"n": 8},
         "The cubic polynomial with roots i,j,k supplies the exact three-point plane equation."),
        ("anisotropic_moment_curve", {"n": 300}, {"n": 11},
         "Scale each axis differently before using the exact factored cubic to form each distance.")),
    "weighted_l1_statistics_absolute_minima": (
        ("distinct_increasing_coordinates", {"q": 200000}, {"q": 40},
         "The minimum sum of distances from consecutive integers is floor(count^2/4)."),
        ("alternating_extreme_coordinates", {"q": 200000}, {"q": 32},
         "The lower median is always the negative extreme; distance is twice the absolute extreme per positive point.")),
    "l1_origin_line_fit": (
        ("oblique_line_with_unit_outliers", {"n": 200000}, {"n": 24},
         "Each pair on slope 2/3 outweighs a same-x point one unit above that line; its total distance is the outlier count."),
        ("dominant_vertical_axis", {"n": 200000}, {"n": 20},
         "Three quarters of the points are on the vertical axis; the horizontal minority is one million units from it.")),
    "weighted_slope_trick_selling_cars": (
        ("buyers_all_right_of_supply", {"n": 2000}, {"n": 12},
         "Every chosen car decreases the price by its color, so take the last slots in stock order."),
        ("alternating_buyer_supply", {"n": 2000}, {"n": 9},
         "Disjoint closest cars are always one unit away and each buyer needs just one car.")),
    "narrow_grid_domino_packings": (
        ("largest_width", {"width": 10**18}, {"width": 2},
         "Evaluate the order-15 recurrence obtained from an independent enumeration of column masks."),
        ("every_low_bit_set", {"width": (1 << 59) - 1}, {"width": 4},
         "The certified recurrence also gives the answer for a width with all 59 low bits set.")),
    "grid_corner_connected_colorings": (
        ("width_six_all_states", {"height": 6, "width": 100}, {"height": 4, "width": 4},
         "An independent column-at-once model merges horizontal links and vertical runs before checking connectivity."),
        ("long_ladder_source_choices", {"height": 2, "width": 100}, {"height": 2, "width": 7},
         "On a ladder all black cells on one column form one run, yielding three scalar recurrence states.")),
    "tournament_path_reference": (
        ("maximum_merge_queries", {"n": 500}, {"n": 29},
         "Each recursive split contributes its two paths alternately, with both heads present until the last comparison."),
        ("reverse_vertex_order", {"n": 500}, {"n": 31},
         "Every higher-numbered vertex beats each lower-numbered vertex, so the unique path is descending.")),
    "successor_set_linear_probing": (
        ("cluster_wraps_around", {"q": 200000}, {"q": 34},
         "Identical probes starting at the last position fill that position, then a prefix of the table."),
        ("pairs_of_wide_keys", {"q": 200000}, {"q": 36},
         "Disjoint pairs start at even positions; the second insertion in each pair advances one position.")),
    "range_add_weighted_minimum": (
        ("same_row_column_events", {"n": 200000, "q": 200000}, {"n": 23, "q": 23},
         "The full set of column strips covers every cell, and all strips start in the first row."),
        ("shifting_row_prefixes", {"n": 200000, "q": 200000}, {"n": 27, "q": 27},
         "Each rectangle erases a prefix of a different row. Its unpainted suffix has a triangular-number sum.")),
    "eytzinger_search": (
        ("distinct_scrambled_thresholds", {"n": 200000, "q": 200000}, {"n": 57, "q": 50},
         "The values are exactly the first N positive odd numbers."),
        ("many_equal_keys", {"n": 200000, "q": 200000}, {"n": 43, "q": 31},
         "Only a threshold of 1 includes the small keys; every threshold includes the largest keys.")),
    "range_add_state_ids": (
        ("distinct_history", {"n": 1000000, "q": 100000}, {"n": 37, "q": 35},
         "Every positive addition strictly increases the total, so all updated states have different IDs."),
        ("reversible_distinct_slices", {"n": 1000000, "q": 100000}, {"n": 35, "q": 32},
         "Each new slice starts at an unused endpoint. The second operation cancels it completely.")),
    "range_add_range_gcd": (
        ("moving_delta_boundaries", {"n": 100000, "q": 100000}, {"n": 33, "q": 30},
         "Each update creates a run of 30 inside the base value 18, so crossing a boundary gives gcd 6."),
        ("wide_alternating_values", {"n": 100000, "q": 100000}, {"n": 39, "q": 36},
         "Whole-array updates preserve the difference of 1 between adjacent values, forcing gcd 1 on longer ranges.")),
    "disjoint_subarray_max_sum": (
        ("all_positive_many_splits", {"n": 200000, "q": 100000}, {"n": 24, "q": 18},
         "A positive array can be fully covered with any positive number of nonempty disjoint intervals."),
        ("alternating_deep_gaps", {"n": 200000, "q": 100000}, {"n": 21, "q": 22},
         "Each negative gap costs more than the next positive element. Extra intervals select negative singletons.")),
    "permutation_order_roundtrip": (
        ("ascending", {"n": 200000}, {"n": 20}, "Every Lehmer digit is zero in the first permutation."),
        ("descending", {"n": 200000}, {"n": 23}, "Every Lehmer digit equals the number of remaining positions.")),
    "pair_xor_order": (
        ("complete_bit_cube", {"bits": 17, "target": 43111}, {"bits": 5, "target": 11},
         "A full cube contains N/2 unordered pairs for each nonzero XOR."),
        ("two_large_equal_groups", {"n": 200000}, {"n": 32},
         "Each equal-value group contributes only zero XOR; cross-group pairs all have XOR 2^29.")),
    "binary_range_statistics": (
        ("unaligned_uniform_run", {"n": 500000, "q": 100000}, {"n": 29, "q": 31},
         "After flipping an interval in the zero string, the longest ones run is its intersection with the query."),
        ("alternating_boundary_pairs", {"n": 500000, "q": 100000}, {"n": 31, "q": 28},
         "An alternating string can create consecutive ones only at one of the two flip boundaries.")),
    "interval_set_range_mex": (
        ("shrinking_prefix", {"n": 100000}, {"n": 24},
         "Point x holds precisely the values 0..N-x before the tail is removed."),
        ("staggered_even_values", {"n": 100000}, {"n": 24},
         "Even values create disjoint intervals. The mex is 1 while 0 is active and 0 after it expires.")),
    "monotone_line_container_frog": (
        ("short_jumps", {"n": 200000}, {"n": 39},
         "For C=1 and consecutive heights, each jump of length d costs at least 2d, with equality at one."),
        ("one_jump", {"n": 200000}, {"n": 36},
         "The maximum jump fee exceeds every squared height difference, so a direct jump beats any two jumps.")),
    "meldable_heap_tree_game": (
        ("root_star_second_maximum", {"n": 200000}, {"n": 27},
         "The first turn removes the largest leaf value, leaving the second largest."),
        ("long_broom_removes_maxima", {"n": 200000, "path": 40000}, {"n": 31, "path": 7},
         "One maximum leaf disappears per handle vertex, and all other handle vertices have value 1.")),
    "persistent_weighted_union_find_branching": (
        ("wide_version_fanout", {"n": 200000, "q": 100000, "k": 10**9}, {"n": 29, "q": 60, "k": 23},
         "Each branch from version zero contains one constraint, and alternate queries still see version zero."),
        ("deep_balanced_components", {"n": 200000, "q": 100000, "k": 10**9, "size": 65536},
         {"n": 29, "q": 54, "k": 23, "size": 16},
         "Every accepted edge links endpoints of a common modular prefix function. A shifted difference contradicts it.")),
    "rollback_lazy_segment_tree": (
        ("million_point_writes_and_resets", {"n": 100000, "m": 100000, "groups": 100000, "steps": 10},
         {"n": 37, "m": 23, "groups": 29, "steps": 10},
         "Each group keeps an independent map of point changes. Both the update and query use the same point."),
        ("million_unaligned_range_writes", {"n": 100000, "m": 100000, "groups": 100000, "steps": 10},
         {"n": 39, "m": 27, "groups": 31, "steps": 10},
         "Track at most two new interval boundaries per update on a constant initial array and reset them after each group.")),
    "offline_li_chao_tree_coins": (
        ("repeated_empty_envelope", {"rounds": 75000}, {"rounds": 37},
         "One coin at distance one produces its face value, and deleting it leaves no candidate."),
        ("overlapping_coin_lifetimes", {"rounds": 100000}, {"rounds": 36},
         "Only the two nearest coins can win: their weights are at least 1000, while farther coins have weight below 2000 and distance at least three.")),
    "top_k_sum_sliding": (
        ("increasing_windows", {"n": 200000, "width": 100001, "k": 50000},
         {"n": 40, "width": 21, "k": 10}, "The smallest K values form an arithmetic progression."),
        ("repeated_extremes", {"n": 200000, "width": 100001, "k": 50001},
         {"n": 40, "width": 21, "k": 11}, "Count low values by parity, then fill any remaining slots with 1e9.")),
    "persistent_stack_notebook": (
        ("deep_growth_then_delete", {"rounds": 250000}, {"rounds": 20},
         "Push consecutive positive values; popping returns the previous value down to the empty stack."),
        ("many_saved_branches", {"rounds": 124999}, {"rounds": 12},
         "Every branch loads the fixed first notebook page and deletes the new last value.")),
    "sparse_table_2d_stamps": (
        ("large_uniform_grid", {"h": 1000, "w": 1000, "black_h": 257, "black_w": 382,
                                "white_h": 1, "white_w": 1},
         {"h": 9, "w": 7, "black_h": 4, "black_w": 5, "white_h": 1, "white_w": 1},
         "On a uniform grid the score is the uncancelled black area times the cell value."),
        ("clipped_affine_grid", {"h": 1000, "w": 973, "black_h": 39, "black_w": 201,
                                 "white_h": 60, "white_w": 32},
         {"h": 8, "w": 9, "black_h": 3, "black_w": 6, "white_h": 4, "white_w": 2},
         "Translation toward the bottom right improves every remaining cell. The white stamp cancels the rightmost columns.")),
    "segment_tree_2d_rectangle_minimum": (
        ("square_crossing_boundaries", {"h": 1000, "w": 1000, "q": 100}, {"h": 11, "w": 10, "q": 60},
         "Monotone decreasing danger puts the minimum at the bottom-right corner of each rectangle."),
        ("wide_crossing_boundaries", {"h": 4, "w": 250000, "q": 10000}, {"h": 4, "w": 19, "q": 60},
         "Monotone increasing danger puts the minimum at the top-left corner across a wide table.")),
    "range_fenwick_tree": (
        ("growing_prefixes", {"n": 100000, "rounds": 50000}, {"n": 23, "rounds": 17},
         "Start with 5 everywhere and accumulate each prefix length times the added amount."),
        ("moving_singletons", {"n": 100000, "rounds": 50000}, {"n": 29, "rounds": 17},
         "Earlier assignments all lie strictly left of the suffix being queried.")),
    "union_find_order_statistics_rank": (
        ("component_ranks_balanced_merges", {"n": 200000}, {"n": 22},
         "The equal-size and tail merges leave one component containing every vertex."),
        ("component_ranks_mixed_operations", {"n": 200000}, {"n": 24},
         "The component of vertex 1 is a growing prefix, and vertex N stays isolated.")),
}

VALIDITY = {
    "delaunay_triangulation_reference": "Local points with N<=200000 and an axis span of at most 2e9.",
    "dfa_minimization_reference": "Local total binary automata with even state count 2<=N<=200000 and state numbers in [0,N).",
    "labeled_tree_matching_counts": "2<=U<=250000 and type is 1 or 2. Output the result for every size from 2 through U.",
    "trapezoid_count": "4<=N<=2000, integer coordinates in [0,10000000], distinct points, and no three collinear.",
    "todd_coxeter_reference": "Local finite presentations with 1 to 4 generators, at most 200000 symbols per relation side, and at most 200005 temporary vertices.",
    "lexicographic_concatenation_second": "1<=T<=150000, 2<=N each, sum(N)<=300000, total length<=1000000, nonempty lowercase words.",
    "lexicographic_string_shuffle": "1<=N<=50, 1<=len(S_i)<=50, and every character is a lowercase ASCII letter.",
    "bitmask_nfa_wildcard": "1<=len(S)<=2000, 1<=len(X),len(Y)<=8, and all characters are 0, 1, or ?.",
    "regex_language_edit_distance": "Each pattern has 1 to 2000 bytes and uses lowercase letters with optional ? or * suffixes.",
    "maximum_weight_window": "1<=N<=100000, coordinates and signed weights in [-1e9,1e9], and side length in [1,1e9].",
    "point_plane_distance_sum": "Upper N=300, six decimal places and coordinates in [-1000,1000]; moment-curve points are never four on a plane.",
    "weighted_l1_statistics_absolute_minima": "1<=Q<=200000, updates have signed coordinates and constants in [-1e9,1e9], and the first query is an update.",
    "l1_origin_line_fit": "2<=N<=200000 and each coordinate is in [-1e6,1e6].",
    "weighted_slope_trick_selling_cars": "1<=M=N<=2000, each color is in [1,1e9], and both lists have distinct entries.",
    "narrow_grid_domino_packings": "The grid has three rows and 1<=N<=10^18 columns, with no obstacles.",
    "grid_corner_connected_colorings": "2<=H<=6 and 2<=W<=100; corners are forced to be black and at least one path must connect them.",
    "tournament_path_reference": "Local driver reads an N by N zero/one adjacency matrix with one oriented edge for each distinct pair.",
    "successor_set_linear_probing": "Q<=200000, the table has 2^20 cells, keys are in [0,1e18], and at least one read is present.",
    "range_add_weighted_minimum": "N=Q<=200000; every rectangle is nonempty and lies within the N by N grid.",
    "eytzinger_search": "1<=N,Q<=200000, all values and thresholds are in [1,1e9].",
    "range_add_state_ids": "N<=1000000, Q<=100000, each update satisfies 0<=L<R<=N and |K|<=100.",
    "range_add_range_gcd": "N,M<=100000, values stay in [1,1e9] and query endpoints are 1-based and nonempty.",
    "disjoint_subarray_max_sum": "Local driver accepts up to 200000 signed elements and counts in [0, N].",
    "permutation_order_roundtrip": "Local driver accepts a permutation of 0..N-1; round-trip preserves every item.",
    "pair_xor_order": "2<=N<=200000; values are in[0,1e9]; ranks are within N(N-1)/2.",
    "binary_range_statistics": "1<=N<=500000, 1<=Q<=100000, S is binary; all queried intervals are nonempty.",
    "range_fenwick_tree": "1<=N,Q<=100000; initial values and positive changes are within the official bounds.",
    "interval_set_range_mex": "1<=N,Q<=100000; operation bounds and strictly increasing query points lie in[1,1e9].",
    "monotone_line_container_frog": "2<=N<=200000, 1<=C<=10^12, and strictly increasing heights in [1,10^6].",
    "meldable_heap_tree_game": "2<=N<=200000; a simple connected tree and 1<=A_i<=10^9 for non-root vertices.",
    "offline_li_chao_tree_coins": "1<=Q<=300000; positions and weights in [0,10^9]; queried positions are unoccupied.",
    "top_k_sum_sliding": "1<=K<=M<=N<=200000; all values in[1,1e9].",
    "persistent_stack_notebook": "1<=Q<=500000; pushed values and page numbers are positive and at most 1e9.",
    "persistent_weighted_union_find_branching": "N<=200000, Q<=100000, 2<=K<=10^9, 0<=B<i, 1<=L<=R<=N and 0<=X<K.",
    "rollback_lazy_segment_tree": "1<=N,M,Q<=100000, 1<=steps<=1000, Q*steps<=10^6; valid 1-based indices and 30-bit values.",
    "sparse_table_2d_stamps": "2<=H,W<=1000; stamp dimensions are within the grid and weights within[1,1e9].",
    "segment_tree_2d_rectangle_minimum":
        "H*W<=1e6, Q<=10000; Q<=100 when H*W<=250000; valid inclusive coordinates and int32 nonnegative values.",
    "union_find_order_statistics_rank": "N,Q<=200000; vertex IDs in[1,N] and ranks in[1,10].",
}


def interval_cases(scenario, dims, inp, out):
    n = dims["n"]
    inp.line(n)
    prefix = scenario == "shrinking_prefix"
    for i in range(n):
        if prefix:
            inp.line(1, n - i, i)
        else:
            inp.line(i + 1, i + n // 2, 2 * i)
    inp.line(n)
    inp.line(*range(1, n + 1))
    for x in range(1, n + 1):
        out.line(n - x + 1 if prefix else int(x <= n // 2))


def monotone_line_cases(scenario, dims, inp, out):
    n = dims["n"]
    if scenario == "short_jumps":
        inp.line(n, 1)
        inp.line(*range(1, n + 1))
        out.line(2 * (n - 1))
    else:
        inp.line(n, 10**12)
        inp.line(*range(5, 5 * n + 1, 5))
        out.line(10**12 + (5 * (n - 1))**2)


def offline_li_chao_cases(scenario, dims, inp, out):
    rounds = dims["rounds"]
    if scenario == "repeated_empty_envelope":
        inp.line(4 * rounds)
        for i in range(rounds):
            inp.line(1, 3 * i, 10**9)
            inp.line(3, 3 * i + 1)
            inp.line(2, 3 * i)
            inp.line(3, 3 * i + 1)
            out.line("1000000000.000000000000")
            out.line("0.000000000000")
        return
    inp.line(3 * rounds)
    for i in range(rounds):
        inp.line(1, 2 * i, 1000 + i % 999)
    for i in range(rounds):
        inp.line(3, 2 * i + 1)
        inp.line(2, 2 * i)
        right = 1000 + (i + 1) % 999 if i + 1 < rounds else 0
        out.line(f"{max(1000 + i % 999, right)}.000000000000")


def top_k_cases(scenario, dims, inp, out):
    n, width, k = (dims[name] for name in ("n", "width", "k"))
    inp.line(n, width, k)
    ascending = scenario == "increasing_windows"
    if ascending:
        inp.line(*range(1, n + 1))
    else:
        inp.line(*(10**9 if i % 2 else 1 for i in range(n)))
    for start in range(n - width + 1):
        if ascending:
            out.line(k * (start + 1) + k * (k - 1) // 2)
        else:
            low = min(k, (start + width + 1) // 2 - (start + 1) // 2)
            out.line(low + (k - low) * 10**9)


def stack_cases(scenario, dims, inp, out):
    n = dims["rounds"]
    if scenario == "deep_growth_then_delete":
        inp.line(2 * n)
        for value in range(1, n + 1):
            inp.line("ADD", value)
            out.line(value)
        for value in reversed(range(n)):
            inp.line("DELETE")
            out.line(value or -1)
    else:
        inp.line(2 + 4 * n)
        inp.line("ADD", 1)
        inp.line("SAVE", 1)
        out.line(1, 1)
        for value in range(2, n + 2):
            inp.line("LOAD", 1)
            inp.line("ADD", value)
            inp.line("SAVE", value)
            inp.line("DELETE")
            out.line(1, value, value, 1)


def sparse_table_cases(scenario, dims, inp, out):
    h, w, bh, bw, wh, ww = (dims[name] for name in ("h", "w", "black_h", "black_w", "white_h", "white_w"))
    inp.line(h, w, bh, bw, wh, ww)
    wh, ww = min(wh, bh), min(ww, bw)
    if scenario == "large_uniform_grid":
        row = " ".join(["1024"] * w)
        for _ in range(h):
            inp.line(row)
        out.line((bh * bw - wh * ww) * 1024)
    else:
        for y in range(h):
            inp.line(*range(y * w + 1, (y + 1) * w + 1))
        kept_width = bw - ww
        first = (h - bh) * w + w - bw + 1
        out.line(bh * kept_width * first + w * kept_width * bh * (bh - 1) // 2
                 + bh * kept_width * (kept_width - 1) // 2)


def segment_tree_cases(scenario, dims, inp, out):
    h, w, q = (dims[name] for name in ("h", "w", "q"))
    decreasing = scenario == "square_crossing_boundaries"
    inp.line(h, w, q)
    for y in range(h):
        start = y * w
        if decreasing:
            inp.line(*range(h * w - start, h * w - start - w, -1))
        else:
            inp.line(*range(start, start + w))
    for i in range(q):
        y1, y2 = sorted((i * 53 % h, (i + 1) * 29 % h))
        x1, x2 = sorted((i * 131 % w, (i + 1) * 65537 % w))
        inp.line(y1, x1, y2, x2)
        out.line(h * w - y2 * w - x2 if decreasing else y1 * w + x1)
    inp.line(0, 0, 0)



def range_fenwick_cases(scenario, dims, inp, out):
    n, rounds = dims["n"], dims["rounds"]
    inp.line(n, 2 * rounds)
    inp.line(*([5] * n))
    if scenario == "growing_prefixes":
        total = 5 * n
        for i in range(1, rounds + 1):
            inp.line(1, 1, i, 10000)
            total += 10000 * i
            inp.line(2, 1, n)
            out.line(total)
    else:
        for i in range(1, rounds + 1):
            pos = n * i // rounds
            inp.line(1, pos, pos, 10000)
            inp.line(2, pos, n)
            out.line(5 * (n - pos + 1) + 10000)


def xor_order_cases(scenario, dims, inp, out):
    if scenario == "complete_bit_cube":
        n, target = 1 << dims["bits"], dims["target"]
        k = (target - 1) * (n // 2) + n // 4 + 1
        inp.line(n, k)
        inp.line(*range(n))
        out.line(target)
    else:
        n = dims["n"]
        half = n // 2
        inp.line(n, half * (half - 1) + 1)
        inp.line(*((1 << 29) if i % 2 else 0 for i in range(n)))
        out.line(1 << 29)


def permutation_order_cases(scenario, dims, inp, out):
    n = dims["n"]
    inp.line(n)
    if scenario == "ascending":
        inp.line(*range(n))
        out.line(*([0] * n))
        out.line(*range(n))
    else:
        inp.line(*range(n - 1, -1, -1))
        out.line(*range(n - 1, -1, -1))
        out.line(*range(n - 1, -1, -1))

def component_rank_cases(scenario, dims, inp, out):
    n = dims["n"]
    inp.line(n, n)
    if scenario == "component_ranks_balanced_merges":
        stride = 1
        while stride < n:
            for first in range(1, n + 1, 2 * stride):
                if first + stride <= n:
                    inp.line(1, first, first + stride)
            stride *= 2
        inp.line(2, 1, 10)
        out.line(n - 9)
        return
    active = 1
    for i in range(n):
        if i % 2 == 0:
            active += 1
            inp.line(1, 1, active)
        else:
            rank = 1 + i // 2 % 10
            if i % 4 == 1:
                inp.line(2, 1, rank)
                out.line(active - rank + 1 if rank <= active else -1)
            else:
                inp.line(2, n, rank)
                out.line(n if rank == 1 else -1)


def meldable_heap_cases(scenario, dims, inp, out):
    n = dims["n"]
    inp.line(n)
    if scenario == "root_star_second_maximum":
        inp.line(*(10**9 - n + i for i in range(1, n)))
        for vertex in range(2, n + 1):
            inp.line(1, vertex)
        out.line(10**9 - 2)
        return
    path = dims["path"]
    inp.line(*(1 if vertex <= path else vertex - path + 1 for vertex in range(2, n + 1)))
    for vertex in range(2, path + 1):
        inp.line(vertex - 1, vertex)
    for vertex in range(path + 1, n + 1):
        inp.line(path, vertex)
    out.line(n - 2 * path + 1)


def persistent_weighted_cases(scenario, dims, inp, out):
    n, q, modulus = (dims[key] for key in ("n", "q", "k"))
    inp.line(n, modulus, q)
    if scenario == "wide_version_fanout":
        for i in range(q // 2):
            left = 1 + (i * 29) % n
            right = left + (i * 391) % (n - left + 1)
            cost = (i * 1000003) % modulus
            base = 2 * i + 1 if i % 5 else 0
            inp.line(0, 0, left, right, cost)
            inp.line(1, base, left, right)
            out.line("YES")
            out.line(cost if base else "UNKNOWN")
        return

    size = dims["size"]

    def prefix(x):
        return (x * 59323 + x * x * 9173) % modulus

    version = 0
    width = 1
    while width < size:
        for start in range(0, size, 2 * width):
            a, b = start + width - 1, start + 2 * width - 1
            inp.line(0, version, a + 1, b, (prefix(b) - prefix(a)) % modulus)
            out.line("YES")
            version += 1
        width *= 2
    for i in range(q - version):
        a = (i * 8743) % (size - 1)
        b = size - 1
        difference = (prefix(b) - prefix(a)) % modulus
        if i % 3 == 0:
            inp.line(0, version, a + 1, b, (difference + 1) % modulus)
            out.line("NO")
        elif i % 3 == 1:
            inp.line(1, version, a + 1, b)
            out.line(difference)
        else:
            inp.line(1, version - 1, 1, b)
            out.line("UNKNOWN")


def rollback_lazy_cases(scenario, dims, inp, out):
    n, m, groups, steps = (dims[key] for key in ("n", "m", "groups", "steps"))
    points_only = scenario == "million_point_writes_and_resets"
    lefts = [1 + (i * 39137) % n for i in range(m)]
    rights = lefts if points_only else [1 + (i * 57223 + n // 3) % n for i in range(m)]
    masks = [((i * 69721) ^ (i << 10)) & ((1 << 30)-1) for i in range(m)] if points_only else [i % 1019 for i in range(m)]
    points = lefts if points_only else [1 + (i * 94837) % n for i in range(m)]
    values = [i % 511 for i in range(n)] if points_only else [511] * n
    inp.line(n, m)
    inp.line(*values)
    for row in (lefts, rights, masks, points, points):
        inp.line(*row)
    inp.line(groups)

    def index(value):
        return min(n, max(1, value)) - 1

    for iteration in range(1, groups + 1):
        start = 1 + (iteration * 8933 + 5) % m
        inp.line(start, steps)
        y = iteration
        if points_only:
            changed = {}
            for step in range(1, steps + 1):
                z = (start + step) % m
                position = index(lefts[z] ^ y)
                value = changed.get(position, values[position])
                mask = masks[z] ^ y
                y = value | mask if z % 2 else value & mask
                changed[position] = y
        else:
            segments = [(0, n, 511)]
            for step in range(1, steps + 1):
                z = (start + step) % m
                left, right = sorted((index(lefts[z] ^ y), index(rights[z] ^ y)))
                right += 1
                position = index(points[z] ^ y)
                mask = masks[z] ^ y
                updated = []
                for lo, hi, value in segments:
                    if hi <= left or right <= lo:
                        updated.append((lo, hi, value))
                        continue
                    if lo < left:
                        updated.append((lo, left, value))
                    updated.append((max(lo, left), min(hi, right), value | mask if z % 2 else value & mask))
                    if right < hi:
                        updated.append((right, hi, value))
                segments = updated
                y = next(value for lo, hi, value in segments if lo <= position < hi)
        out.line(y)


def successor_set_cases(scenario, dims, inp, out):
    q = dims["q"]
    capacity = 1 << 20
    inp.line(q)
    if scenario == "cluster_wraps_around":
        for step in range(q - 1):
            inp.line(1, capacity - 1 + capacity * step)
        inp.line(2, q - 3)
        out.line(capacity - 1 + capacity * (q - 2))
    else:
        base = ((10**18 // capacity) - 1) * capacity
        for step in range(q // 4):
            position = step * 2
            first, second = base + position, base + position - capacity
            inp.line(1, first)
            inp.line(2, position)
            out.line(first)
            inp.line(1, second)
            inp.line(2, position + 1)
            out.line(second)
        for _ in range(q % 4):
            inp.line(2, capacity - 1)
            out.line(-1)


def weighted_minimum_cases(scenario, dims, inp, out):
    n, q = dims["n"], dims["q"]
    inp.line(n, q)
    if scenario == "same_row_column_events":
        for col in range(1, q + 1):
            inp.line(1, col, n, col)
        out.line(0)
    else:
        total = 0
        triangle = n * (n + 1) // 2
        for row in range(1, q + 1):
            end = 1 + ((row * 169453 + 137) % n)
            inp.line(row, 1, row, end)
            total += row * (triangle - end * (end + 1) // 2)
        out.line(total % 998244353)


def eytzinger_search_cases(scenario, dims, inp, out):
    n, q = dims["n"], dims["q"]
    inp.line(n, q)
    if scenario == "distinct_scrambled_thresholds":
        inp.line(*(2 * i + 1 for i in range(n - 1, -1, -1)))
        for i in range(q):
            value = 1 + ((i * 134917 + 44293) % (2 * n + 2))
            inp.line(value)
            out.line(n - min(n, value // 2))
    else:
        large = 10**9
        inp.line(*(large if i % 2 else 1 for i in range(n)))
        for i in range(q):
            value = (1, 2, large - 1, large)[i % 4]
            inp.line(value)
            out.line(n if value == 1 else n // 2)


def range_add_state_id_cases(scenario, dims, inp, out):
    n, q = dims["n"], dims["q"]
    inp.line(n, q)
    if scenario == "distinct_history":
        for step in range(q - 1):
            left = (step * 823127) % (n // 2)
            right = n - (step * 476897) % (n // 2)
            inp.line("!", left, right, 1)
        inp.line("?")
        out.line(q - 1)
    else:
        for step in range(q // 4):
            left = step
            right = n - (step * 673) % (n // 2)
            inp.line("!", left, right, 100)
            inp.line("?")
            out.line(step * 4 + 1)
            inp.line("!", left, right, -100)
            inp.line("?")
            out.line(0)
        for _ in range(q % 4):
            inp.line("?")
            out.line(0)


def range_add_gcd_cases(scenario, dims, inp, out):
    n, q = dims["n"], dims["q"]
    inp.line(n)
    if scenario == "moving_delta_boundaries":
        inp.line(*([18] * n))
        inp.line(q)
        for i in range(q // 5):
            left = 2 + (i * 1799) % (n // 3)
            right = n - 1 - (i * 1873) % (n // 3)
            inp.line(12, left, right)
            inp.line(0, left, right)
            out.line(30)
            inp.line(0, 1, n)
            out.line(6)
            inp.line(-12, left, right)
            inp.line(0, left, left)
            out.line(18)
        for _ in range(q % 5):
            inp.line(0, 1, n)
            out.line(18)
    else:
        base = 999999929
        inp.line(*(base + i % 2 for i in range(n)))
        inp.line(q)
        for i in range(q // 4):
            left = 1 + (i * 12341) % (n - 1)
            right = left + 1 + (i * 33013) % (n - left)
            inp.line(-111, 1, n)
            inp.line(0, left, right)
            out.line(1)
            inp.line(0, left, left)
            out.line(base + (left - 1) % 2 - 111)
            inp.line(111, 1, n)
        for _ in range(q % 4):
            inp.line(0, 1, n)
            out.line(1)


def disjoint_subarray_cases(scenario, dims, inp, out):
    n, q = dims["n"], dims["q"]
    inp.line(n, q)
    uniform = scenario == "all_positive_many_splits"
    positive, gap = 13, 10**9
    inp.line(*(positive if uniform or i % 2 == 0 else -gap for i in range(n)))
    positive_count = (n + 1) // 2
    for i in range(q):
        k = n if i == 0 else (i * 104729) % (n + 1)
        inp.line(k)
        if uniform:
            out.line(positive * n if k else 0)
        else:
            out.line(min(k, positive_count) * positive - max(0, k - positive_count) * gap)


def binary_range_cases(scenario, dims, inp, out):
    n, q = dims["n"], dims["q"]
    uniform = scenario == "unaligned_uniform_run"
    inp.line(n, q)
    inp.line("0" * n if uniform else ("10" * n)[:n])
    for step in range(q // 3):
        l = (step * 6827 + 3) % (n // 2)
        r = n - (step * 5851 + 5) % (n // 3)
        inp.line(1, l + 1, r)
        if uniform:
            a = (step * 27487 + 4) % (n // 2)
            b = n - (step * 22031 + 8) % (n // 3)
            inp.line(2, a + 1, b)
            out.line(max(0, min(r,b) - max(l,a)))
        else:
            inp.line(2, 1, n)
            out.line(2 if l % 2 == 1 or (r < n and r % 2 == 0) else 1)
        inp.line(1, l + 1, r)
    for _ in range(q % 3):
        inp.line(2, 1, n)
        out.line(0 if uniform else 1)


def delaunay_triangulation_reference_cases(scenario, dims, inp, out):
    if scenario == "rectangular_grid_cocircular":
        width, height = dims["width"], dims["height"]
        inp.line(width * height)
        for x in range(width - 1, -1, -1):
            for y in range(height):
                inp.line(x * 4_000_000 - 1_000_000_000, y * 4_000_000 - 1_000_000_000)
        out.line(3 * width * height + 1 - 2 * width - 2 * height)
    else:
        n, distinct = dims["n"], dims["distinct"]
        inp.line(n)
        for i in range(n):
            x = (i * (distinct - 1)) % distinct * (2_000_000_000 // (distinct - 1))
            inp.line(x, -x)
        out.line(distinct - 1)


def dfa_minimization_reference_cases(scenario, dims, inp, out):
    n = dims["n"]
    equivalent_pairs = scenario == "paired_equivalent_cycle_states"
    width = 2 if equivalent_pairs else 1
    inp.line(n, 2, n - 1, 60)
    for i in range(n):
        inp.line(int(i < width), (i + width) % n, i ^ int(equivalent_pairs))
    out.line(n // width)
    for q in range(60):
        a = q * 6329 % n
        b = (a ^ 1) if q % 3 == 0 else (a + q % 3 - 1) % n
        inp.line(a, b)
        out.line(int(a // width == b // width))


def labeled_tree_matching_count_cases(scenario, dims, inp, out):
    n, mod = dims["n"], 998244353
    fact = [1] * (n + 1)
    for i in range(2, n + 1):
        fact[i] = i * fact[i - 1] % mod
    inverse_fact = [1] * (n + 1)
    inverse_fact[n] = pow(fact[n], -1, mod)
    for i in range(n, 0, -1):
        inverse_fact[i - 1] = i * inverse_fact[i] % mod

    if scenario == "fixed_root_newton_doublings":
        inp.line(n, 1)
        rooted = [0] + [pow(i, i - 1, mod) * inverse_fact[i] % mod for i in range(1, n + 1)]
        losing, derivative = [0, 1], [0, 0]
        for i in range(2, n + 1):
            coefficient = sum(derivative[j] * losing[i - j] for j in range(1, i)) % mod
            value = coefficient * pow(i - 1, -1, mod) % mod
            losing.append(value)
            derivative.append((rooted[i] - value) * i % mod)
            out.line(value * fact[i - 1] % mod)
    else:
        inp.line(n, 2)
        for i in range(2, n + 1):
            perfect = 0
            if i % 2 == 0:
                m = i // 2
                contract = pow(m, max(m - 2, 0), mod)
                perfect = fact[i] * inverse_fact[m] * pow(2, m - 2, mod) * contract % mod
            out.line((pow(i, i - 2, mod) - perfect) % mod)


def trapezoid_count_cases(scenario, dims, inp, out):
    n = dims["n"]
    inp.line(n)

    def two(count):
        return count * (count - 1) // 2

    def equal_sums(size):
        for total in range(2 * size - 1):
            yield max(0, (total - 1) // 2 - max(0, total - size + 1) + 1)

    if scenario == "single_integer_parabola":
        for x in range(n):
            inp.line(x, x * x)
        out.line(sum(two(count) for count in equal_sums(n)))
    else:
        half, lift = n // 2, 4000000
        for i in range(half):
            inp.line(half + i, 5000000 + lift + i * i)
            inp.line(half - i, 5000000 - lift - i * i)
        # Same-half slopes are i+j. Cross-half slopes are larger and have an exact fraction key.
        cross = Counter()
        for i in range(half):
            for j in range(i, half):
                difference, sum_x = 2 * lift + i * i + j * j, i + j
                factor = gcd(difference, sum_x)
                cross[difference // factor, sum_x // factor] += 1 if i == j else 2
        # Exactly the half diameters share a midpoint, giving all the parallelograms.
        out.line(sum(two(2 * count) for count in equal_sums(half)) + sum(map(two, cross.values())) - two(half))


def todd_coxeter_reference_cases(scenario, dims, inp, out):
    period = dims["period"]
    collapse = scenario == "many_temporary_vertices_all_identified"
    alphabet = 4 if collapse else 1
    inp.line(alphabet, 1 + alphabet * collapse, period + alphabet + 1, 40)
    inp.line(period, *([0] * period))
    inp.line(0)
    if collapse:
        for g in range(alphabet):
            inp.line(1, g)
            inp.line(0)
    out.line(1 if collapse else period)
    for i in range(40):
        length = (7919 * i) % (31 if collapse else period)
        other = length + i % 3
        if not collapse and i % 4 == 1:
            other += period
        inp.line(length, *(j % alphabet for j in range(length)))
        inp.line(other, *(i % alphabet for _ in range(other)))
        out.line(int(collapse or (other - length) % period == 0))


def lexicographic_concatenation_second_cases(scenario, dims, inp, out):
    n, total = dims["n"], dims["total"]
    if scenario == "one_long_word_many_equal_short_words":
        inp.line(1)
        inp.line(n)
        inp.line("a" * (total - n) + "b")
        for _ in range(n - 1):
            inp.line("a")
        out.line("a" * (total - 1) + "b")
    else:
        tests = n // 2
        length, longer_cases = divmod(total, tests)
        inp.line(tests)
        for i in range(tests):
            wide = "b" + "c" * (length - 2 + (i < longer_cases))
            inp.line(2)
            inp.line("a")
            inp.line(wide)
            out.line(wide + "a")


def lexicographic_string_shuffle_cases(scenario, dims, inp, out):
    n, length = dims["n"], dims["length"]
    inp.line(n)
    if scenario == "all_equal_long_prefixes":
        word = "z" * (length - 1) + "a"
        for _ in range(n):
            inp.line(word)
        out.line(word * n)
    else:
        counts = Counter()
        for i in range(n):
            word = [chr(ord("a") + (i * 13 + j * 17) % 26) for j in range(length)]
            counts.update(word)
            inp.line("".join(sorted(word)))
        out.line("".join(letter * count for letter, count in sorted(counts.items())))


def bitmask_nfa_wildcard_cases(scenario, dims, inp, out):
    n, block = dims["n"], dims["block"]
    mod = 998244353
    inp.line("?" * n)
    if scenario == "ambiguous_word_boundaries":
        inp.line("?" * block)
        inp.line("?" * (block - 1))
        out.line(pow(2, n, mod))
    else:
        inp.line("?" * (block - 1) + "0")
        inp.line("?" * (block - 2) + "1?")
        free, forced_y = 1, 0
        for _ in range(n // block):
            free, forced_y = ((free + forced_y) << (block - 1)) % mod, (free << (block - 2)) % mod
        out.line((free + forced_y) % mod)


def regex_language_edit_distance_cases(scenario, dims, inp, out):
    n = dims["n"]
    if scenario == "fixed_mismatches":
        inp.line("a" * n)
        inp.line("b" * n)
        out.line(n)
    else:
        right_stars = n // 4 - 1
        c_count = n - 2 * right_stars
        inp.line("a*" * (n // 2 - 1) + "bc")
        inp.line("a*" * right_stars + "c" * c_count)
        out.line(c_count - 1)


def maximum_weight_window_cases(scenario, dims, inp, out):
    n = dims["n"]
    if scenario == "wide_window_negative_fences":
        side = 10**9
        inp.line(n, side)
        for i in range(n // 3):
            inp.line(-side, i, -10**9)
            inp.line(0, i, 10**9)
            inp.line(side, i, -10**9)
        for _ in range(n % 3):
            inp.line(0, 0, 0)
        out.line((n // 3) * 10**9)
    else:
        inp.line(n, 1)
        for i in range(n // 4):
            x = y = 4 * i
            inp.line(x, y, 800000000)
            inp.line(x + 1, y, 800000000)
            inp.line(x, y + 1, -400000000)
            inp.line(x + 1, y + 1, -400000000)
        out.line(1600000000)


def point_plane_distance_cases(scenario, dims, inp, out):
    n = dims["n"]
    if scenario == "unit_moment_curve":
        scale_x, scale_y, scale_z = 1, 1, 1
        parameters, p = range(1, n + 1), 0
    else:
        scale_x, scale_y, scale_z = 5000, 700, 200
        parameters = range(-(n // 2), n - n // 2)
        p = parameters.start - 1

    def write_point(t):
        positions = (scale_x * t, scale_y * t**2, scale_z * t**3)
        inp.line(*(f"{coordinate / 10**6:.6f}" for coordinate in positions))

    inp.line(n)
    write_point(p)
    for parameter in parameters:
        write_point(parameter)
    total = fsum(abs((p - a) * (p - b) * (p - c)) /
                 hypot((a * b + a * c + b * c) / scale_x, (a + b + c) / scale_y, 1 / scale_z)
                 for a, b, c in combinations(parameters, 3))
    out.line(f"{total / 10**6:.14g}")


def weighted_l1_statistics_cases(scenario, dims, inp, out):
    q = dims["q"]
    inp.line(q)
    for i in range(1, q // 2 + 1):
        if scenario == "distinct_increasing_coordinates":
            start = -10**9
            inp.line(1, start + i - 1, start)
            inp.line(2)
            out.line(start + (i - 1) // 2, i * i // 4 + i * start)
        else:
            value = -10**9 if i % 2 else 10**9
            inp.line(1, value, -17 if i % 2 else 17)
            inp.line(2)
            out.line(-10**9, 2 * 10**9 * (i // 2) - 17 * (i % 2))


def l1_origin_line_fit_cases(scenario, dims, inp, out):
    n = dims["n"]
    inp.line(n)
    if scenario == "oblique_line_with_unit_outliers":
        inp.line(*(3 * (i // 3 + 2) for i in range(n)))
        inp.line(*(2 * (i // 3 + 2) + (i % 3 == 2) for i in range(n)))
        out.line(n // 3, 1)
    else:
        minority = n // 4
        inp.line(*(10**6 if i < minority else 0 for i in range(n)))
        inp.line(*(0 if i < minority else (-1 if i % 2 else 1) * 10**6 for i in range(n)))
        out.line(minority * 10**6, 1)


def weighted_slope_trick_cases(scenario, dims, inp, out):
    n = dims["n"]
    inp.line(n, n)
    if scenario == "buyers_all_right_of_supply":
        first = 600000000
        inp.line(*range(first, first + n))
        inp.line(*range(n, 0, -1))
        buyer_sum = n * first + n * (n - 1) // 2
        for capacity in range(1, n + 1):
            full, remaining = divmod(n, capacity)
            supply_sum = capacity * full * (2 * n - full + 1) // 2 + remaining * (n - full)
            out.line(buyer_sum - supply_sum)
    else:
        inp.line(*range(2, 2 * n + 1, 2))
        inp.line(*range(1, 2 * n, 2))
        for _ in range(n):
            out.line(n)


def narrow_grid_domino_cases(scenario, dims, inp, out):
    width = dims["width"]
    modulus = 1000000007
    # Enumerate the 27 possible pairs of disjoint masks and all vertical domino masks.
    # The resulting sequence satisfies these coefficients for 27 consecutive indices,
    # so Cayley–Hamilton for that transfer matrix gives the identity for all widths.
    initial = [1, 2, 5, 22, 75, 264, 941, 3286, 11623, 40960, 144267, 508812, 1792981, 6319994, 22277291]
    recurrence = [1, 5, 11, 5, 14, 8, 3, 0, -5, -11, -1, 2, 0, 0, 1]
    order = len(initial)

    def multiply(left, right):
        product = [0] * (2 * order - 1)
        for i, a in enumerate(left):
            for j, b in enumerate(right):
                product[i + j] += a * b
        for degree in range(len(product) - 1, order - 1, -1):
            high = product[degree] % modulus
            for i, coefficient in enumerate(recurrence):
                product[degree - i - 1] += coefficient * high
        return [value % modulus for value in product[:order]]

    result, power = [1] + [0] * (order - 1), [0, 1] + [0] * (order - 2)
    remaining = width
    while remaining:
        if remaining & 1:
            result = multiply(result, power)
        power = multiply(power, power)
        remaining >>= 1
    inp.line(width)
    out.line(sum(a * b for a, b in zip(result, initial)) % modulus)


def grid_corner_coloring_cases(scenario, dims, inp, out):
    height, width = dims["height"], dims["width"]
    modulus = 1000000007
    inp.line(height, width)
    if scenario == "long_ladder_source_choices":
        top, bottom, both = 1, 0, 1
        for _ in range(width - 1):
            top, bottom, both = ((top + both) % modulus, (bottom + both) % modulus,
                                 (top + bottom + both) % modulus)
        out.line((bottom + both) % modulus)
        return

    @cache
    def transitions(previous, first):
        counts = Counter()
        for mask in range(1, 1 << height):
            rows = [row for row in range(height) if (mask >> row) & 1]
            if first and rows[0] != 0:
                continue
            parent = list(range(height))

            def root(row):
                while parent[row] != row:
                    row = parent[row]
                return row

            def join(u, v):
                u, v = sorted((root(u), root(v)))
                parent[v] = u

            connections = {}
            for row in rows:
                if row > 0 and (mask >> (row - 1)) & 1:
                    join(row - 1, row)
                group = previous[row]
                if group:
                    join(connections.setdefault(group, row), row)
            if not first and -1 not in connections:
                continue
            marked = root(0 if first else connections[-1])
            groups = [0] * height
            for row in rows:
                representative = root(row)
                groups[row] = -1 if representative == marked else representative + 1
            counts[tuple(groups)] += 1
        return tuple(counts.items())

    states = {(0,) * height: 1}
    for column in range(width):
        following = {}
        for state, count in states.items():
            for successor, multiplicity in transitions(state, column == 0):
                following[successor] = (following.get(successor, 0) + count * multiplicity) % modulus
        states = following
    out.line(sum(count for state, count in states.items() if state[-1] == -1) % modulus)


def tournament_path_cases(scenario, dims, inp, out):
    n = dims["n"]

    def interleave_halves(left, right):
        if right - left < 2:
            return list(range(left, right))
        middle = (left + right) // 2
        lower = interleave_halves(left, middle)
        upper = interleave_halves(middle, right)
        order = []
        for i, vertex in enumerate(upper):
            order.append(vertex)
            if i < len(lower):
                order.append(lower[i])
        return order

    order = (interleave_halves(0, n) if scenario == "maximum_merge_queries"
             else list(reversed(range(n))))
    ranks = [0] * n
    for i, vertex in enumerate(order):
        ranks[vertex] = i
    inp.line(n)
    for rank in ranks:
        inp.line("".join("1" if rank < other else "0" for other in ranks))
    for vertex in order:
        out.line(vertex)


_GENERATORS = {
    "delaunay_triangulation_reference": delaunay_triangulation_reference_cases,
    "dfa_minimization_reference": dfa_minimization_reference_cases,
    "labeled_tree_matching_counts": labeled_tree_matching_count_cases,
    "trapezoid_count": trapezoid_count_cases,
    "todd_coxeter_reference": todd_coxeter_reference_cases,
    "lexicographic_concatenation_second": lexicographic_concatenation_second_cases,
    "lexicographic_string_shuffle": lexicographic_string_shuffle_cases,
    "bitmask_nfa_wildcard": bitmask_nfa_wildcard_cases,
    "regex_language_edit_distance": regex_language_edit_distance_cases,
    "maximum_weight_window": maximum_weight_window_cases,
    "point_plane_distance_sum": point_plane_distance_cases,
    "weighted_l1_statistics_absolute_minima": weighted_l1_statistics_cases,
    "l1_origin_line_fit": l1_origin_line_fit_cases,
    "weighted_slope_trick_selling_cars": weighted_slope_trick_cases,
    "narrow_grid_domino_packings": narrow_grid_domino_cases,
    "grid_corner_connected_colorings": grid_corner_coloring_cases,
    "tournament_path_reference": tournament_path_cases,
    "successor_set_linear_probing": successor_set_cases,
    "range_add_weighted_minimum": weighted_minimum_cases,
    "eytzinger_search": eytzinger_search_cases,
    "range_add_state_ids": range_add_state_id_cases,
    "range_add_range_gcd": range_add_gcd_cases,
    "disjoint_subarray_max_sum": disjoint_subarray_cases,
    "permutation_order_roundtrip": permutation_order_cases,
    "pair_xor_order": xor_order_cases,
    "binary_range_statistics": binary_range_cases,
    "interval_set_range_mex": interval_cases,
    "monotone_line_container_frog": monotone_line_cases,
    "meldable_heap_tree_game": meldable_heap_cases,
    "offline_li_chao_tree_coins": offline_li_chao_cases,
    "top_k_sum_sliding": top_k_cases,
    "persistent_stack_notebook": stack_cases,
    "persistent_weighted_union_find_branching": persistent_weighted_cases,
    "rollback_lazy_segment_tree": rollback_lazy_cases,
    "sparse_table_2d_stamps": sparse_table_cases,
    "segment_tree_2d_rectangle_minimum": segment_tree_cases,
    "range_fenwick_tree": range_fenwick_cases,
    "union_find_order_statistics_rank": component_rank_cases,
}


def generate(plan, inp, out):
    _GENERATORS[plan["verify_id"]](plan["scenario"], plan["dimensions"], inp, out)
