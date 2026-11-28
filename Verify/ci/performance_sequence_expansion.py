# SPDX-License-Identifier: CC0-1.0
"""Whole-program cases for non-Library Checker sequence and string libraries."""
from __future__ import annotations

LIBRARY_VERIFY_IDS = {
    "Algorithm/Adjacent_Swap_Distance.cpp": "adjacent_swap_distance",
    "Algorithm/Cyclic_Inversion_Statistics.cpp": "cyclic_rotation_inversions_adjacent_swaps",
    "Algorithm/Bounded_Zigzag_Subsequence.cpp": "bounded_zigzag_subsequence",
    "Algorithm/Coordinate_Compression.cpp": "coordinate_compression",
    "Algorithm/Interval_LIS.cpp": "interval_lis",
    "Algorithm/Interval_Scheduling.cpp": "interval_schedule_max_weight",
    "Algorithm/LIS_Statistics.cpp": "lis_count",
    "Algorithm/Modular_Pair_Sum_Order.cpp": "modular_pair_sum_order",
    "Algorithm/Monge_Shortest_Paths.cpp": "monge_shortest_paths_baseball",
    "Algorithm/Pair_Product_Order.cpp": "pair_product_order",
    "Algorithm/Pair_Sum_Order.cpp": "pair_sum_order_handshake",
    "Algorithm/Parallel_Binary_Search.cpp": "parallel_binary_search_stamp_rally",
    "Algorithm/Range_Mex.cpp": "subarray_mex_histogram",
    "Algorithm/Rollback_LIS.cpp": "rollback_lis_tree",
    "Algorithm/State_Cycle.cpp": "state_cycle_sequence_sum",
    "Algorithm/Subarray_Median_Order.cpp": "subarray_median_order",
    "Algorithm/Subset_Sum_Meet_In_Middle.cpp": "subset_sum_meet_in_middle_max",
    "Algorithm/Weighted_Monotone_Matching.cpp": "weighted_monotone_matching_flowers",
    "DataStructure/Interval_Map.cpp": "interval_map_paint",
    "DataStructure/Range_Add_Threshold_Min.cpp": "range_add_threshold_min",
    "DataStructure/Range_Equal_Runs.cpp": "range_equal_runs",
    "Algorithm/Lexicographic_Subsequence.cpp": "lexicographic_subsequence",
    "DataStructure/Range_Nondecreasing_Subsequence.cpp": "range_nondecreasing_subsequence",
    "Algorithm/Forbidden_Sum_Partitions.cpp": "forbidden_sum_partitions",
    "Geometry/Lattice_Shape_Transforms.cpp": "lattice_shape_transforms",
    "String/Cyclic_Xor_Matches.cpp": "cyclic_xor_matches",
    "DataStructure/Ordered_Multiset.cpp": "ordered_multiset_extrema",
    "Geometry/Chord_Crossing_Queries.cpp": "chord_crossing_queries",
    "Algorithm/Dominance_Counts.cpp": "dominance_counts",
    "Algorithm/Sparse_Obstacle_Grid_Paths.cpp": "sparse_obstacle_grid_paths",
    "Algorithm/Target_Sum_Subrectangles.cpp": "target_sum_subrectangles",
    "Algorithm/Knuth_Interval_DP.cpp": "knuth_interval_dp",
    "Algorithm/Binary_Grid_Squares.cpp": "binary_grid_squares",
    "DataStructure/Run_Length_Queue.cpp": "run_length_queue",
    "String/Repeated_Subsequence.cpp": "repeated_subsequence",
    "String/Balanced_Pivot_Subsequence.cpp": "balanced_pivot_subsequence",
    "String/Three_Symbol_Frequency_Counts.cpp": "three_symbol_frequency_counts",
    "String/Pattern_Range_Occurrences.cpp": "pattern_range_occurrences",
    "Algorithm/Exact_Subarray_Extrema.cpp": "exact_subarray_extrema",
    "Algorithm/Longest_Bitonic_Subsequence.cpp": "longest_bitonic_subsequence",
    "Math/Poisson_Binomial.cpp": "poisson_binomial",
    "DataStructure/Range_Add_Total_Variation.cpp": "range_add_total_variation",
    "Algorithm/Inversion_Limited_Partitions.cpp": "inversion_limited_partitions",
    "String/Dictionary_Segmenter.cpp": "dictionary_segmenter",
    "Algorithm/Deadline_Job_Selection.cpp": "deadline_job_selection",
    "String/Periodic_Pattern_Runs.cpp": "periodic_pattern_runs",
    "String/Pattern_Stamping.cpp": "pattern_stamping",
    "Algorithm/Subarray_Frequency_Windows.cpp": "subarray_frequency_windows_distinct",
    "Algorithm/Subarray_Sum_Counts.cpp": "subarray_sum_counts_equal",
    "Algorithm/Clamped_Translation.cpp": "clamped_translation",
    "Algorithm/Weighted_Interval_Stabbing.cpp": "weighted_interval_stabbing",
    "Algorithm/Maximum_Allowed_Pair.cpp": "maximum_allowed_pair",
    "DataStructure/Monotone_Value_Shift.cpp": "monotone_value_shift",
    "DataStructure/Nearest_Value_Index.cpp": "nearest_value_index",
    "DataStructure/Point_Set_Mex.cpp": "point_set_mex",
    "DataStructure/Sliding_Window_Extrema.cpp": "sliding_window_extrema",
    "String/Bracket_Sequence.cpp": "bracket_sequence_range",
    "String/Concatenated_Pattern_Count.cpp": "concatenated_pattern_count",
    "String/Cyclic_Rotation.cpp": "cyclic_rotation",
    "String/Cyclic_Rotation_Order.cpp": "cyclic_rotation_pairs",
    "String/Distinct_Subsequence_Order.cpp": "distinct_subsequence_order",
    "String/Dynamic_Palindrome.cpp": "dynamic_palindrome",
    "String/Dynamic_Prefix_Filter.cpp": "dynamic_prefix_filter",
    "String/Dynamic_Substring_Matching.cpp": "dynamic_substring_matching",
    "String/Edit_Alignment.cpp": "edit_alignment_sequence_matching",
    "String/Grid_Pattern_Matching.cpp": "grid_pattern_matching_all",
    "String/Hamming_Distance_Matching.cpp": "hamming_distance_matching_binary_min",
    "String/Offline_Prefix_Lcp_Sum.cpp": "offline_prefix_lcp_sum",
    "String/Pattern_Avoidance.cpp": "pattern_avoidance",
    "String/Pattern_Erasure.cpp": "pattern_erasure",
    "String/Prefix_Bitmap_Table.cpp": "prefix_bitmap_table",
    "String/Run_Length_Z.cpp": "run_length_z_sum",
    "String/Shortest_Absent_Subsequence.cpp": "shortest_absent_subsequence",
    "String/Shortest_Common_Superstring.cpp": "shortest_common_superstring",
    "String/Sorted_Substring_Index.cpp": "sorted_substring_index",
    "String/String_Periods.cpp": "string_periods_best_representation",
    "String/String_Radix_Order.cpp": "string_radix_order_alphabet",
    "String/Substring_Comparison.cpp": "substring_comparison_common_prefix",
    "String/Suffix_Tree.cpp": "suffix_tree_nonoverlapping_repeat",
}

PROFILES = {
    "coordinate_compression": (
        ("same_column_decreasing_rows", {"n": 100000}, {"n": 40},
         "Every point has column rank one, while decreasing input rows reverse their ranks."),
        ("shuffled_grid_sparse_coordinates", {"n": 100000}, {"n": 40},
         "The shuffled full grid has one-based row i/10+1 and column i%10+1 for its index i.")),
    "point_set_mex": (
        ("long_distance_mex_jumps", {"n": 200000}, {"n": 40},
         "Temporarily remove one entry of 0..n-1, making it the mex; restore it to return the mex to n."),
        ("fill_all_missing_values", {"n": 200000}, {"n": 51},
         "Setting every position to its index in order makes the mex advance by one after each update.")),
    "sliding_window_extrema": (
        ("increasing_full_ring", {"n": 1000000, "width": 500001}, {"n": 75, "width": 41},
         "The strictly increasing array makes every window start the minimum and keeps the queue full."),
        ("decreasing_plateaus", {"n": 1000000, "width": 333333}, {"n": 80, "width": 33},
         "For a nonincreasing array every window ends at its minimum; equal blocks precede decreases.")),
    "pair_sum_order_handshake": (
        ("large_threshold_tie", {"n": 100000}, {"n": 17},
         "All ordered pairs have the same value, with a near-maximum rank cutting the tie block."),
        ("increasing_upper_triangle", {"n": 100000}, {"n": 19},
         "Pairs from 1..N with sums at least N+1 form an exact triangle; closed-form row sums give the answer.")),
    "string_radix_order_alphabet": (
        ("maximum_width_shared_prefix", {"n": 50000}, {"n": 56},
         "Reverse the distinct length-ten strings sharing six initial letters; the fixed-width suffix encodes their custom order."),
        ("shuffled_prefix_relationships", {"n": 50000}, {"n": 39},
         "Bijective radix digits form a prefix tree; preorder traversal in the custom alphabet gives the sorted words.")),
    "dynamic_prefix_filter": (
        ("maximum_length_chain", {"length": 499998}, {"length": 77},
         "A nearly maximum-length word is inserted, excluded by its first byte, and followed by one allowed word."),
        ("maximum_query_count_with_duplicates", {"q": 200000}, {"q": 48},
         "Half the prior words disappear when b is blocked. Subsequent a words increase the count, b words leave it unchanged.")),
    "prefix_bitmap_table": (
        ("maximum_width_every_row_matches", {"n": 1000, "width": 3000, "q": 4000}, {"n": 75, "width": 40, "q": 24},
         "All rows are identical, and every full-width query matches every row."),
        ("alternating_replacements_at_last_column", {"n": 1000, "width": 3000, "q": 4000}, {"n": 65, "width": 31, "q": 24},
         "Alternate the common last byte between a and b. The full-width prefix ending in b alternates between N and zero.")),
    "interval_map_paint": (
        ("growing_prefix_run", {"n": 500000, "q": 200000}, {"n": 43, "q": 36},
         "Repainting the leftmost run with its next neighbor's color adds exactly one cell to the run."),
        ("isolated_recolors_without_merges", {"n": 500000, "q": 200000}, {"n": 45, "q": 34},
         "Recolor nonadjacent singletons to color one. Its total count grows by one while the number of runs stays fixed.")),
    "subarray_median_order": (
        ("descending_distinct_median_positions", {"n": 100000}, {"n": 23},
         "After reversing, each subarray's upper median index is floor((left+right+1)/2); count endpoints by their sum."),
        ("alternating_even_width_ties", {"n": 100000}, {"n": 26},
         "Even windows always choose the high value. Odd windows split evenly by starting parity, so the high value wins.")),
    "range_equal_runs": (
        ("alternating_middle_split", {"n": 200000, "q": 200000}, {"n": 42, "q": 36},
         "Alternately split one full-length run in the middle and merge it again; query the whole array each time."),
        ("adjacent_equal_pairs", {"n": 200000, "q": 200000}, {"n": 36, "q": 30},
         "Create one equal adjacent pair per point update and query exactly its two elements.")),
    "lexicographic_subsequence": (
        ("long_suffix_replaces_prefix", {"n": 100000, "k": 50000}, {"n": 14, "k": 7},
         "The first minimum byte discards the whole saved prefix. Every minimum byte is then selected."),
        ("select_all_sawtooth", {"n": 100000, "k": 100000}, {"n": 13, "k": 13},
         "Select every position despite frequent descending pairs that cannot remove any candidate.")),
    "range_nondecreasing_subsequence": (
        ("descending_alphabet_cycles", {"n": 200000, "q": 200000}, {"n": 32, "q": 38},
         "Full-array queries on descending ABCD blocks. Changing the first D to A allows one extra retained letter."),
        ("nonaligned_two_symbol_boundaries", {"n": 200000, "q": 200000}, {"n": 37, "q": 40},
         "Move the boundary between D and A by one and vary both query endpoints; the smaller block is replaced.")),
    "cyclic_rotation_pairs": (
        ("equal_classes_low_period", {"n": 200000}, {"n": 18},
         "Two phase shifts of the same period-two string. One class is smaller and both classes have equal multiplicity."),
        ("long_common_prefix_distinct_markers", {"n": 200000}, {"n": 17},
         "Two single-marker strings. A rotation with more leading a's is smaller; z loses against y at equal prefix lengths.")),
    "cyclic_rotation_inversions": (
        ("ascending_max_permutation", {"n": 300000}, {"n": 27},
         "Each left shift by k of an ascending permutation introduces exactly k*(N-k) inversions."),
        ("descending_max_permutation", {"n": 300000}, {"n": 25},
         "Every inversion of the ascending counterpart becomes a non-inversion and vice versa.")),
    "cyclic_rotation_inversions_adjacent_swaps": (
        ("toggle_first_pair", {"n": 200000, "q": 200000, "high": 1}, {"n": 8, "q": 14, "high": 1},
         "Every swap toggles the first pair of an ascending permutation. Compute the sum for one reversal operation."),
        ("swap_disjoint_pairs_large_powers", {"n": 200000, "q": 200000, "high": 1000000000}, {"n": 10, "q": 20, "high": 4},
         "Swap every adjacent disjoint pair and restore it on the next pass, alternating even and odd operation counts.")),
    "range_add_threshold_min": (
        ("uniform_keys_wide_alternating_adds", {"n": 100000, "k": 33333, "q": 50000}, {"n": 43, "k": 15, "q": 40},
         "With parking everywhere, alternate one rent between the endpoints of the value constraint."),
        ("varying_threshold_and_lazy_offsets", {"n": 100000, "k": 21431, "q": 50000}, {"n": 67, "k": 17, "q": 44},
         "Parking is a prefix. Thresholds cross both the feasible set of starts and the range affected by a cheap rent.")),
    "bounded_zigzag_subsequence": (
        ("wide_unique_alternating", {"n": 200000}, {"n": 30},
         "Distinct heights alternate between rising low values and falling high values; every item can be retained."),
        ("no_allowed_pair_sparse_coordinates", {"n": 200000}, {"n": 23},
         "Permuted distinct odd heights provide no pair with difference one; only one positive score can be earned.")),
    "pattern_erasure": (
        ("nested_cascading_erasure", {"n": 200000}, {"n": 50},
         "Each BC removes the last remaining A from a large prefix. Only the final B bytes survive."),
        ("repeated_near_miss_no_erasure", {"n": 200000}, {"n": 48},
         "Every AB matches two of three pattern bytes, and the next A causes failure without erasure.")),
    "pattern_avoidance": (
        ("all_126_forbidden_words", {"length": 10**18, "m": 126}, {"length": 8, "m": 126},
         "Every binary word up to length six is forbidden. The two singleton bans remove every transition."),
        ("length_six_bans_maximum_exponent", {"length": 10**18, "m": 32}, {"length": 9, "m": 32},
         "Forbid exactly the six-byte words ending in a. The first five letters are free, then all must be b.")),
    "sorted_substring_index": (
        ("alternating_middle_descent", {"n": 100000, "q": 100000}, {"n": 43, "q": 36},
         "Alternate one internal m with z, creating and removing a descent. Query the whole string each time."),
        ("interior_letter_outside_sorted_slice", {"n": 100000, "q": 100000}, {"n": 47, "q": 42},
         "Keep the queried suffix sorted and move the only y outside it; every other query fails the inclusion test.")),
    "forbidden_sum_partitions": (
        ("distinct_prefixes_fibonacci_partitions", {"n": 200000}, {"n": 24},
         "All values are one, so prefixes are distinct and allowed blocks have at least two entries."),
        ("constant_prefixes_no_partition", {"n": 200000}, {"n": 21},
         "All positions reuse the same prefix; every nonempty block has the forbidden sum.")),
    "lattice_shape_transforms": (
        ("dense_asymmetric_quarter_turn", {"n": 200}, {"n": 9},
         "Nearly the full grid is occupied; an asymmetric set of missing cells rotates to the target."),
        ("dense_incompatible_hole_separation", {"n": 200}, {"n": 8},
         "Two dense shapes have identical point counts and extents, but their two holes are at different distances.")),
    "cyclic_xor_matches": (
        ("all_rotations_uniform", {"n": 200000}, {"n": 23},
         "Uniform inputs are equal after the same XOR for every rotation, maximizing output size."),
        ("single_outlier_unique_rotation", {"n": 200000}, {"n": 17},
         "The neighboring differences are mostly equal and include a unique cyclic position for the outlier.")),
    "ordered_multiset_extrema": (
        ("increasing_insert_then_erase_max", {"q": 200000}, {"q": 38},
         "Sorted distinct insertions and successive maximum deletions exercise AVL rotations and changing extrema."),
        ("duplicates_partial_and_absent_erasure", {"q": 200000}, {"q": 28},
         "Many equivalent insertions share one node; skips of absent keys and the final deletion are observable.")),
    "chord_crossing_queries": (
        ("every_chord_crosses_swept_arcs", {"n": 1000000, "m": 200000, "q": 200000}, {"n": 24, "m": 10, "q": 17},
         "Every query crosses every registered chord as the open arc advances; sweep events still change."),
        ("nested_or_single_boundary", {"n": 1000000, "m": 200000, "q": 200000}, {"n": 27, "m": 11, "q": 19},
         "Disjoint short chords are fully contained in large arcs, or exactly one crosses at a narrow boundary.")),
    "dominance_counts": (
        ("every_pair_equal", {"n": 200000}, {"n": 42},
         "A full tie group tests non-strict comparison and gives N squared matching assignments beyond 32-bit."),
        ("permuted_anticorrelated_ranks", {"n": 200000}, {"n": 41},
         "Distinct permuted coordinates give one total dominance order and exercise every compressed rank.")),
    "sparse_obstacle_grid_paths": (
        ("incomparable_diagonal_barrier", {"h": 100000, "w": 100000, "k": 3000}, {"h": 13, "w": 14, "k": 9},
         "A complete antidiagonal separates the endpoints, with pairwise incomparable obstacle positions."),
        ("single_row_dominance_chain", {"h": 100000, "w": 100000, "k": 3000}, {"h": 11, "w": 23, "k": 17},
         "All walls dominate one another on the second row, and every surviving path passes a known gate.")),
    "target_sum_subrectangles": (
        ("distinct_band_prefixes", {"h": 547, "w": 548, "tail": 244}, {"h": 9, "w": 8, "tail": 5},
         "The nearly square all-positive grid forces each band prefix to be a distinct hash key, with no matches."),
        ("checkerboard_dense_matches", {"h": 547, "w": 548, "tail": 244}, {"h": 8, "w": 11, "tail": 3},
         "A checkerboard reuses almost all band prefixes and counts every rectangle with an even side, exceeding 32 bits.")),
    "knuth_interval_dp": (
        ("equal_billion_weights", {"n": 3000}, {"n": 23},
         "All circular cuts tie; the balanced binary tree leaf depths give the exact 64-bit total."),
        ("one_dominant_tablet", {"n": 3000}, {"n": 17},
         "A single dominant tablet must merge last, leaving the uniform contiguous remainder to be optimally merged.")),
    "binary_grid_squares": (
        ("all_open_maximum_count", {"h": 3000, "w": 3000, "holes": 0}, {"h": 9, "w": 14, "holes": 0},
         "Every square is valid on a maximum grid; the total exceeds signed and unsigned 32-bit ranges."),
        ("maximum_holes_then_open", {"h": 3000, "w": 3000, "holes": 100000}, {"h": 11, "w": 13, "holes": 32},
         "A maximum number of obstacles fills a row-major prefix, then a large open grid restarts the recurrence.")),
    "run_length_queue": (
        ("merged_billion_counts", {"q": 200000, "count": 10**9}, {"q": 36, "count": 11},
         "Half the operations append identical billion-copy runs and the rest remove each count, with 64-bit sums."),
        ("one_pop_many_runs", {"q": 200000, "count": 1}, {"q": 33, "count": 1},
         "Every append creates a distinct run, then one removal consumes all of them in order.")),
    "repeated_subsequence": (
        ("one_letter_full_capacity", {"n": 100000, "m": 100000, "copies": 10**12},
         {"n": 21, "m": 18, "copies": 8},
         "Maximum-length all-equal strings consume the available copies exactly and stress a single large occurrence list."),
        ("large_interleaved_wraps", {"n": 100000, "m": 100000, "copies": 10**12},
         {"n": 24, "m": 18, "copies": 7},
         "Two alternating symbols force large jumps and wraps in every target group, with an exact capacity on ab pairs.")),
    "balanced_pivot_subsequence": (
        ("alternating_many_interior_crossings", {"n": 100000, "q": 100000}, {"n": 45, "q": 32},
         "Long aligned slices of 1/2 place the optimum near the middle of many separator candidates."),
        ("separator_plateau_and_empty_slices", {"n": 100000, "q": 100000}, {"n": 36, "q": 28},
         "An entire middle block ties for each long query; short ranges can contain just a separator or none.")),
    "three_symbol_frequency_counts": (
        ("single_symbol_wide_prefix_map", {"n": 200000}, {"n": 23},
         "Every prefix difference is new, but two unused symbols always have equal counts."),
        ("two_long_runs_large_answer", {"n": 200000}, {"n": 25},
         "Count every interval crossing the run boundary, except equal-sized contributions from both sides.")),
    "pattern_range_occurrences": (
        ("dense_overlapping_matches", {"n": 400000, "q": 200000, "m": 10}, {"n": 35, "q": 19, "m": 10},
         "All starts match; query lengths alternate exactly at and just below the pattern size."),
        ("single_suffix_match", {"n": 400000, "q": 200000, "m": 10}, {"n": 29, "q": 17, "m": 10},
         "Only the final position matches, so excluding the last character changes the response.")),
    "exact_subarray_extrema": (
        ("equal_extrema_wide_count", {"n": 200000}, {"n": 19},
         "An identical minimum and maximum lets every nonempty interval qualify."),
        ("alternating_bounds_and_blockers", {"n": 200000}, {"n": 23},
         "An out-of-range separator ends each block with exactly one qualifying interval.")),
    "longest_bitonic_subsequence": (
        ("full_mountain_witness", {"n": 300000}, {"n": 18},
         "The whole input rises to one peak and falls, so the chosen witness spans every position."),
        ("all_equal_one_element", {"n": 300000}, {"n": 23},
         "Every value ties, so a strict result contains exactly one index.")),
    "poisson_binomial": (
        ("fair_coins_with_symmetric_tail", {"n": 2999}, {"n": 27},
         "Odd fair trials split their outcomes equally across the majority threshold."),
        ("tiny_majority_tail", {"n": 2999}, {"n": 33},
         "The majority probability is below the displayed precision; evaluation accumulates it directly.")),
    "range_add_total_variation": (
        ("repeated_interior_boundaries", {"n": 100000, "q": 100000}, {"n": 13, "q": 11},
         "The two boundaries of a long interior range grow past 32 bits."),
        ("global_shift_and_edge_updates", {"n": 100000, "q": 100000}, {"n": 12, "q": 14},
         "Whole-array changes alternate with canceling updates to one endpoint over a large initial variation.")),
    "inversion_limited_partitions": (
        ("rising_all_cuts_valid", {"n": 200000}, {"n": 13},
         "All strictly increasing intervals have zero inversions, so each cut is independent."),
        ("falling_one_inversion_limit", {"n": 200000}, {"n": 12},
         "Only blocks of length one or two are valid, forcing continual evictions and Fibonacci counts.")),
    "dictionary_segmenter": (
        ("ambiguous_prefixes_with_valid_split", {"n": 100000}, {"n": 65},
         "Repeat a block with two competing dictionary prefixes and retain a complete segmentation."),
        ("unsegmentable_final_byte", {"n": 100000}, {"n": 75},
         "An otherwise valid chain finishes with a byte missing from the dictionary.")),
    "deadline_job_selection": (
        ("dense_time_states", {"n": 5000}, {"n": 10},
         "All one-unit jobs fit, creating dense finish-time states and a reward beyond 32 bits."),
        ("scattered_tight_deadlines", {"n": 5000}, {"n": 12},
         "Interleaved long jobs would displace higher-value short jobs; the optimal schedule takes the latter.")),
    "periodic_pattern_runs": (
        ("coprime_unbounded_run", {"n": 500000, "m": 499999}, {"n": 17, "m": 16},
         "The long homogeneous pattern matches at every residue, forming one cyclic walk through the full period."),
        ("finite_chain_cut_by_one_byte", {"n": 500000, "m": 1}, {"n": 21, "m": 1},
         "A single different byte cuts one long run, so the maximum is the period length minus one.")),
    "pattern_stamping": (
        ("long_overwrite_chain", {"n": 200000}, {"n": 11},
         "The only final exact stamp unlocks reverse peeling one position at a time."),
        ("last_remaining_mismatch", {"n": 200000}, {"n": 12},
         "The same long chain removes every suffix position until one leading byte absent from the stamp remains.")),
    "subarray_frequency_windows_distinct": (
        ("all_distinct_wide_map", {"n": 100000, "k": 100000}, {"n": 47, "k": 47},
         "Every value is unique and all values fit, so the map grows to the full distinct population."),
        ("cyclic_erase_and_insert", {"n": 100000, "k": 32}, {"n": 71, "k": 9},
         "Every window of length K+1 contains K+1 types, repeatedly evicting the oldest type.")),
    "subarray_sum_counts_equal": (
        ("positive_large_subarray_sums", {"n": 200000}, {"n": 28},
         "Uniform positives give a closed form for exact sums and exercise the linear path."),
        ("signed_distinct_prefixes", {"n": 200000}, {"n": 27},
         "Opposite large entries sum to one per pair and produce distinct alternating prefix keys.")),
    "clamped_translation": (
        ("wide_unbounded_additions", {"n": 200000, "q": 200000}, {"n": 35, "q": 32},
         "Large additions produce a shift wider than 32 bits without either bound."),
        ("bounded_canceling_additions", {"n": 200000, "q": 200000}, {"n": 34, "q": 29},
         "Initial bounds survive pairs of canceling large shifts. Inputs exercise both caps and the middle.")),
    "weighted_interval_stabbing": (
        ("positive_suffixes_same_end", {"n": 200000}, {"n": 11},
         "All long overlapping suffix intervals end together; choosing the last point earns every weight."),
        ("alternating_signed_singletons", {"n": 200000}, {"n": 12},
         "Every position ends one interval. The optimal witness includes only the positive singleton positions.")),
    "maximum_allowed_pair": (
        ("dense_high_score_block", {"n": 100000, "m": 100000, "p": 100000}, {"n": 21, "m": 19, "p": 106},
         "Square-shaped exclusions cover the intersection of both high-score prefixes, forcing long candidate scans."),
        ("wholly_excluded_top_row", {"n": 100000, "m": 100000, "p": 100000}, {"n": 17, "m": 11, "p": 11},
         "One row has every candidate forbidden, forcing full column ordering before the next highest row can succeed.")),
    "monotone_value_shift": (
        ("repeated_wide_updates", {"n": 200000, "q": 300000}, {"n": 28, "q": 21},
         "Each update shifts all ratings still below the top boundary and leaves larger ratings at 500001."),
        ("cascade_equal_ratings", {"n": 200000, "q": 300000}, {"n": 35, "q": 27},
         "Successive singleton ranges merge longer equal-rating runs, retaining the higher original ratings.")),
    "nearest_value_index": (
        ("dense_two_sided_rank_search", {"n": 100000, "q": 100000}, {"n": 43, "q": 39},
         "Reverse the consecutive coordinates, then vary both center and rank so the shorter side sometimes runs out."),
        ("two_large_distance_tie_classes", {"n": 100000, "q": 100000}, {"n": 34, "q": 29},
         "Half the points lie at each constraint endpoint. Queries at the midpoint make every distance tie.")),
    "grid_pattern_matching_all": (
        ("dense_overlapping_occurrences", {"height": 1000, "width": 1000, "rows": 2, "columns": 3},
         {"height": 21, "width": 24, "rows": 2, "columns": 3},
         "Every valid top-left coordinate matches the uniform rectangle; almost one million matches in the full case."),
        ("distinct_full_width_rows", {"height": 1000, "width": 1000, "rows": 1000, "columns": 1000},
         {"height": 19, "width": 23, "rows": 19, "columns": 23},
         "The only possible placement covers the entire text. Distinct row prefixes keep most trie nodes separate.")),
    "edit_alignment_sequence_matching": (
        ("disjoint_alphabets", {"n": 1000}, {"n": 33},
         "Every target element must be created by a replacement or insertion, and all replacements suffice."),
        ("shift_by_one", {"n": 1000}, {"n": 37},
         "Erase the first element and insert it after the rest. Equal-length arrays are different at every index.")),
    "adjacent_swap_distance": (
        ("reversed_adjusted_permutation", {"n": 200000}, {"n": 21},
         "After adding indices the first array is increasing and the second is its reverse."),
        ("reversed_equal_value_blocks", {"n": 200000}, {"n": 24},
         "Every element of one equal-value block crosses every element of the other block.")),
    "interval_lis": (
        ("all_intervals_usable", {"n": 100000}, {"n": 30},
         "Every interval contains 1 through n, so choosing the index in each one gives length n."),
        ("decreasing_singletons", {"n": 100000}, {"n": 30},
         "The only permitted values are decreasing, so the longest increasing subsequence has length one.")),
    "interval_schedule_max_weight": (
        ("disjoint_slots", {"n": 200000}, {"n": 10},
         "All singleton slots are distinct, so every request can be accepted."),
        ("paired_requests", {"n": 200000}, {"n": 10},
         "Each slot has two requests. Taking the higher profit from every pair is optimal.")),
    "lis_count": (
        ("paired_increasing_levels", {"n": 200000}, {"n": 30},
         "A longest increasing subsequence chooses either occurrence of each distinct value."),
        ("descending_distinct_values", {"n": 200000}, {"n": 30},
         "Each single element is a longest increasing subsequence.")),
    "modular_pair_sum_order": (
        ("all_residues_in_both_lists", {"n": 100000, "modulus": 100000}, {"n": 30, "modulus": 30},
         "Each residue occurs in exactly n pairs."),
        ("all_pairs_wrap_with_large_modulus", {"n": 100000, "modulus": 10000000}, {"n": 31, "modulus": 126},
         "Adding modulus-1 decreases each left residue by one modulo the modulus.")),
    "monge_shortest_paths_baseball": (
        ("uniform_weights_many_centers", {"n": 50000, "k": 1779}, {"n": 17, "k": 4},
         "Unit weights split optimally into nearly equal contiguous blocks with their medians as centers."),
        ("sparse_equispaced_weighted_sites", {"n": 50000, "k": 421, "spacing": 7, "weight": 13},
         {"n": 16, "k": 3, "spacing": 3, "weight": 3},
         "Only regularly spaced equally weighted sites contribute; scale the uniform block answer by spacing and weight.")),
    "pair_product_order": (
        ("largest_distinct_positive_product", {"n": 200000, "value": 1000000000}, {"n": 25, "value": 1000},
         "The two largest distinct values give the unique largest product."),
        ("last_negative_product_at_extremes", {"n": 200000, "value": 1000000000}, {"n": 27, "value": 1000},
         "Only opposite-sign pairs have a negative product, and all those products are equal.")),
    "parallel_binary_search_stamp_rally": (
        ("chain_components", {"n": 100000, "q": 100000}, {"n": 23, "q": 34},
         "Before the last edge, the chain prefix and the last singleton cover t+2 vertices."),
        ("star_shared_component", {"n": 100000, "q": 100000}, {"n": 23, "q": 34},
         "Both starts belong to the first star edge, and t spokes reach t+1 vertices.")),
    "subarray_mex_histogram": (
        ("identity_permutation", {"n": 200000}, {"n": 34},
         "The n prefixes have mex 1 through n. Every other interval omits 0."),
        ("alternating_out_of_range", {"n": 200000}, {"n": 34},
         "Only isolated large values omit 0. Every other interval has mex 1.")),
    "rollback_lis_tree": (
        ("decreasing_chain", {"n": 200000}, {"n": 32},
         "Every path is strictly decreasing, so its longest increasing subsequence has length one."),
        ("increasing_binary_tree", {"n": 200000}, {"n": 32},
         "Every root-to-vertex path has increasing labels and values.")),
    "state_cycle_sequence_sum": (
        ("maximum_count_fixed_point", {"n": 10000000000, "modulus": 100000}, {"n": 78, "modulus": 43},
         "Every state equals one, so the sum equals the number of terms."),
        ("long_modular_orbit", {"n": 10000000000, "modulus": 99707}, {"n": 91, "modulus": 47},
         "An explicit table of visited states finds the transient and the 49852-term full-size period.")),
    "subset_sum_meet_in_middle_max": (
        ("odd_limit_even_weights", {"n": 40}, {"n": 12},
         "Weights are twice 1 through n. Every even sum up to the total is attainable."),
        ("every_weight_too_large", {"n": 40}, {"n": 12},
         "Every nonempty subset exceeds the limit, so only the empty subset is feasible.")),
    "weighted_monotone_matching_flowers": (
        ("all_increasing_and_heavy", {"n": 200000}, {"n": 23},
         "Every height is increasing and each positive beauty can be retained."),
        ("decreasing_keys_with_one_heavy_item", {"n": 200000}, {"n": 23},
         "Only a singleton is possible, and one flower has the largest beauty 10^9.")),
    "bracket_sequence_range": (
        ("nested_pairs_after_point_replacements", {"n": 200000, "q": 200000}, {"n": 46, "q": 32},
         "A prefix contains min(number of opens, number of closes) pairs, including one possible close at its midpoint."),
        ("all_closes_before_all_opens", {"n": 200000, "q": 200000}, {"n": 46, "q": 32},
         "Changing the last close at the boundary preserves the close-then-open order, so no pair can be removed.")),
    "concatenated_pattern_count": (
        ("doubling_overlap", {"n": 100000, "pattern_length": 50}, {"n": 9, "pattern_length": 6},
         "A run of length m*2^(n-1) contains m*2^(n-1)-m+1 occurrences of a^m."),
        ("boundary_match_chain", {"n": 100000, "pattern_length": 50}, {"n": 14, "pattern_length": 6},
         "Every copy of a^(m-1)b contributes one match, and no match crosses its final b.")),
    "cyclic_rotation": (
        ("alternating_ties", {"n": 1000}, {"n": 24},
         "Every rotation is one of the two alternating strings."),
        ("one_distinguishing_byte", {"n": 1000}, {"n": 25},
         "The only b can be placed last for the minimum, or first for the maximum.")),
    "distinct_subsequence_order": (
        ("long_single_byte_result", {"n": 1000000}, {"n": 12},
         "Each distinct subsequence uses between 1 and n copies of a. The last uses all n."),
        ("huge_rank_with_four_repetitions", {"n": 1000000}, {"n": 12},
         "The first block has its full length before the last choice of the three trailing blocks, using only the last byte.")),
    "dynamic_palindrome": (
        ("center_updates", {"n": 999999, "q": 100000}, {"n": 63, "q": 36},
         "Changing the center of an odd-length palindrome preserves symmetry."),
        ("edge_updates", {"n": 1000000, "q": 100000}, {"n": 64, "q": 36},
         "The final character remains a, so the whole string matches exactly when the first is a.")),
    "dynamic_substring_matching": (
        ("long_almost_equal_runs", {"n": 5000, "q": 5000, "m": 1000}, {"n": 200, "q": 24, "m": 40},
         "Every a-run has length m-1 until the first separator is changed back to a."),
        ("rare_byte_at_last_position", {"n": 5000, "q": 5000, "m": 1000}, {"n": 153, "q": 28, "m": 65},
         "The searched word can end only at the final position, when it holds the unique b.")),
    "hamming_distance_matching_binary_min": (
        ("dense_alternating_one_flip", {"n": 1000000, "m": 500000}, {"n": 48, "m": 22},
         "Every window is one of two alternating words. Flipping one byte changes the distances to 1 and m-1."),
        ("one_complementary_window", {"n": 1000000, "m": 1000000}, {"n": 48, "m": 48},
         "Equal lengths allow one window. Each of its bytes differs from the pattern.")),
    "offline_prefix_lcp_sum": (
        ("one_deep_branch", {"n": 100000, "q": 200000}, {"n": 16, "q": 40},
         "Only one word is extended. It matches every other word for one byte and itself for its full length."),
        ("shared_extensions", {"n": 100000, "q": 200000}, {"n": 16, "q": 32},
         "Same-first-byte words contribute one each, and words already extended in that cohort contribute one more.")),
    "run_length_z_sum": (
        ("equal_alternating_runs", {"n": 200000, "length": 1000000000}, {"n": 16, "length": 3},
         "Positions inside a runs have triangular partial matches. At a run start the entire remaining suffix matches."),
        ("unique_first_byte", {"n": 200000, "length": 1000000000}, {"n": 15, "length": 4},
         "The first run contributes triangular partial matches and one full-text match. All other positions start with another byte.")),
    "shortest_absent_subsequence": (
        ("maximum_single_byte_text", {"n": 200000}, {"n": 20},
         "The smallest missing letter is b because the entire text consists only of a."),
        ("all_symbols_in_each_completed_block", {"n": 200000}, {"n": 55},
         "Each complete alphabet block contributes one arbitrary subsequence letter, but the final partial block provides no extra a.")),
    "shortest_common_superstring": (
        ("unrelated_runs", {"n": 20, "block": 5000}, {"n": 6, "block": 3},
         "Each word uses a disjoint symbol, so all N runs must appear separately."),
        ("ordered_overlaps", {"n": 20, "block": 5000}, {"n": 6, "block": 3},
         "The only overlaps form a path, saving one run of length block at each of N-1 transitions.")),
    "string_periods_best_representation": (
        ("one_repeated_byte", {"n": 500000}, {"n": 27},
         "Only single-character pieces are primitive."),
        ("alternating_primitive_splits", {"n": 500000}, {"n": 28},
         "Of all cuts, only odd cut positions make both pieces primitive for even n at least six.")),
    "substring_comparison_common_prefix": (
        ("identical_long_prefixes", {"n": 100000, "q": 3000000}, {"n": 12, "q": 41},
         "Each pair of distinct words has common prefix length eight."),
        ("one_byte_forks", {"n": 100000, "q": 3000000}, {"n": 12, "q": 36},
         "Each n-query cycle compares word zero with all others and compares it with word one twice.")),
    "suffix_tree_nonoverlapping_repeat": (
        ("identical_bytes", {"n": 5000}, {"n": 36},
         "Two copies of the first floor(n/2) characters attain the non-overlap bound."),
        ("matching_both_sides_of_separator", {"n": 4999}, {"n": 37},
         "Equal blocks on either side of the separator attain the non-overlap bound.")),
}

VALIDITY = {
    "coordinate_compression": "1<=N<=100000, coordinates in [1,10^9], and all pairs distinct.",
    "point_set_mex": "1<=N,Q<=200000; 0<=A_i,x_i<=10^9; every update position is in 1..N.",
    "sliding_window_extrema": "1<=L<=N<=1000000, 1<=A_i<=10^9.",
    "pair_sum_order_handshake": "1<=N<=100000, 1<=M<=N*N, all 1<=A_i<=100000.",
    "string_radix_order_alphabet": "2<=N<=50000; alphabet is a lowercase permutation; distinct lowercase words have length in 1..10.",
    "dynamic_prefix_filter": "1<=Q<=200000; all words are nonempty lowercase; total word length <=500000.",
    "prefix_bitmap_table": "1<=N<=1000, 1<=L<=3000, 1<=Q<=4000; lowercase fixed-width rows, distinct valid substitution bytes and legal prefixes.",
    "interval_map_paint": "1<=N<=500000, 1<=Q<=200000; 1<=x,c<=N, at least one count query.",
    "subarray_median_order": "1<=N<=100000; each value is in 1..10^9.",
    "range_equal_runs": "1<=N,Q<=200000; 0<=a_i,x<=10^9, valid 1-indexed point and inclusive range queries.",
    "lexicographic_subsequence": "1<=K<=N<=100000; the string contains N lowercase English letters.",
    "range_nondecreasing_subsequence": "1<=N,Q<=200000; letters A-D and valid 1-indexed positions/ranges.",
    "cyclic_rotation_pairs": "1<=N<=200000; both strings consist of N lowercase English letters.",
    "cyclic_rotation_inversions": "2<=N<=300000; the input is a permutation of 0..N-1.",
    "cyclic_rotation_inversions_adjacent_swaps": "2<=N<=200000, 1<=Q<=200000; permutation of 1..N, 1<=x<N, 1<=K<=10^9.",
    "range_add_threshold_min": "1<=K<=N<=100000, 1<=Q<=50000, rents in 1..10^9, binary parking and thresholds in 0..K.",
    "bounded_zigzag_subsequence": "1<=N<=200000; 1<=L<=R<=10^9; heights in 1..10^9 and scores in -10^9..10^9.",
    "pattern_erasure": "1<=|S|<=200000 and every byte belongs to ABC.",
    "pattern_avoidance": "1<=N<=10^18, 1<=M<=126; distinct binary patterns of length 1..6.",
    "sorted_substring_index": "1<=N,Q<=100000, S and replacement characters lowercase English; valid 1-indexed positions.",
    "forbidden_sum_partitions": "1<=N<=200000; |K|<=10^15; each absolute item value <=10^9.",
    "lattice_shape_transforms": "1<=N<=200 and both N-by-N shapes of # and dot have at least one occupied cell.",
    "cyclic_xor_matches": "1<=N<=200000; both input sequences consist of N integers in 0..2^30-1.",
    "ordered_multiset_extrema": "1<=Q<=200000; 0<=keys<=10^9; 1<=delete count<=Q; extrema queries are nonempty.",
    "chord_crossing_queries": "2<=N<=10^6; 1<=M<=min(N//2,200000); 1<=Q<=200000; unique even base endpoints and odd queries in 1..2N.",
    "dominance_counts": "1<=N<=200000; A_i and B_i are integers in 0..10^9; order and duplicates are unrestricted.",
    "sparse_obstacle_grid_paths": "2<=H,W<=100000; 1<=K<=3000; walls are distinct in-grid cells other than endpoints.",
    "target_sum_subrectangles": "1<=T<=25000; H,W>=1; total H*W<=300000; grid bytes are # and dot.",
    "knuth_interval_dp": "2<=N<=3000 and all N circular tablet weights in 1..10^9.",
    "binary_grid_squares": "1<=H,W<=3000; 0<=obstacles<=min(H*W,10^5); all obstacle cells are distinct.",
    "run_length_queue": "1<=Q<=200000; 1<=count,value<=10^9; removals consume no more than the current length or 10^9.",
    "repeated_subsequence": "1<=N<=10^12; 1<=|S|,|T|<=100000; both strings contain lowercase English letters.",
    "balanced_pivot_subsequence": "1<=N,Q<=100000; S in {1,2,/}^N; 1<=L<=R<=N.",
    "three_symbol_frequency_counts": "1<=N<=200000; input contains only A, B, and C.",
    "pattern_range_occurrences": "1<=|S|<=400000; 1<=|T|<=10; 1<=Q<=200000; 1<=L_i<=R_i<=|S|; lowercase ASCII.",
    "exact_subarray_extrema": "1<=N<=200000; each value and extrema in 1..200000; Y<=X.",
    "longest_bitonic_subsequence": "1<=N<=300000; 1<=A_i<=N.",
    "poisson_binomial": "1<=N<=2999 and N is odd; probabilities have two decimal places in 0.01..0.99.",
    "range_add_total_variation": "1<=N,Q<=100000; 1<=L_i<=R_i<=N; |A_i|,|V_i|<=10^9.",
    "inversion_limited_partitions": "1<=N<=200000; 0<=K<=N(N-1)/2; 1<=A_i<=10^9.",
    "dictionary_segmenter": "1<=|S|<=100000; S uses lowercase ASCII letters.",
    "deadline_job_selection": "1<=N<=5000; 1<=D_i,C_i<=5000; 1<=S_i<=10^9.",
    "periodic_pattern_runs": "1<=|S|,|T|<=500000; both strings use lowercase ASCII letters.",
    "pattern_stamping": "1<=N<=200000; 1<=M<=min(N,5); both strings use uppercase ASCII letters.",
    "subarray_frequency_windows_distinct": "1<=K<=N<=100000; 1<=A_i<=10^9.",
    "subarray_sum_counts_equal": "1<=N<=200000; |K|<=10^15; |A_i|<=10^9.",
    "clamped_translation": "1<=N,Q<=200000; |a_i|,|x_j|<=10^9; t_i in 1..3.",
    "weighted_interval_stabbing": "1<=N,M<=200000; 1<=L_i<=R_i<=N; absolute weights <=10^9.",
    "maximum_allowed_pair": "1<=N,M<=100000; 0<=P<=min(100000,N*M-1); positive scores <=10^9; P distinct valid pairs.",
    "monotone_value_shift": "1<=N<=200000; 1<=Q<=300000; 1<=L_i<=R_i<=500000; 1<=X_j<=500000.",
    "nearest_value_index": "1<=N,Q<=100000; coordinates in -10^8..10^8 and ranks in 1..N.",
    "grid_pattern_matching_all": "1<=H,W,R,C<=1000; every grid has exactly its declared dimensions and only ASCII alphanumeric bytes.",
    "edit_alignment_sequence_matching": "1<=N,M<=1000 and all sequence elements are in [1,10^9].",
    "adjacent_swap_distance": "2<=N<=200000 and both integer arrays have all values in 0..10^9.",
    "interval_lis": "1<=N<=100000 and intervals satisfy 1<=left<=right<=100000.",
    "interval_schedule_max_weight": "1<=N,T<=200000; inclusive integer slots in 1..T; profits in 1..10^9.",
    "lis_count": "1<=N<=200000; all input values are integers in [-10^9,10^9].",
    "monge_shortest_paths_baseball": "1<=K<=N<=50000; 0<=P_i<=100000, 1<=sum(P)<=100000.",
    "modular_pair_sum_order": "1<=N<=100000, 1<=rank<=N^2, 2<=modulus<=10^7, all values in [0,modulus).",
    "pair_product_order": "2<=N<=200000, 1<=rank<=N(N-1)/2, each value in [-10^9,10^9].",
    "parallel_binary_search_stamp_rally": "3<=N<=100000; N-1 sorted-endpoint edges form a tree; Q<=100000; distinct numbered starts; thresholds 3..N.",
    "subarray_mex_histogram": "1<=N<=200000; all array values lie in 0..10^9.",
    "rollback_lis_tree": "2<=N<=200000; distinct integer endpoints form a tree; every value lies in 1..10^9.",
    "state_cycle_sequence_sum": "1<=N<=10^10; 0<=initial state<modulus<=100000.",
    "subset_sum_meet_in_middle_max": "1<=N<=40; all weights and the limit lie in 1..10^9.",
    "weighted_monotone_matching_flowers": "1<=N<=200000, heights permute 1..N, all beauties in 1..10^9.",
    "bracket_sequence_range": "1<=N,Q<=200000, text contains only parentheses, each replacement and range stays in bounds.",
    "concatenated_pattern_count": "N<=100000; 1<=|T|<=50; alphanumeric literal characters total at most 100000; only earlier stages are referenced.",
    "cyclic_rotation": "The lowercase input contains 1..1000 characters.",
    "distinct_subsequence_order": "1<=text length<=10^6, lowercase bytes, 1<=rank<=10^18.",
    "dynamic_palindrome": "N<=1000000, Q<=100000; lowercase bytes; update and query positions lie in 1..N.",
    "dynamic_substring_matching": "1<=N,Q<=5000; 1<=pattern length<=1000; lowercase words and legal point updates.",
    "hamming_distance_matching_binary_min": "1<=M<=N<=1000000; both strings consist of 0 and 1.",
    "offline_prefix_lcp_sum": "1<=N,Q<=200000, initial total length <=200000; lowercase strings, valid indices, at least one sum query.",
    "run_length_z_sum": "1<=N<=200000, each length in 1..10^9; adjacent lowercase run bytes differ.",
    "shortest_absent_subsequence": "The text consists of 1..200000 lowercase bytes.",
    "shortest_common_superstring": "1<=N<=20; all words are nonempty lowercase; total length <=200000.",
    "string_periods_best_representation": "The lowercase input contains 1..500000 characters.",
    "substring_comparison_common_prefix": "2<=N<=100000; total lowercase length <=800000; M<=3000000; seed and delta in [0,N*(N-1)).",
    "suffix_tree_nonoverlapping_repeat": "2<=N<=5000; the input length matches N and all characters are lowercase.",
}


def _adjacent_swap(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "reversed_adjusted_permutation":
        inp.line(*([n] * n))
        inp.line(*(2 * n - 1 - 2 * i for i in range(n)))
        out.line(n * (n - 1) // 2)
    else:
        half = n // 2
        inp.line(*(n - i + int(i >= half) for i in range(n)))
        inp.line(*(n - i + int(i < half) for i in range(n)))
        out.line(half * half)


def _interval_lis(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "all_intervals_usable":
        for _ in range(n):
            inp.line(1, n)
        out.line(n)
    else:
        for i in range(n, 0, -1):
            inp.line(i, i)
        out.line(1)


def _intervals(inp, out, n: int, scenario: str) -> None:
    profit = 10**9
    if scenario == "disjoint_slots":
        inp.line(n, n)
        for i in range(n, 0, -1):
            inp.line(i, i, profit)
        out.line(n * profit)
        return
    inp.line(n, n // 2)
    for i in range(n // 2, 0, -1):
        inp.line(i, i, 1)
        inp.line(i, i, profit)
    out.line(n // 2 * profit)


def _stamp_rally(inp, out, n: int, q: int, scenario: str) -> None:
    chain = scenario == "chain_components"
    inp.line(n, n - 1)
    for vertex in range(2, n + 1):
        inp.line(vertex - 1 if chain else 1, vertex)
    inp.line(q)
    for i in range(q):
        target = 3 + (997 * i) % (n - 2)
        inp.line(1, n if chain else 2, target)
        out.line(target - (2 if chain else 1))


def _lis_count(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "paired_increasing_levels":
        inp.line(*(i // 2 for i in range(n)))
        out.line(pow(2, n // 2, 1_000_000_007))
    else:
        inp.line(*range(n, 0, -1))
        out.line(n)


def _mex_histogram(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "identity_permutation":
        inp.line(*range(n))
        out.line(n * (n - 1) // 2)
        for _ in range(n):
            out.line(1)
        return
    inp.line(*(0 if i % 2 == 0 else 10**9 for i in range(n)))
    out.line(n // 2)
    out.line(n * (n + 1) // 2 - n // 2)
    for _ in range(n - 1):
        out.line(0)


def _rollback_lis(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    binary = scenario == "increasing_binary_tree"
    if binary:
        inp.line(*range(1, n + 1))
    else:
        inp.line(*range(n, 0, -1))
    for vertex in range(2, n + 1):
        inp.line(vertex // 2 if binary else vertex - 1, vertex)
    for vertex in range(1, n + 1):
        out.line(vertex.bit_length() if binary else 1)


def _modular_pair_sum(inp, out, n: int, modulus: int, scenario: str) -> None:
    if scenario == "all_residues_in_both_lists":
        rank = n * n * 3 // 4 + 1
        inp.line(n, rank, modulus)
        inp.line(*range(n))
        inp.line(*range(n - 1, -1, -1))
        out.line((rank - 1) // n)
    else:
        inp.line(n, n * (n - 1) + 1, modulus)
        inp.line(*range(modulus - n, modulus))
        inp.line(*([modulus - 1] * n))
        out.line(modulus - 2)


def _pair_product(inp, out, n: int, value: int, scenario: str) -> None:
    if scenario == "largest_distinct_positive_product":
        inp.line(n, n * (n - 1) // 2)
        inp.line(*(value - i for i in range(n)))
        out.line(value * (value - 1))
    else:
        half = n // 2
        positive = n - half - 1
        inp.line(n, half * positive)
        inp.line(*([-value] * half), 0, *([value] * positive))
        out.line(-value * value)


def _state_cycle(inp, out, n: int, modulus: int, scenario: str) -> None:
    if scenario == "maximum_count_fixed_point":
        inp.line(n, 1, modulus)
        out.line(n)
        return

    inp.line(n, 2, modulus)
    state = 2
    first = {}
    sums = [0]
    while len(first) < n and state not in first:
        first[state] = len(first)
        sums.append(sums[-1] + state)
        state = state * state % modulus
    if len(first) == n:
        out.line(sums[-1])
    else:
        start = first[state]
        periods, extra = divmod(n - start, len(first) - start)
        out.line(periods * (sums[-1] - sums[start]) + sums[start + extra])


def _subset_sum(inp, out, n: int, scenario: str) -> None:
    if scenario == "odd_limit_even_weights":
        limit = 2 * (n * (n + 1) // 4) + 1
        inp.line(n, limit)
        inp.line(*(2 * weight for weight in range(1, n + 1)))
        out.line(limit - 1)
    else:
        inp.line(n, 1)
        inp.line(*(10**9 - i for i in range(n)))
        out.line(0)


def _weighted_monotone_matching(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "all_increasing_and_heavy":
        inp.line(*range(1, n + 1))
        inp.line(*([1000000000] * n))
        out.line(n * 1000000000)
    else:
        inp.line(*range(n, 0, -1))
        inp.line(*(1000000000 if i == n // 2 else 1 for i in range(n)))
        out.line(1000000000)


def _bracket_sequence(inp, out, n: int, q: int, scenario: str) -> None:
    half = n // 2
    inp.line(n, q)
    if scenario == "nested_pairs_after_point_replacements":
        inp.line("(" * half + ")" * half)
        for i in range(q // 2):
            changed = i % 2
            inp.line(1, half, 1 + changed)
            right = n - (i * 127) % half
            inp.line(2, 1, right)
            out.line(2 * min(half - changed, right - half + changed))
    else:
        inp.line(")" * half + "(" * half)
        for i in range(q // 2):
            inp.line(1, half, 1 + i % 2)
            left = 1 + (i * 193) % half
            right = n - (i * 127) % half
            inp.line(2, left, right)
            out.line(0)


def _concatenated(inp, out, n: int, pattern_length: int, scenario: str) -> None:
    modulus = 998244353
    inp.line(n)
    if scenario == "doubling_overlap":
        pattern = "a" * pattern_length
        inp.line(pattern)
        inp.line(pattern)
        for i in range(1, n):
            inp.line("~", i, i)
        out.line((pattern_length * pow(2, n - 1, modulus) - pattern_length + 1) % modulus)
    else:
        prefix = "a" * (pattern_length - 1)
        inp.line(prefix + "b")
        inp.line(prefix)
        inp.line("b")
        inp.line("~", 1, 2)
        for i in range(3, n):
            inp.line("~", i, 3)
        out.line((n - 2) % modulus)


def _cyclic_rotation(inp, out, n: int, scenario: str) -> None:
    if scenario == "alternating_ties":
        inp.line("ba" * (n // 2))
        out.line("ab" * (n // 2))
        out.line("ba" * (n // 2))
    else:
        inp.line("a" * (n // 2) + "b" + "a" * (n - n // 2 - 1))
        out.line("a" * (n - 1) + "b")
        out.line("b" + "a" * (n - 1))


def _dynamic_palindrome(inp, out, n: int, q: int, scenario: str) -> None:
    center = scenario == "center_updates"
    inp.line(n, q)
    inp.line("a" * n)
    for i in range(q // 2):
        char = "b" if i % 2 == 0 else "a"
        inp.line(1, n // 2 + 1 if center else 1, char)
        inp.line(2, 1, n)
        out.line("Yes" if center or char == "a" else "No")


def _distinct_subsequence_order(inp, out, n: int, scenario: str) -> None:
    if scenario == "long_single_byte_result":
        word = "a" * n
        inp.line(word)
        inp.line(n)
        out.line(word)
    else:
        block = n // 4
        inp.line("".join(byte * block for byte in "abcd"))
        inp.line(block + (block + 1) ** 3 - 1)
        out.line("a" * block + "d" * block)


def _dynamic_substring(inp, out, n: int, q: int, m: int, scenario: str) -> None:
    inp.line(n, q)
    if scenario == "long_almost_equal_runs":
        inp.line(("a" * (m - 1) + "b") * (n // m))
        pattern = "a" * m
        position = m
        on, off = "a", "b"
    else:
        inp.line("a" * (n - 1) + "b")
        pattern = "a" * (m - 1) + "b"
        position = n
        on, off = "b", "a"
    for turn in range(q // 2):
        match = turn % 2 == 0
        inp.line(1, position, on if match else off)
        inp.line(2, pattern)
        out.line("Yes" if match else "No")


def _hamming_distance(inp, out, n: int, m: int, scenario: str) -> None:
    inp.line("01" * (n // 2))
    if scenario == "dense_alternating_one_flip":
        inp.line("10" * (m // 2 - 1) + "11")
        out.line(1)
    else:
        inp.line("10" * (m // 2))
        out.line(m)


def _offline_prefix_lcp_sum(inp, out, n: int, q: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "one_deep_branch":
        for _ in range(n):
            inp.line("a")
        inp.line(q)
        for i in range(q // 2):
            inp.line(1, 1, "a")
            inp.line(2, 1)
            out.line(n + i + 1)
    else:
        for i in range(n):
            inp.line("a" if i % 2 == 0 else "b")
        inp.line(q)
        for i in range(q // 2):
            inp.line(1, i + 1, "c")
            inp.line(2, i + 1)
            out.line(n // 2 + i // 2 + 1)


def _run_length_z(inp, out, n: int, length: int, scenario: str) -> None:
    inp.line(n)
    inp.line(*([length] * n))
    partial = length * (length - 1) // 2
    if scenario == "equal_alternating_runs":
        inp.line("ab" * (n // 2))
        halves = n // 2
        total = length * halves * (halves + 1) + halves * partial
    else:
        inp.line("z" + ("ab" * ((n + 1) // 2))[:n - 1])
        total = length * n + partial
    out.line(total % 1_000_000_007)


def _shortest_absent_subsequence(inp, out, n: int, scenario: str) -> None:
    if scenario == "maximum_single_byte_text":
        inp.line("a" * n)
        out.line("b")
    else:
        blocks, tail = divmod(n, 26)
        inp.line("abcdefghijklmnopqrstuvwxyz" * blocks + "bcdefghijklmnopqrstuvwxyz"[:tail])
        out.line("a" * (blocks + 1))


def _shortest_superstring(inp, out, n: int, block: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "unrelated_runs":
        for i in range(n):
            inp.line(chr(ord("a") + i) * (2 * block))
        out.line(2 * block * n)
    else:
        for i in range(n):
            inp.line(chr(ord("a") + i) * block + chr(ord("b") + i) * block)
        out.line(block * (n + 1))


def _string_periods(inp, out, n: int, scenario: str) -> None:
    if scenario == "one_repeated_byte":
        inp.line("a" * n)
        out.line(n)
        out.line(1)
    else:
        inp.line("ab" * (n // 2))
        out.line(2)
        out.line(n // 2)


def _substring_comparison(inp, out, n: int, q: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "identical_long_prefixes":
        for _ in range(n):
            inp.line("a" * 8)
        modulus = n * (n - 1)
        inp.line(q, 71 % modulus, (17 * n + 7) % modulus)
        out.line(8 * q)
    else:
        for i in range(n):
            inp.line("aaaaaaab" if i % 2 == 0 else "aaaaaaac")
        inp.line(q, 0, n - 1)
        out.line(8 * q - (q // n) * (n // 2 + 1))


def _suffix_tree(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "identical_bytes":
        inp.line("a" * n)
    else:
        half = ("abc" * (n // 6 + 1))[:n // 2]
        inp.line(half + "d" + half)
    out.line(n // 2)


def _monge_baseball(inp, out, n: int, k: int, scenario: str, spacing: int = 1, weight: int = 1) -> None:
    count = (n - 1) // spacing + 1
    width, extra = divmod(count, k)
    short_cost = width * width // 4
    long_cost = (width + 1) * (width + 1) // 4
    inp.line(n, k)
    inp.line(*(weight if i % spacing == 0 else 0 for i in range(n)))
    out.line(weight * spacing * ((k - extra) * short_cost + extra * long_cost))


def _point_set_mex(inp, out, n: int, scenario: str) -> None:
    upper_bound = 10**9
    if scenario == "long_distance_mex_jumps":
        inp.line(n, n)
        inp.line(*range(n))
        for turn in range(0, n, 2):
            position = (turn // 2 * 511) % n
            inp.line(position + 1, upper_bound)
            inp.line(position + 1, position)
            out.line(position)
            out.line(n)
    else:
        inp.line(n, n)
        inp.line(*([upper_bound] * n))
        for position in range(n):
            inp.line(position + 1, position)
            out.line(position + 1)


def _sliding_window_extrema(inp, out, n: int, width: int, scenario: str) -> None:
    inp.line(n, width)
    if scenario == "increasing_full_ring":
        inp.line(*range(1, n + 1))
        out.line(*range(1, n - width + 2))
    else:
        values = [1 + (n - i) // 4 for i in range(n)]
        inp.line(*values)
        out.line(*values[width - 1:])


def _pair_sum_handshake(inp, out, n: int, scenario: str) -> None:
    if scenario == "large_threshold_tie":
        rank = n * n - 17
        inp.line(n, rank)
        inp.line(*([100000] * n))
        out.line(200000 * rank)
        return
    inp.line(n, n * (n + 1) // 2)
    inp.line(*range(1, n + 1))
    out.line(sum(i * i + i * (2 * n + 1 - i) // 2 for i in range(1, n + 1)))


def _string_radix_order(inp, out, n: int, scenario: str) -> None:
    if scenario == "maximum_width_shared_prefix":
        alphabet = "zyxwvutsrqponmlkjihgfedcba"

        def word(number):
            digits = ((number // 26**power) % 26 for power in range(3, -1, -1))
            return "aaaaaa" + "".join(alphabet[digit] for digit in digits)

        inp.line(alphabet)
        inp.line(n)
        for number in range(n - 1, -1, -1):
            inp.line(word(number))
        for number in range(n):
            out.line(word(number))
        return

    alphabet = "klmnopqrstuvwxyzabcdefghij"

    def word(number):
        letters = []
        while number:
            number, digit = divmod(number - 1, len(alphabet))
            letters.append(alphabet[digit])
        return "".join(reversed(letters))

    def visit(prefix, number):
        if number > n:
            return
        if number:
            out.line(prefix)
        for digit, byte in enumerate(alphabet, 1):
            visit(prefix + byte, 26 * number + digit)

    inp.line(alphabet)
    inp.line(n)
    for number in range(n, 0, -1):
        inp.line(word(number))
    visit("", 0)


def _dynamic_prefix_filter(inp, out, scenario: str, length: int = 0, q: int = 0) -> None:
    if scenario == "maximum_length_chain":
        inp.line(3)
        inp.line(2, "a" * length)
        inp.line(1, "a")
        inp.line(2, "b")
        for count in (1, 0, 1):
            out.line(count)
        return

    before = q // 2
    inp.line(q)
    for i in range(before):
        inp.line(2, "aaaa" if i == 0 else "aaa" if i % 2 == 0 else "baa")
        out.line(i + 1)
    inp.line(1, "b")
    count = (before + 1) // 2
    out.line(count)
    for i in range(q - before - 1):
        inp.line(2, "aa" if i % 2 == 0 else "bb")
        count += i % 2 == 0
        out.line(count)


def _prefix_bitmap_table(inp, out, n: int, width: int, q: int, scenario: str) -> None:
    inp.line(n, width, q)
    row = "a" * width
    for _ in range(n):
        inp.line(row)
    if scenario == "maximum_width_every_row_matches":
        for _ in range(q):
            inp.line(2, row)
            out.line(n)
        return

    query = "a" * (width - 1) + "b"
    for step in range(q // 2):
        if step % 2 == 0:
            inp.line(1, width, "a", "b")
            out.line(n)
        else:
            inp.line(1, width, "b", "a")
            out.line(0)
        inp.line(2, query)


def _interval_map_paint(inp, out, n: int, q: int, scenario: str) -> None:
    inp.line(n, q)
    for turn in range(q // 2):
        if scenario == "growing_prefix_run":
            color = turn + 2
            inp.line(1, 1, color)
            inp.line(2, color)
        else:
            inp.line(1, 5 + 2 * turn, 1)
            inp.line(2, 1)
        out.line(turn + 2)


def _subarray_median_order(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "alternating_even_width_ties":
        inp.line(*(10**9 if i % 2 else 1 for i in range(n)))
        out.line(10**9)
        return

    inp.line(*range(n, 0, -1))
    cumulative = 0
    middle = n * (n + 1) // 4
    for center in range(n):
        for endpoint_sum in (2 * center - 1, 2 * center):
            low = max(0, endpoint_sum - (n - 1))
            high = endpoint_sum // 2
            cumulative += max(0, high - low + 1)
        if cumulative > middle:
            out.line(center + 1)
            break


def _range_equal_runs(inp, out, n: int, q: int, scenario: str) -> None:
    inp.line(n, q)
    if scenario == "alternating_middle_split":
        middle = n // 2
        values = [1] * n
        values[middle] = 0
        inp.line(*values)
        for turn in range(q // 2):
            value = 1 - turn % 2
            inp.line(1, middle + 1, value)
            inp.line(2, 1, n)
            out.line(n if value else max(middle, n - middle - 1))
    else:
        inp.line(*range(n))
        for turn in range(q // 2):
            right = 2 * turn + 2
            inp.line(1, right, right - 2)
            inp.line(2, right - 1, right)
            out.line(2)


def _lexicographic_subsequence(inp, out, n: int, k: int, scenario: str) -> None:
    inp.line(n, k)
    if scenario == "long_suffix_replaces_prefix":
        inp.line("b" * (n - k) + "a" * k)
        out.line("a" * k)
    else:
        alphabet = "zyxwvutsrqponmlkjihgfedcba"
        text = (alphabet * ((n - 1) // len(alphabet) + 1))[:n]
        inp.line(text)
        out.line(text)


def _range_nondecreasing_subsequence(inp, out, n: int, q: int, scenario: str) -> None:
    inp.line(n, q)
    if scenario == "descending_alphabet_cycles":
        inp.line("DCBA" * (n // 4))
        for turn in range(q // 2):
            changed = turn % 2 == 0
            inp.line(1, 1, "A" if changed else "D")
            inp.line(2, 1, n)
            out.line(n - n // 4 - int(changed))
    else:
        half = n // 2
        inp.line("D" * half + "A" * (n - half))
        for turn in range(q // 2):
            changed = turn % 2 == 0
            inp.line(1, half + 1, "D" if changed else "A")
            left = 1 + (37 * turn + 3) % half
            right = half + 1 + (31 * turn + 11) % (n - half)
            inp.line(2, left, right)
            split = half + int(changed)
            out.line(min(split - left + 1, right - split))


def _cyclic_rotation_pairs(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "equal_classes_low_period":
        inp.line("ab" * (n // 2))
        inp.line("ba" * (n // 2))
        out.line(3 * n * n // 4)
    else:
        inp.line("a" * (n - 1) + "z")
        inp.line("a" * (n - 1) + "y")
        out.line(n * (n - 1) // 2)


def _cyclic_rotation_inversions(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    ascending = scenario == "ascending_max_permutation"
    inp.line(*(range(n) if ascending else range(n - 1, -1, -1)))
    total = n * (n - 1) // 2
    for shift in range(n):
        increased = shift * (n - shift)
        out.line(increased if ascending else total - increased)


def _cyclic_rotation_inversions_adjacent_swaps(inp, out, n: int, q: int, high: int, scenario: str) -> None:
    modulus = 998244353
    inv_n = pow(n, -1, modulus)
    pair_count = n * (n - 1) // 2
    sorted_rotation_sum = n * (n * n - 1) // 6
    inp.line(n, q)
    inp.line(*range(1, n + 1))
    for turn in range(q):
        if scenario == "toggle_first_pair":
            position, inversions, steps = 1, (turn + 1) % 2, 1
        else:
            pairs = n // 2
            position = 2 * (turn % pairs) + 1
            phase = (turn + 1) % (2 * pairs)
            inversions = min(phase, 2 * pairs - phase)
            steps = high - turn % 2
        inp.line(position, steps)
        rotation_sum = sorted_rotation_sum + inversions * (n - 2)
        histories = pow(n - 1, steps, modulus)
        sign = -1 if steps % 2 else 1
        common = ((histories - sign) * inv_n) % modulus
        answer = common * rotation_sum + sign * inversions
        if sign < 0:
            answer = histories * pair_count - answer
        out.line(answer % modulus)


def _range_add_threshold_min(inp, out, n: int, k: int, q: int, scenario: str) -> None:
    uniform = scenario == "uniform_keys_wide_alternating_adds"
    parked = n if uniform else n // 6
    position = n // 2 if uniform else parked + k // 2
    expensive = 10**9
    inp.line(n, k, q)
    inp.line(*([expensive] * n))
    inp.line(*([1] * parked + [0] * (n - parked)))
    for turn in range(q // 2):
        cheap = turn % 2 == 0
        inp.line(1, position + 1, 1 if cheap else expensive)
        threshold = k if uniform else (turn * 8191) % (k + 1)
        inp.line(2, threshold, 0)
        eligible_end = min(n - k, parked - threshold) if threshold else n - k
        affected_start = max(0, position + 1 - k)
        if eligible_end < 0:
            out.line("IMPOSSIBLE")
        else:
            saving = expensive - 1 if cheap and affected_start <= eligible_end else 0
            out.line(k * expensive - saving)


def _bounded_zigzag_subsequence(inp, out, n: int, scenario: str) -> None:
    score = 10**9
    wide = scenario == "wide_unique_alternating"
    inp.line(n, 1, 10**9 if wide else 1)
    for i in range(n):
        if wide:
            height = 10**9 - i // 2 if i % 2 else 1 + i // 2
        else:
            height = 1 + 2 * (37 * i % n)
        inp.line(height, score)
    out.line(score * (n if wide else 1))


def _pattern_erasure(inp, out, n: int, scenario: str) -> None:
    if scenario == "nested_cascading_erasure":
        repeats, tail = divmod(n, 3)
        inp.line("A" * repeats + "BC" * repeats + "B" * tail)
        out.line("B" * tail)
    else:
        text = "AB" * (n // 2) + "A" * (n % 2)
        inp.line(text)
        out.line(text)


def _pattern_avoidance(inp, out, length: int, m: int, scenario: str) -> None:
    inp.line(length, m)
    if scenario == "all_126_forbidden_words":
        for width in range(1, 7):
            for value in range(1 << width):
                inp.line(format(value, f"0{width}b").translate(str.maketrans("01", "ab")))
        out.line(0)
    else:
        for value in range(32):
            inp.line(format(value, "05b").translate(str.maketrans("01", "ab")) + "a")
        out.line(32)


def _sorted_substring_index(inp, out, n: int, q: int, scenario: str) -> None:
    full = scenario == "alternating_middle_descent"
    alphabet = "abcdefghijklmnopqrstuvwxyz" if full else "abcdefghijklmnopqrstuvwxz"
    body_size = n if full else n - 1
    base, extra = divmod(body_size, len(alphabet))
    widths = [base + (index < extra) for index in range(len(alphabet))]
    body = "".join(letter * width for letter, width in zip(alphabet, widths))
    inp.line(n)
    inp.line(body if full else "z" + body)
    inp.line(q)
    position = sum(widths[:12]) + 1 if full else 1
    left = 1 if full else 2
    changed = "z" if full else "y"
    restored = "m" if full else "z"
    for turn in range(q // 2):
        altered = turn % 2 == 0
        inp.line(1, position, changed if altered else restored)
        inp.line(2, left, n)
        out.line("No" if altered else "Yes")


def _forbidden_sum_partitions(inp, out, n: int, scenario: str) -> None:
    if scenario == "distinct_prefixes_fibonacci_partitions":
        inp.line(n, 1)
        inp.line(*([1] * n))
        previous, current = 1, 0
        for _ in range(n - 1):
            previous, current = current, (previous + current) % 998244353
        out.line(current)
    else:
        inp.line(n, 0)
        inp.line(*([0] * n))
        out.line(0)


def _lattice_shape_transforms(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "dense_asymmetric_quarter_turn":
        source = {(1, 0), (1, 1), (n - 1, n - 3)}
        target = {(n - 1 - y, x) for x, y in source}
        out.line("Yes")
    else:
        source = {(1, 1), (2, 1)}
        target = {(1, 1), (2, 2)}
        out.line("No")
    for holes in (source, target):
        for y in range(n):
            inp.line("".join("." if (x, y) in holes else "#" for x in range(n)))


def _cyclic_xor_matches(inp, out, n: int, scenario: str) -> None:
    base, mask = 2**29 + 8, 200000003
    source = [base] * n
    target = [base ^ mask] * n
    inp.line(n)
    if scenario == "all_rotations_uniform":
        for shift in range(n):
            out.line(shift, mask)
    else:
        first, last = n // 3, 2 * n // 3
        source[first] ^= 1
        target[last] ^= 1
        out.line((first - last) % n, mask)
    inp.line(*source)
    inp.line(*target)


def _ordered_multiset_extrema(inp, out, q: int, scenario: str) -> None:
    inp.line(q)
    if scenario == "increasing_insert_then_erase_max":
        end = q // 2
        for i in range(1, end + 1):
            inp.line(1, i)
        for i in range(q - end):
            if i % 2 == 0:
                inp.line(3)
                out.line(end - 1)
            else:
                inp.line(2, end, 1)
                end -= 1
    else:
        high = 10**9
        inserted = q // 3
        inp.line(1, 0)
        for _ in range(inserted):
            inp.line(1, high)
        copies = inserted
        for i in range(q - inserted - 1):
            if i % 3 == 0:
                inp.line(3)
                out.line(high if copies else 0)
            elif i % 3 == 1:
                amount = 1 + i % 7
                inp.line(2, high, amount)
                copies = max(0, copies - amount)
            else:
                inp.line(2, 11, q)


def _chord_crossing_queries(inp, out, n: int, m: int, q: int, scenario: str) -> None:
    inp.line(n, m)
    every = scenario == "every_chord_crosses_swept_arcs"
    for i in range(1, m + 1):
        if every:
            inp.line(2 * i, 2 * (i + m))
        else:
            inp.line(4 * i - 2, 4 * i)
    inp.line(q)
    for index in range(q):
        i = 1 + index % m
        if every:
            inp.line(2 * i - 1, 2 * (i + m) - 1)
            out.line(m)
        elif index % 3 == 0:
            inp.line(1, 4 * i + 1)
            out.line(0)
        else:
            inp.line(4 * i - 1, 4 * m + 1)
            out.line(1)


def _dominance_counts(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "every_pair_equal":
        inp.line(*([10**9] * n))
        inp.line(*([0] * n))
        out.line(n * n)
    else:
        values = [37 * i % n for i in range(n)]
        inp.line(*values)
        inp.line(*(10**9 - value for value in values))
        out.line(n * (n + 1) // 2)


def _sparse_obstacle_grid_paths(inp, out, h: int, w: int, k: int, scenario: str) -> None:
    inp.line(h, w, k)
    if scenario == "incomparable_diagonal_barrier":
        for row in range(k, 0, -1):
            inp.line(row, k + 1 - row)
        out.line(0)
    else:
        from math import comb
        for col in range(k, 0, -1):
            inp.line(2, col)
        out.line(comb(h + w - 2 - k, h - 1) % 1000000007)


def _target_sum_subrectangles(inp, out, h: int, w: int, tail: int, scenario: str) -> None:
    inp.line(2)
    inp.line(h, w)
    if scenario == "distinct_band_prefixes":
        for _ in range(h):
            inp.line("#" * w)
        out.line(0)
    else:
        rows = [("#." * ((w + 1)//2))[:w], (".#" * ((w + 1)//2))[:w]]
        for i in range(h):
            inp.line(rows[i % 2])
        all_rectangles = h * (h + 1) // 2 * (w * (w + 1) // 2)
        odd_h = (h + 1) // 2 * ((h + 2) // 2)
        odd_w = (w + 1) // 2 * ((w + 2) // 2)
        out.line(all_rectangles - odd_h * odd_w)
    inp.line(1, tail)
    inp.line("#" * tail)
    out.line(0)


def _knuth_interval_dp(inp, out, n: int, scenario: str) -> None:
    def unit_cost(size):
        depth = (size - 1).bit_length()
        return size * (depth + 1) - (1 << depth)

    inp.line(n)
    if scenario == "equal_billion_weights":
        inp.line(*([10**9] * n))
        out.line(10**9 * unit_cost(n))
    else:
        values = [1] * n
        values[n // 3] = 10**9
        inp.line(*values)
        out.line(10**9 + n - 1 + unit_cost(n - 1))


def _binary_grid_squares(inp, out, h: int, w: int, holes: int, scenario: str) -> None:
    inp.line(h, w, holes)
    for cell in range(holes):
        i, j = divmod(cell, w)
        inp.line(i + 1, j + 1)
    blocked_rows, partial = divmod(holes, w)
    clear_rows = h - blocked_rows - bool(partial)
    answer = sum((clear_rows - side + 1) * (w - side + 1) for side in range(1, min(clear_rows, w) + 1))
    if partial:
        answer += sum(w - partial - side + 1 for side in range(1, min(clear_rows + 1, w - partial) + 1))
    out.line(answer)


def _run_length_queue(inp, out, q: int, count: int, scenario: str) -> None:
    inp.line(q)
    high = 10**9
    if scenario == "merged_billion_counts":
        for _ in range(q // 2):
            inp.line(1, count, high)
        for _ in range(q // 2, q):
            inp.line(2, count)
            out.line(count * high)
    else:
        n = q - 1
        for i in range(n):
            inp.line(1, count, high if i % 2 == 0 else 1)
        inp.line(2, n * count)
        out.line(count * (((n + 1) // 2) * high + n // 2))


def _repeated_subsequence(inp, out, n: int, m: int, copies: int, scenario: str) -> None:
    inp.line(copies)
    if scenario == "one_letter_full_capacity":
        inp.line("a" * n)
        inp.line("a" * m)
        out.line(n * copies // m)
    else:
        inp.line("ab" * (n // 2))
        inp.line("ab" * (m // 2))
        pair_capacity = n // 2 * copies // (m // 2)
        out.line((pair_capacity + 1) // 2)


def _balanced_pivot_subsequence(inp, out, n: int, q: int, scenario: str) -> None:
    inp.line(n, q)
    if scenario == "alternating_many_interior_crossings":
        periods = n // 3
        variation = min(173, max(1, periods // 3))
        inp.line("1/2" * periods + "1/2"[:n % 3])
        for i in range(q):
            first = i % variation
            last = periods - 13 * i % variation
            inp.line(3 * first + 1, 3 * last)
            out.line(2 * ((last - first + 1) // 2) + 1)
    else:
        side = n // 3
        separators = n - 2 * side
        inp.line("1" * side + "/" * separators + "2" * side)
        for i in range(q):
            if i % 4 == 0:
                pivot = side + 1 + 37 * i % separators
                inp.line(pivot, pivot)
                out.line(1)
            elif i % 4 == 1:
                inp.line(1 + i % side, side)
                out.line(0)
            else:
                left = 1 + 13 * i % side
                right_arm = 1 + 23 * i % side
                inp.line(left, side + separators + right_arm)
                out.line(2 * min(side - left + 1, right_arm) + 1)


def _three_symbol_frequency_counts(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "single_symbol_wide_prefix_map":
        inp.line("A" * n)
        out.line(0)
    else:
        left = n // 2
        right = n - left
        inp.line("A" * left + "B" * right)
        out.line(left * right - min(left, right))


def _pattern_range_occurrences(inp, out, n: int, q: int, m: int, scenario: str) -> None:
    dense = scenario == "dense_overlapping_matches"
    inp.line(q)
    inp.line("a" * n if dense else "b" * (n - m) + "a" * m)
    inp.line("a" * m)
    for i in range(q):
        if dense:
            start = 1 + (37 * i % (n - m + 1))
            length = m - (i & 1)
            inp.line(start, start + length - 1)
            out.line("Yes" if length == m else "No")
        elif i % 3 == 0:
            inp.line(1, n)
            out.line("Yes")
        elif i % 3 == 1:
            inp.line(1, n - 1)
            out.line("No")
        else:
            inp.line(n - m + 1, n)
            out.line("Yes")


def _exact_subarray_extrema(inp, out, n: int, scenario: str) -> None:
    if scenario == "equal_extrema_wide_count":
        inp.line(n, 1, 1)
        inp.line(*([1] * n))
        out.line(n * (n + 1) // 2)
    else:
        inp.line(n, 3, 1)
        inp.line(*(i % 4 + 1 for i in range(n)))
        out.line((n + 1) // 4)


def _longest_bitonic_subsequence(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    if scenario == "full_mountain_witness":
        inp.line(*(min(i + 1, n - i + 1) for i in range(n)))
        out.line(n)
    else:
        inp.line(*([n] * n))
        out.line(1)


def _poisson_binomial(inp, out, n: int, scenario: str) -> None:
    fair = scenario == "fair_coins_with_symmetric_tail"
    inp.line(n)
    inp.line(*(["0.50" if fair else "0.01"] * n))
    out.line("0.500000000000" if fair else "0.000000000000")


def _range_add_total_variation(inp, out, n: int, q: int, scenario: str) -> None:
    inp.line(n, q)
    value = 10**9
    if scenario == "repeated_interior_boundaries":
        assert n >= 3
        inp.line(*([0] * n))
        for i in range(1, q + 1):
            inp.line(2, n - 1, value)
            out.line(2 * i * value)
    else:
        inp.line(*(value if i & 1 else -value for i in range(n)))
        variation = 2 * value * (n - 1)
        for i in range(q):
            if i % 3 == 0:
                inp.line(1, n, value)
            elif i % 3 == 1:
                inp.line(1, 1, value)
                variation -= value
            else:
                inp.line(1, 1, -value)
                variation += value
            out.line(variation)


def _inversion_limited_partitions(inp, out, n: int, scenario: str) -> None:
    modulus = 10**9 + 7
    if scenario == "rising_all_cuts_valid":
        inp.line(n, 0)
        inp.line(*range(1, n + 1))
        out.line(pow(2, n - 1, modulus))
    else:
        inp.line(n, 1)
        inp.line(*range(n, 0, -1))
        previous, next_value = 1, 1
        for _ in range(n):
            previous, next_value = next_value, (previous + next_value) % modulus
        out.line(previous)


def _dictionary_segmenter(inp, out, n: int, scenario: str) -> None:
    if scenario == "ambiguous_prefixes_with_valid_split":
        groups, remaining = divmod(n, 11)
        assert remaining % 5 == 0
        inp.line("dreameraser" * groups + "dream" * (remaining // 5))
        out.line("YES")
    else:
        assert n % 5 == 0
        inp.line("erase" * (n // 5 - 1) + "erasz")
        out.line("NO")


def _deadline_job_selection(inp, out, n: int, scenario: str) -> None:
    inp.line(n)
    unit_reward = 10**9
    if scenario == "dense_time_states":
        for _ in range(n):
            inp.line(n, 1, unit_reward)
        out.line(n * unit_reward)
    else:
        assert n % 2 == 0
        short_jobs = n // 2
        for _ in range(short_jobs):
            inp.line(n, n - short_jobs + 1, 1)
            inp.line(short_jobs, 1, unit_reward)
        out.line(short_jobs * unit_reward)


def _periodic_pattern_runs(inp, out, n: int, m: int, scenario: str) -> None:
    if scenario == "coprime_unbounded_run":
        inp.line("a" * n)
        inp.line("a" * m)
        out.line(-1)
    else:
        assert m == 1
        inp.line("a" * (n - 1) + "b")
        inp.line("a")
        out.line(n - 1)


def _pattern_stamping(inp, out, n: int, scenario: str) -> None:
    stamp = "ABCDE"
    possible = scenario == "long_overwrite_chain"
    tail = "A" * (n - len(stamp)) + stamp
    inp.line(n, len(stamp))
    inp.line(tail if possible else "Z" + tail[1:])
    inp.line(stamp)
    out.line("Yes" if possible else "No")


def _subarray_frequency_windows_distinct(inp, out, n: int, k: int, scenario: str) -> None:
    inp.line(n, k)
    period = n if scenario == "all_distinct_wide_map" else k + 1
    inp.line(*(1 + i % period for i in range(n)))
    out.line(k)


def _subarray_sum_counts_equal(inp, out, n: int, scenario: str) -> None:
    value = 10**9
    if scenario == "signed_distinct_prefixes":
        inp.line(n, 1)
        inp.line(*(1 - value if i & 1 else value for i in range(n)))
        out.line(n - 1)
    else:
        length = max(1, n // 3)
        inp.line(n, value * length)
        inp.line(*([value] * n))
        out.line(n - length + 1)


def _clamped_translation(inp, out, n: int, q: int, scenario: str) -> None:
    unbounded = scenario == "wide_unbounded_additions"
    limit = 10**9
    inp.line(n)
    start = 0
    if not unbounded:
        assert n % 2 == 0
        inp.line(-100, 2)
        inp.line(100, 3)
        start = 2
    for i in range(start, n):
        inp.line(limit if unbounded or not (i & 1) else -limit, 1)

    inp.line(q)
    values = (-limit, -100, -10, 0, 100, limit)
    queries = [values[i % len(values)] for i in range(q)]
    inp.line(*queries)
    for value in queries:
        out.line(value + n * limit if unbounded else min(max(value, -100), 100))


def _weighted_interval_stabbing(inp, out, n: int, scenario: str) -> None:
    inp.line(n, n)
    weight = 10**9
    if scenario == "positive_suffixes_same_end":
        for i in range(1, n + 1):
            inp.line(i, n, weight)
        out.line(n * weight)
    else:
        for i in range(1, n + 1):
            inp.line(i, i, weight if i & 1 else -weight)
        out.line(((n + 1) // 2) * weight)


def _maximum_allowed_pair(inp, out, n: int, m: int, p: int, scenario: str) -> None:
    inp.line(n, m, p)
    if scenario == "dense_high_score_block":
        from math import isqrt
        width = isqrt(p)
        inp.line(*range(n, 0, -1))
        inp.line(*range(m, 0, -1))
        for index in range(p):
            i, j = divmod(index, width)
            inp.line(i + 1, j + 1)
        out.line(n + m - width)
    else:
        inp.line(*(n - 37 * i % n for i in range(n)))
        inp.line(*(m - 37 * j % m for j in range(m)))
        for j in range(1, p + 1):
            inp.line(1, j)
        out.line(n + m - 1)


def _monotone_value_shift(inp, out, n: int, q: int, scenario: str) -> None:
    wide = scenario == "repeated_wide_updates"
    inp.line(n)
    for i in range(1, n + 1):
        if wide:
            inp.line(1, 500000)
        else:
            inp.line(i, i)
    inp.line(q)
    for i in range(q):
        rating = 1 + (1009 * i % (500000 if wide else 2 * n + 7))
        inp.line(rating)
        out.line(min(rating + n, 500001) if wide else max(rating, n + 1))


def _nearest_value_index(inp, out, n: int, q: int, scenario: str) -> None:
    inp.line(n, q)
    if scenario == "dense_two_sided_rank_search":
        inp.line(*range(n, 0, -1))
        for turn in range(q):
            center = 1 + (347 * turn + 13) % n
            rank = 1 + (431 * turn + 17) % n
            inp.line(center, rank)
            short_side = min(center - 1, n - center)
            out.line(rank // 2 if rank <= 2 * short_side + 1 else rank - short_side - 1)
    else:
        endpoint = 10**8
        inp.line(*([endpoint, -endpoint] * (n // 2)))
        centers = (-endpoint, -endpoint + 1, 0, endpoint - 1, endpoint)
        for turn in range(q):
            center = centers[turn % len(centers)]
            rank = 1 + (2897 * turn + 19) % n
            inp.line(center, rank)
            near, far = endpoint - abs(center), endpoint + abs(center)
            out.line(near if rank <= n // 2 else far)


def _grid_patterns(inp, out, height: int, width: int, rows: int, columns: int, scenario: str) -> None:
    inp.line(height, width)
    if scenario == "dense_overlapping_occurrences":
        for _ in range(height):
            inp.line("a" * width)
        inp.line(rows, columns)
        for _ in range(rows):
            inp.line("a" * columns)
        for y in range(height - rows + 1):
            for x in range(width - columns + 1):
                out.line(y, x)
        return

    alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    text = [alphabet[i // len(alphabet)] + alphabet[i % len(alphabet)] + "a" * (width - 2)
            for i in range(height)]
    for row in text:
        inp.line(row)
    inp.line(rows, columns)
    for row in text:
        inp.line(row)
    out.line(0, 0)


def _coordinate_compression(inp, out, n, scenario):
    inp.line(10**9, 10**9, n)
    if scenario == "same_column_decreasing_rows":
        for i in range(n, 0, -1):
            inp.line(i * 1000, 10**9)
            out.line(i, 1)
        return
    for i in range(n):
        row, column = divmod(i * 17 % n, 10)
        inp.line(1 + row * 1000, 2 + column * 10007)
        out.line(row + 1, column + 1)


def _edit_alignment(inp, out, n, scenario):
    inp.line(n, n)
    if scenario == "disjoint_alphabets":
        inp.line(*([1] * n))
        inp.line(*([2] * n))
        out.line(n)
    else:
        inp.line(*range(1, n + 1))
        inp.line(*range(2, n + 1), 1)
        out.line(2)


_GENERATORS = {
    "coordinate_compression": _coordinate_compression,
    "point_set_mex": _point_set_mex,
    "sliding_window_extrema": _sliding_window_extrema,
    "pair_sum_order_handshake": _pair_sum_handshake,
    "string_radix_order_alphabet": _string_radix_order,
    "dynamic_prefix_filter": _dynamic_prefix_filter,
    "prefix_bitmap_table": _prefix_bitmap_table,
    "interval_map_paint": _interval_map_paint,
    "subarray_median_order": _subarray_median_order,
    "range_equal_runs": _range_equal_runs,
    "lexicographic_subsequence": _lexicographic_subsequence,
    "range_nondecreasing_subsequence": _range_nondecreasing_subsequence,
    "cyclic_rotation_pairs": _cyclic_rotation_pairs,
    "cyclic_rotation_inversions": _cyclic_rotation_inversions,
    "cyclic_rotation_inversions_adjacent_swaps": _cyclic_rotation_inversions_adjacent_swaps,
    "range_add_threshold_min": _range_add_threshold_min,
    "bounded_zigzag_subsequence": _bounded_zigzag_subsequence,
    "pattern_erasure": _pattern_erasure,
    "pattern_avoidance": _pattern_avoidance,
    "sorted_substring_index": _sorted_substring_index,
    "forbidden_sum_partitions": _forbidden_sum_partitions,
    "lattice_shape_transforms": _lattice_shape_transforms,
    "cyclic_xor_matches": _cyclic_xor_matches,
    "ordered_multiset_extrema": _ordered_multiset_extrema,
    "chord_crossing_queries": _chord_crossing_queries,
    "dominance_counts": _dominance_counts,
    "sparse_obstacle_grid_paths": _sparse_obstacle_grid_paths,
    "target_sum_subrectangles": _target_sum_subrectangles,
    "knuth_interval_dp": _knuth_interval_dp,
    "binary_grid_squares": _binary_grid_squares,
    "run_length_queue": _run_length_queue,
    "repeated_subsequence": _repeated_subsequence,
    "balanced_pivot_subsequence": _balanced_pivot_subsequence,
    "three_symbol_frequency_counts": _three_symbol_frequency_counts,
    "pattern_range_occurrences": _pattern_range_occurrences,
    "exact_subarray_extrema": _exact_subarray_extrema,
    "longest_bitonic_subsequence": _longest_bitonic_subsequence,
    "poisson_binomial": _poisson_binomial,
    "range_add_total_variation": _range_add_total_variation,
    "inversion_limited_partitions": _inversion_limited_partitions,
    "dictionary_segmenter": _dictionary_segmenter,
    "deadline_job_selection": _deadline_job_selection,
    "periodic_pattern_runs": _periodic_pattern_runs,
    "pattern_stamping": _pattern_stamping,
    "subarray_frequency_windows_distinct": _subarray_frequency_windows_distinct,
    "subarray_sum_counts_equal": _subarray_sum_counts_equal,
    "clamped_translation": _clamped_translation,
    "weighted_interval_stabbing": _weighted_interval_stabbing,
    "maximum_allowed_pair": _maximum_allowed_pair,
    "monotone_value_shift": _monotone_value_shift,
    "nearest_value_index": _nearest_value_index,
    "grid_pattern_matching_all": _grid_patterns,
    "edit_alignment_sequence_matching": _edit_alignment,
    "adjacent_swap_distance": _adjacent_swap,
    "interval_lis": _interval_lis,
    "interval_schedule_max_weight": _intervals,
    "lis_count": _lis_count,
    "modular_pair_sum_order": _modular_pair_sum,
    "monge_shortest_paths_baseball": _monge_baseball,
    "pair_product_order": _pair_product,
    "parallel_binary_search_stamp_rally": _stamp_rally,
    "subarray_mex_histogram": _mex_histogram,
    "rollback_lis_tree": _rollback_lis,
    "state_cycle_sequence_sum": _state_cycle,
    "subset_sum_meet_in_middle_max": _subset_sum,
    "weighted_monotone_matching_flowers": _weighted_monotone_matching,
    "bracket_sequence_range": _bracket_sequence,
    "concatenated_pattern_count": _concatenated,
    "cyclic_rotation": _cyclic_rotation,
    "distinct_subsequence_order": _distinct_subsequence_order,
    "dynamic_palindrome": _dynamic_palindrome,
    "dynamic_substring_matching": _dynamic_substring,
    "hamming_distance_matching_binary_min": _hamming_distance,
    "offline_prefix_lcp_sum": _offline_prefix_lcp_sum,
    "run_length_z_sum": _run_length_z,
    "shortest_absent_subsequence": _shortest_absent_subsequence,
    "shortest_common_superstring": _shortest_superstring,
    "string_periods_best_representation": _string_periods,
    "substring_comparison_common_prefix": _substring_comparison,
    "suffix_tree_nonoverlapping_repeat": _suffix_tree,
}


def generate(plan: dict, inp, out) -> None:
    _GENERATORS[plan["verify_id"]](inp, out, **plan["dimensions"], scenario=plan["scenario"])
