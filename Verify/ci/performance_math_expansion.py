# SPDX-License-Identifier: CC0-1.0
"""Deterministic performance cases for non-LC math libraries."""

from array import array
from bisect import bisect_right
from decimal import Decimal
from math import comb, factorial, gcd, isqrt

LIBRARY_VERIFY_IDS = {
    "DataStructure/Subset_Xor_Square_Range.cpp": "subset_xor_square_range_yuki3792",
    "Algorithm/Josephus.cpp": "josephus_survivor_aoj0085",
    "Algorithm/Dyadic_Interval_Cover.cpp": "dyadic_interval_cover_abc349_d",
    "Math/Divisibility_Chains.cpp": "divisibility_chain_ends_arc116_c",
    "Math/Pairwise_Coprime_Progression.cpp": "pairwise_coprime_progression_yuki3832",
    "Graph/Tree_Subset_Medians.cpp": "tree_subset_medians_yuki3834",
    "Math/Weighted_Taylor_Shifts.cpp": "weighted_taylor_shifts_yuki3793",
    "Geometry/Dice_Orientation.cpp": "dice_orientation_yuki1572",
    "Algorithm/Line_Chip_Firing.cpp": "line_chip_firing_yuki3801",
    "Graph/Hub_Sparse_Distances.cpp": "hub_sparse_distances_yuki3810",
    "Math/Histogram_Rook_Cover.cpp": "histogram_rook_cover_agc041_f",
    "Math/Ferrers_Rook.cpp": "ferrers_rook_abc272_ex",
    "Math/Xor_Convolution_Without_Division.cpp": "xor_convolution_without_division_yuki1901",
    "Math/Krawtchouk_Transform.cpp": "krawtchouk_transform_arc133_f",
    "Math/Polynomial_Differential_Operator.cpp": "polynomial_differential_operator_yuki2583",
    "Math/Reeds_Sloane.cpp": "reeds_sloane_abc129_f",
    "Graph/Short_Simple_Path.cpp": "short_simple_path_yuki408",
    "Graph/Round_Robin_Schedule.cpp": "round_robin_schedule_arc208_d",
    "Math/Exponential_Coefficient_Transform.cpp": "exponential_coefficient_transform_agc076_d",
    "Graph/Degree_Sequence.cpp": "degree_sequence_yahoo2018_e",
    "Math/Polynomial_Recurrence_Fit.cpp": "polynomial_recurrence_fit_abc222_h",
    "Math/Polynomial_Moments.cpp": "polynomial_moments_yuki1875",
    "Math/Prefix_Dominant_Compositions.cpp": "prefix_dominant_compositions_yuki3480",
    "Math/Young_Tableau_Count.cpp": "domino_tableau_yuki2149",
    "Math/Interval_Matrix_Rank.cpp": "interval_matrix_rank_abc399_g",
    "Algorithm/Multiset_Permutation.cpp": "multiset_permutation_abc202_d",
    "Algorithm/Subarray_Aggregate_Runs.cpp": "subarray_aggregate_runs_yuki1036",
    "Math/Polynomial_Recurrence.cpp": "polynomial_recurrence_abc222_h",
    "Math/Polynomial_Matrix_Product.cpp": "polynomial_matrix_product_arc174_c",
    "Algorithm/Adjacent_Swap_Parity.cpp": "adjacent_swap_parity_arc214_e",
    "Math/Matrix_Pencil_Determinant.cpp": "matrix_pencil_determinant_yuki1303",
    "Math/Tridiagonal_Matrix_Power.cpp": "tridiagonal_matrix_power_arc228_e",
    "Math/Modular_Shift_Sums.cpp": "modular_shift_sums_abc483_f",
    "Math/Modular_Shift_Mex.cpp": "modular_shift_mex_abc465_g",
    "Math/Modular_Shift_Minimum.cpp": "modular_shift_minimum_arc182_e",
    "Graph/LCM_Spanning_Tree.cpp": "lcm_spanning_tree_yuki3827",
    "Math/Permutation_Comparison_Count.cpp": "permutation_comparison_count_dp_t",
    "Algorithm/Alien_Trick.cpp": "alien_trick_abc462_f",
    "Algorithm/Subtraction_Game.cpp": "subtraction_game_dp_k",
    "Math/Xor_Two_Independent_Sets.cpp": "xor_two_independent_sets_game_x",
    "Math/Linear_Matroid_Parity.cpp": "linear_matroid_parity_abc482_g",
    "Geometry/Disk_Lattice_Points.cpp": "disk_lattice_points_abc191_d",
    "Graph/Hamiltonian_Route.cpp": "hamiltonian_route_abc180_e",
    "Algorithm/Nonadjacent_Selection.cpp": "nonadjacent_selection_abc218_h",
    "Math/Matrix_Permanent.cpp": "matrix_permanent_dp_o",
    "Algorithm/Exact_Cover.cpp": "exact_cover_abc345_d",
    "Algorithm/Maximum_Subarray.cpp": "maximum_submatrix_awc0097_d",
    "Algorithm/Digit_DP.cpp": "digit_dp_dp_s",
    "Algorithm/Sparse_Knapsack.cpp": "sparse_knapsack_yuki626",
    "Math/Bounded_Compositions.cpp": "bounded_composition_counts_dp_m",
    "Algorithm/Subarray_Extrema.cpp": "subarray_extrema_agc005_b",
    "Algorithm/Largest_Rectangle.cpp": "largest_histogram_rectangle_abc189_c",
    "Algorithm/Knapsack_By_Value.cpp": "knapsack_by_value_dp_e",
    "Algorithm/Top_K_Cartesian_Sums.cpp": "top_k_cartesian_sums_abc123_d",
    "Algorithm/Optimal_Adjacent_Merge.cpp": "optimal_adjacent_merge_dp_n",
    "Math/Floor_Sum_Moments.cpp": "floor_sum_moments_abc402_g",
    "Math/Floor_Path_Product.cpp": "floor_path_product_abc429_g",
    "Math/Relaxed_Convolution.cpp": "relaxed_convolution_abc315_ex",
    "Math/Mixed_Radix.cpp": "permutation_rank_decimal_yuki696",
}
PROFILES = {
    "subset_xor_square_range_yuki3792": (
        ("maximum_mixed_updates_and_aggregate", {"n": 500000, "q": 500000}, {"n": 12, "q": 19},
         "Half the queries change originally zero positions between zero and the highest 30-bit value; wide queries exercise repeated columns."),
        ("maximum_unaligned_queries", {"n": 500000, "q": 500000}, {"n": 12, "q": 19},
         "Unaligned wide intervals have all 30 independent bits; every 32nd operation is a singleton bonus check.")),
    "josephus_survivor_aoj0085": (
        ("maximum_linear_fallback", {"n": 999, "t": 50}, {"n": 25, "t": 7},
         "Maximum dataset count and near-maximum step to exercise the linear fallback."),
        ("maximum_small_steps", {"n": 999, "t": 50}, {"n": 25, "t": 7},
         "Full-size rings alternating the step-two formula and batched recurrence.")),
    "dyadic_interval_cover_abc349_d": (
        ("maximum_cover_block_count", {"h": 60}, {"h": 6},
         "Both ends are one unit inside the largest dyadic interval, giving 2(h-1) output blocks."),
        ("maximum_aligned_interval", {"h": 60}, {"h": 7},
         "The complete maximum interval is one aligned output block.")),
    "divisibility_chain_ends_arc116_c": (
        ("maximum_length_and_end_value", {"n": 200000, "m": 200000}, {"n": 5, "m": 38},
         "Both limits simultaneously, matching the judge's published maximum sample."),
        ("maximum_end_value_length_two", {"n": 2, "m": 200000}, {"n": 2, "m": 43},
         "Count all divisible pairs up to the maximum value, independently by harmonic summation.")),
    "pairwise_coprime_progression_yuki3832": (
        ("maximum_first_missing_prime_boundary", {"t": 100000}, {"t": 8},
         "All primes through 47 divide the step. Alternating lengths straddle the sharp threshold at 105."),
        ("maximum_length_many_steps", {"t": 100000}, {"t": 8},
         "Each case reaches the maximum length and varies the large step. No 64-bit step allows this many pairwise coprime terms.")),
    "tree_subset_medians_yuki3834": (
        ("maximum_star_pair_medians", {"n": 200000, "k": 2}, {"n": 9, "k": 2},
         "Maximum degree: between two selected leaves there are three medians, or two if one is the center."),
        ("maximum_path_balanced_halves", {"n": 200000, "k": 100000}, {"n": 10, "k": 6},
         "All cut sizes on a maximum-depth path. Counting balanced cuts by Vandermonde gives the exact average.")),
    "weighted_taylor_shifts_yuki3793": (
        ("maximum_full_order_finite_difference", {"n": 100000, "m": 100000}, {"n": 12, "m": 12},
         "Maximum coefficient and shift counts; the Nth finite difference of x^N leaves only the factorial."),
        ("maximum_dense_high_order_difference", {"n": 100000, "m": 100000}, {"n": 14, "m": 14},
         "Dense maximum polynomial and dense signed shifts. A finite difference of order N-1 leaves exactly one linear polynomial.")),
    "dice_orientation_yuki1572": (
        ("maximum_open_corner_to_corner", {"h": 1000, "w": 1000}, {"h": 8, "w": 8},
         "Full grid, with Manhattan distance attained by rolls in groups of four and an alternating six-roll loop."),
        ("maximum_open_region_isolated_goal", {"h": 1000, "w": 1000}, {"h": 8, "w": 8},
         "An unreachable corner goal leaves almost the whole maximum grid and its orientations to be explored.")),
    "line_chip_firing_yuki3801": (
        ("maximum_single_pile", {"n": 200000}, {"n": 17},
         "Every chip starts at the maximum coordinate. The final positions are symmetric around it."),
        ("maximum_shuffled_adjacent_piles", {"n": 200000}, {"n": 16},
         "The input has two chips at each adjacent center, shuffled. They fill one final interval with a fixed mean.")),
    "hub_sparse_distances_yuki3810": (
        ("maximum_zero_contractions", {"n": 200000, "m": 200000, "q": 200000}, {"n": 10, "m": 10, "q": 19},
         "Two long zero-cost chains, including late zero edges. Their contracted gate costs are 3 and 5."),
        ("maximum_positive_shortcut_core", {"n": 200000, "m": 200000, "q": 200000}, {"n": 12, "m": 12, "q": 23},
         "Sixty disjoint shortcuts give the maximum number of endpoints. Later weights exceed the warp bound.")),
    "histogram_rook_cover_agc041_f": (
        ("maximum_full_histogram", {"n": 400}, {"n": 4},
         "A full square has the maximum number of cells. Its cover count is the union of no empty rows and no empty columns."),
        ("maximum_alternating_histogram", {"n": 400}, {"n": 4},
         "Maximum height and width with separated single-column runs above the base row. Each tall column requires a rook, and the base row also needs one.")),
    "ferrers_rook_abc272_ex": (
        ("maximum_sparse_and_full_columns", {"n": 200000}, {"n": 7},
         "Maximum square board with interleaved first-row and full columns. The first row adds a single Bernoulli trial."),
        ("maximum_dense_two_height_columns", {"n": 200000}, {"n": 8},
         "Maximum dense board with just two extra rows. Membership of the two rows in the tall columns is hypergeometric.")),
    "krawtchouk_transform_arc133_f": (
        ("maximum_dense_one_flip", {"n": 100000, "k": 1}, {"n": 12, "k": 1},
         "Maximum state count with dense initial distribution, confirmed by one direct transition."),
        ("maximum_states_zero_eigenvalue", {"n": 100000, "k": 998244352}, {"n": 10, "k": 998244352},
         "At a positive multiple of the field group order, all nonzero modes remain and the zero-eigenvalue mode vanishes.")),
    "polynomial_differential_operator_yuki2583": (
        ("maximum_unshifted_invariant_space", {"n": 100000, "m": 100000, "degree": 2, "constant": 0},
         {"n": 12, "m": 14, "degree": 2, "constant": 0},
         "Maximum input and precision, with parity alternating under the operator on polynomials of degree at most two."),
        ("maximum_shifted_invariant_space", {"n": 100000, "m": 100000, "degree": 17, "constant": 7},
         {"n": 11, "m": 14, "degree": 17, "constant": 7},
         "Both limits, with a nonzero diagonal and a larger invariant polynomial space.")),
    "reeds_sloane_abc129_f": (
        ("simultaneous_count_and_modulus_maxima",
         {"l": 10**18 - 1, "a": 1, "b": 1, "m": 10**9}, {"l": 25, "a": 1, "b": 1, "m": 10**9},
         "The arithmetic progression reaches the largest final term and modulus while crossing all 18 digit widths."),
        ("mixed_prime_powers_coprime_to_base",
         {"l": 333333333333333330, "a": 9, "b": 3, "m": 999999999}, {"l": 57, "a": 9, "b": 3, "m": 999999999},
         "Many digits and a composite modulus coprime to 10; large powers must be evaluated beyond the six sampled terms.")),
    "short_simple_path_yuki408": (
        ("maximum_dense_bipartite_no_cycle", {"n": 20000, "l": 100, "r": 500, "extra": 0, "m": 50000},
         {"n": 14, "l": 4, "r": 6, "extra": 0, "m": 24},
         "Both limits attained, with many short walks through the root but every cycle has even length."),
        ("maximum_sole_rooted_pentagon", {"n": 20000, "l": 100, "r": 499, "extra": 95, "m": 50000},
         {"n": 18, "l": 4, "r": 5, "extra": 3, "m": 28},
         "A unique pentagon appended at the last vertex labels; the rest is bipartite and touches it only at the root.")),
    "round_robin_schedule_arc208_d": (
        ("maximum_all_pairs_prescribed", {"n": 500, "pairs": 250}, {"n": 10, "pairs": 5},
         "An even-size Latin square with every required match in the first round."),
        ("maximum_mixed_pairs_and_byes", {"n": 500, "pairs": 123}, {"n": 12, "pairs": 2},
         "An even-size Latin square requiring both two-player matches and individual byes in its first color.")),
    "xor_convolution_without_division_yuki1901": (
        ("maximum_dense_by_low_three_bits", {"n": 17}, {"n": 4},
         "Both inputs contain polynomials up to degree 31. The right input is periodic in the three low bits."),
        ("maximum_last_index_polynomial_impulse", {"n": 17}, {"n": 3},
         "The right input consists of x^31 at the all-ones index. Output coefficients can reach degree 62.")),
    "exponential_coefficient_transform_agc076_d": (
        ("maximum_boundary_red_offsets", {"n": 250000, "m": 250000, "offset": 0},
         {"n": 17, "m": 20, "offset": 0},
         "Both judge dimensions are maximal. Red values at integer boundaries reduce to a parking function."),
        ("maximum_interior_red_offsets", {"n": 250000, "m": 250000, "offset": 73919437},
         {"n": 19, "m": 23, "offset": 73919437},
         "Both judge dimensions are maximal and all red values lie strictly inside the bins.")),
    "degree_sequence_yahoo2018_e": (
        ("complete_graph_degree_sum_over_int32", {"n": 100000}, {"n": 24},
         "Complete graph at maximum vertex count. The sum n(n-1) exceeds signed 32-bit."),
        ("one_missing_degree_to_complete_graph", {"n": 100000}, {"n": 25},
         "Only the first degree is lowered by one. The original sum is odd and increasing that first degree restores the complete graph.")),
    "polynomial_moments_yuki1875": (
        ("distinct_rates_with_canceling_multiplicities", {"n": 100000, "moves": 100000, "residual": 0},
         {"n": 22, "moves": 11, "residual": 0},
         "Maximum numbers of types and moves. Many shared rates occur in pairs whose positive multiplicities sum to the modulus."),
        ("maximum_input_with_nonconstant_expansion", {"n": 100000, "moves": 100000, "residual": 10},
         {"n": 24, "moves": 12, "residual": 4},
         "Pairs cancel to the requested order. Ten residual types give a nonconstant exponential polynomial checked by direct powers.")),
    "prefix_dominant_compositions_yuki3480": (
        ("maximum_coprime_totals", {"n": 500000, "p": 500000, "q": 499999}, {"n": 12, "p": 7, "q": 9},
         "The largest neighboring totals and maximal output length. Factorial tables independently compute each binomial coefficient."),
        ("equal_unit_totals", {"n": 500000, "p": 1, "q": 1}, {"n": 40, "p": 1, "q": 1},
         "There is one position of mass one in each sequence. Choosing their ordered locations gives triangular numbers.")),
    "domino_tableau_yuki2149": (
        ("maximum_rectangular_shape", {"height": 1000, "width": 1000}, {"height": 4, "width": 6},
         "Every input width and the height are maximal. Quotient hook denominators are independently evaluated as rectangular factorial ratios."),
        ("large_even_area_with_nonempty_core", {"height": 1000, "width": 1000}, {"height": 4, "width": 4},
         "An almost full rectangle of even area cannot be removed because two rows change the same abacus parity.")),
    "interval_matrix_rank_abc399_g": (
        ("maximum_edges_with_one_full_rank_color", {"n": 150, "m": 500000, "c": 45}, {"n": 8, "m": 100, "c": 4},
         "A complete color class has capacity n-1. Every other color has capacity one and their total capacity is too small."),
        ("simultaneous_vertex_edge_and_color_limits", {"n": 150, "m": 500000, "c": 300}, {"n": 6, "m": 107, "c": 9},
         "All three size limits. Each color has a shared spanning path and many extra edges, while its capacity is one.")),
    "multiset_permutation_abc202_d": (
        ("first_word_starting_with_b", {"a": 30, "b": 30}, {"a": 6, "b": 6},
         "Both multiplicities are maximal. The rank is the first after the entire a-prefix block."),
        ("alternating_word_far_from_edges", {"a": 30, "b": 29}, {"a": 6, "b": 5},
         "A nearly maximum-length answer repeatedly changes which occurrence block contains the rank.")),
    "subarray_aggregate_runs_yuki1036": (
        ("many_distinct_gcds", {"n": 500000, "bits": 59}, {"n": 93, "bits": 9},
         "Increasing powers of two create many distinct suffix gcds. Each 1 resets the cycle."),
        ("consecutive_fibonacci_values", {"n": 500000}, {"n": 87},
         "Alternates two large consecutive Fibonacci values. Each interval of length at least two has gcd 1.")),
    "polynomial_recurrence_abc222_h": (
        ("maximum_tree_degree", {"n": 10000000}, {"n": 76},
         "Maximum target index, cross-checked by iterating the coefficient recurrence instead of multiplying polynomial blocks."),
        ("just_before_recurrence_block_doubling", {"n": 1048577}, {"n": 103},
         "The update count is one below a block-doubling boundary, forcing a nearly full direct tail.")),
    "polynomial_matrix_product_arc174_c": (
        ("largest_quadratic_transitions", {"n": 1000000}, {"n": 31},
         "Maximum coupon count. An independent linear recurrence for total expectation and turn difference gives both outputs."),
        ("just_before_block_doubling_boundary", {"n": 524287}, {"n": 127},
         "One below the next power-of-two block boundary, exercising the longest remainder.")),
    "adjacent_swap_parity_arc214_e": (
        ("one_far_apart_equal_pair", {"n": 300000, "k": 2}, {"n": 7, "k": 2},
         "One adjacent unequal swap is odd. The only equal pair surrounds all other values, making the even correction 2n-3."),
        ("many_duplicate_rectangles", {"n": 300000, "k": 999999999}, {"n": 8, "k": 7},
         "Two copies of the same reversed block give m*(m-1) stable inversions and m empty rectangles for parity correction.")),
    "matrix_pencil_determinant_yuki1303": (
        ("complete_graph_constant_matrix", {"n": 100}, {"n": 5},
         "All possible roads exist. The linear matrix is zero, and the exact count follows Cayley's formula."),
        ("path_with_dense_additions", {"n": 100}, {"n": 6},
         "For each missing shortcut, replacing any existing edge on its unique path gives a distinct spanning tree.")),
    "tridiagonal_matrix_power_arc228_e": (
        ("largest_judge_sample", {"n": 10**9, "m": 125000}, {"n": 6, "m": 2},
         "The official third sample exercises both extreme exponent and maximum tridiagonal side."),
        ("unconstrained_pairs_with_maximum_width", {"n": 125000, "m": 125000}, {"n": 6, "m": 6},
         "No position has an upper bound on its values. The answer is n! times the derangement count, with the largest tail size.")),
    "modular_shift_sums_abc483_f": (
        ("dense_moduli_many_periods", {"n": 500000, "upper": 500000, "queries": 200000},
         {"n": 68, "upper": 111, "queries": 29},
         "Fill all offsets once plus a duplicate at zero. The balanced part is constant, and the sparse duplicate drops yield exact answers."),
        ("large_moduli_sparse_wraps", {"n": 500000, "upper": 500000, "queries": 200000},
         {"n": 24, "upper": 32, "queries": 27},
         "Each large modulus has offsets zero and modulus minus one. The wrap corrections are arithmetic prefixes.")),
    "modular_shift_mex_abc465_g": (
        ("sparse_isolated_residue_updates", {"n": 200000, "m": 10**9, "c": 618033989, "k": 10**9, "q": 200000},
         {"n": 15, "m": 73, "c": 29, "k": 73, "q": 30},
         "Every occupied residue is isolated. Coprime shifts visit each residue once, so all updates have total n."),
        ("toggle_full_and_one_missing", {"n": 200000, "m": 200000, "c": 123457, "k": 10**9, "q": 200000},
         {"n": 11, "m": 11, "c": 2, "k": 22, "q": 22},
         "Alternately delete and restore one residue via a duplicate. Full periods have total m*k or k*(m-1)/2.")),
    "modular_shift_minimum_arc182_e": (
        ("coprime_full_circle", {"n": 100000, "m": 10**9, "c": 618033989, "k": 10**9, "offset": 0},
         {"n": 8, "m": 112, "c": 71, "k": 224, "offset": 0},
         "A coprime high-Euclidean-depth step visits every residue. Sum one descending interval per cyclic gap."),
        ("periodic_nonuniform_gaps", {"n": 100000, "m": 10**9, "c": 2500, "k": 10**9, "offset": 2500},
         {"n": 8, "m": 112, "c": 2, "k": 112, "offset": 2},
         "Alternating gaps exceed a single progression step by different amounts. Sum the grid residues over complete periods.")),
    "lcm_spanning_tree_yuki3827": (
        ("dividing_minimum_at_maximum", {"n": 200000, "upper": 200000}, {"n": 48, "upper": 80},
         "The smallest label divides every other label. The exact minimum is the sum excluding one smallest label."),
        ("two_coprime_centers_at_maximum", {"n": 200000, "upper": 200000}, {"n": 48, "upper": 60},
         "Every label attaches to center 2 or 3 with its own weight, and linking center 3 into the root at 2 costs three extra.")),
    "permutation_comparison_count_dp_t": (
        ("alternating_comparisons_at_maximum", {"n": 3000}, {"n": 31},
         "Alternating comparisons densely exercise the convolution splits, with the Entringer triangle as oracle."),
        ("one_descent_at_maximum", {"n": 3000}, {"n": 22},
         "An isolated descent splits sorted runs of equal lengths. Exclude the totally increasing permutation.")),
    "alien_trick_abc462_f": (
        ("dense_equal_edit_ties", {"n": 300000, "extra": 10}, {"n": 30, "extra": 5},
         "Each occurrence needs three edits. The target sits inside a wide tie at penalty -3."),
        ("forced_full_realignment", {"n": 300000, "extra": 1}, {"n": 15, "extra": 1},
         "Adding one occurrence requires a complete ABC tiling. The shifted input differs at every position.")),
    "subtraction_game_dp_k": (
        ("dense_moves_at_maximum", {"moves": 100, "stones": 100000, "stride": 1},
         {"moves": 7, "stones": 56, "stride": 1},
         "All removals from 1 through K are allowed; the Grundy value is size modulo K+1."),
        ("scaled_moves_at_maximum", {"moves": 100, "stones": 100000, "stride": 99},
         {"moves": 9, "stones": 72, "stride": 3},
         "The remainder modulo the stride is invariant. Divide out the stride and use the dense-move Grundy formula.")),
    "xor_two_independent_sets_game_x": (
        ("two_complete_bases", {"n": 200, "bits": 60, "t": 10}, {"n": 8, "bits": 3, "t": 2},
         "Two copies of an invertible triangular basis saturate the union rank even after adding the target."),
        ("target_outside_span", {"n": 200, "bits": 59, "t": 10}, {"n": 9, "bits": 4, "t": 2},
         "Every input vector lacks the target's highest bit, so no turn sequence can form the target.")),
    "linear_matroid_parity_abc482_g": (
        ("complete_graph_unit_capacity", {"n": 500, "capacity": 1}, {"n": 6, "capacity": 1},
         "Dense graph and maximum field dimension; pairing arbitrary vertices achieves the capacity bound."),
        ("unbalanced_bipartite_capacity_two", {"n": 250, "left": 120, "capacity": 2},
         {"n": 7, "left": 3, "capacity": 2},
         "The smaller side limits the answer; two disjoint cyclic matchings saturate it. Both trials are required.")),
    "disk_lattice_points_abc191_d": (
        ("integer_center_max_radius", {"x": -1000000000, "y": 1000000000, "radius": 1000000000},
         {"x": -70000, "y": 30000, "radius": 90000},
         "A translated maximal disk contains exact tangencies and an answer wider than 32 bits."),
        ("fractional_center_max_radius", {"x": -999999999, "y": 5000, "radius": 1000000000},
         {"x": -293657, "y": 255000, "radius": 80029},
         "Maximum-radius shifted disk exercises fractional grid intersections with signed decimal input.")),
    "hamiltonian_route_abc180_e": (
        ("collinear_cities", {"n": 17, "span": 1000000}, {"n": 7, "span": 103},
         "A tour must cross the full x span in both directions; monotone visit order attains the bound."),
        ("inclined_diagonal_cities", {"n": 17, "span": 1000000}, {"n": 8, "span": 709},
         "A monotone diagonal tour simultaneously attains each horizontal variation and the vertical ascent bound.")),
    "nonadjacent_selection_abc218_h": (
        ("equal_edges_balanced_colors", {"n": 200000, "weight": 1000000000}, {"n": 12, "weight": 47},
         "Alternating colors cross every edge and maximize the aggregate of maximum-sized gains."),
        ("one_weak_first_edge", {"n": 200000, "weight": 1000000000}, {"n": 14, "weight": 307},
         "Unequal color counts lose at least one edge. Alternating odd segments can lose only the global minimum.")),
    "matrix_permanent_dp_o": (
        ("complete_bipartite", {"n": 21}, {"n": 7},
         "All perfect pairs are compatible, so the exact count is n factorial."),
        ("almost_complete_missing_diagonal", {"n": 21}, {"n": 8},
         "All off-diagonal pairs are valid. The subfactorial recurrence counts the matchings.")),
    "exact_cover_abc345_d": (
        ("tiling_with_unused_pieces", {"side": 10, "tile_height": 2, "tile_width": 10, "tiles": 7},
         {"side": 6, "tile_height": 2, "tile_width": 6, "tiles": 5},
         "Long strips tile the board exactly, leaving two optional unit tiles unused."),
        ("equal_area_incompatible_corners", {"side": 10, "tile_height": 4, "tile_width": 5, "tiles": 7},
         {"side": 6, "tile_height": 4, "tile_width": 3, "tiles": 6},
         "Every edge would partition into two equal segments, requiring incompatible orientations in each corner.")),
    "maximum_submatrix_awc0097_d": (
        ("all_negative_square", {"rows": 500, "columns": 500, "value": -10000}, {"rows": 7, "columns": 7, "value": -8},
         "Every nonempty rectangle has negative weight; a single cell is optimal."),
        ("all_positive_tall", {"rows": 500, "columns": 495, "value": 10000}, {"rows": 8, "columns": 6, "value": 17},
         "The optimal rectangle covers the whole tall matrix; the full answer exceeds signed 32-bit range.")),
    "digit_dp_dp_s": (
        ("full_nines", {"digits": 10000, "divisor": 100}, {"digits": 4, "divisor": 7},
         "An all-nine bound includes every padded digit vector; use a power of the digit distribution."),
        ("decimal_power_bound", {"digits": 10000, "divisor": 100}, {"digits": 5, "divisor": 3},
         "A power-of-ten bound gives every shorter number and one additional number of digit sum one.")),
    "sparse_knapsack_yuki626": (
        ("quantized_large_weights", {"n": 5000, "limit": 2048, "scale": 1048576},
         {"n": 14, "limit": 61, "scale": 137},
         "Large raw weights are exact multiples of one scale; dense DP in scaled units supplies the expected value."),
        ("at_most_two_heavy", {"n": 5000, "limit": 2147483648}, {"n": 17, "limit": 13097},
         "Each weight is above one third of capacity. An independent weighted pair search suffices.")),
    "bounded_composition_counts_dp_m": (
        ("uncapped_stars_and_bars", {"n": 100, "k": 100000, "bound": 100000}, {"n": 6, "k": 20, "bound": 20},
         "Every per-part cap is at least the target; stars and bars gives an exact modular binomial coefficient."),
        ("tight_all_parts", {"n": 100, "k": 100000, "bound": 1000}, {"n": 8, "k": 48, "bound": 6},
         "The target equals the sum of caps, so only the all-maximal assignment is possible.")),
    "subarray_extrema_agc005_b": (
        ("increasing_permutation", {"n": 200000}, {"n": 45},
         "Increasing order retains the full monotone stack; each subarray minimum is its first element."),
        ("alternating_extremes", {"n": 200000}, {"n": 34},
         "High values can be minima only in singleton intervals; each low value has two starting positions.")),
    "largest_histogram_rectangle_abc189_c": (
        ("full_height_plateau", {"n": 10000, "height": 100000}, {"n": 34, "height": 327},
         "A constant plateau attains height times the entire width and exercises equal-height handling."),
        ("strictly_descending_heights", {"n": 10000}, {"n": 39},
         "Each width is best at the beginning; the largest width times height is floor((n+1)^2/4).")),
    "knapsack_by_value_dp_e": (
        ("maximum_value_table", {"n": 100, "value": 1000, "weight": 10000000},
         {"n": 15, "value": 41, "weight": 1000},
         "All items fit exactly at capacity; the value table reaches the maximum total value of 100000."),
        ("wide_near_limit_proportional", {"n": 100, "value": 1000, "weight": 18000},
         {"n": 14, "value": 42, "weight": 167},
         "Weights are proportional to distinct values; choose a budget one below the weight of target+1.")),
    "top_k_cartesian_sums_abc123_d": (
        ("equal_large_scores", {"n": 1000, "k": 3000, "score": 10000000000}, {"n": 7, "k": 60, "score": 101},
         "All product choices tie above 32-bit range; the expected result repeats the exact score K times."),
        ("distinct_mixed_radix_scores", {"n": 1000, "k": 3000, "score": 10000000000}, {"n": 10, "k": 57, "score": 1001},
         "Three positional weights make every choice distinct; the top K scores are consecutive integers.")),
    "optimal_adjacent_merge_dp_n": (
        ("equal_weights", {"n": 200000, "weight": 999999937}, {"n": 45, "weight": 29},
         "An optimal full binary tree for equal weights has leaves at only two consecutive depths."),
        ("one_positive_interior", {"n": 200000, "weight": 1000000000}, {"n": 41, "weight": 137},
         "Both zero-weight runs merge independently at no cost; the interior positive leaf must have depth at least two.")),
    "floor_sum_moments_abc402_g": (
        ("fibonacci_full_period", {"q": 100000, "n": 832040, "m": 832040, "a": 514229},
         {"q": 40, "n": 34, "m": 34, "a": 21},
         "Consecutive Fibonacci moduli force long Euclidean reductions; one coprime period permutes all residues."),
        ("descending_partial_period", {"q": 100000, "n": 999983, "m": 1000000, "a": 999999},
         {"q": 43, "n": 29, "m": 43, "a": 42},
         "Slope -1 modulo m traverses a cyclic interval; descending residues and wrap points have a closed product sum.")),
    "floor_path_product_abc429_g": (
        ("weighted_fibonacci_periods", {"q": 100, "n": 999280040, "m": 832040, "a": 514229, "r": 998244353},
         {"q": 35, "n": 68, "m": 34, "a": 21, "r": 998244353},
         "Consecutive Fibonacci moduli force long Euclidean reductions; each complete period is a geometric series."),
        ("weighted_reverse_mixed_modulus", {"q": 100, "n": 999999937, "m": 1000000000, "a": 999999999, "r": 999292928},
         {"q": 33, "n": 29, "m": 43, "a": 42, "r": 243968},
         "Descending exponents form at most two geometric intervals. Modulus has both unit and nilpotent components.")),
    "relaxed_convolution_abc315_ex": (
        ("constant_dense_feedback", {"n": 200000}, {"n": 65},
         "Force F_i=1 using the reciprocal of the triangular count of pairs i+j<n."),
        ("alternating_dense_feedback", {"n": 200000}, {"n": 77},
         "Force F_i=(-1)^i using the closed alternating sum of i+j<n pair weights.")),
    "permutation_rank_decimal_yuki696": (
        ("descending_exact_factorial", {"n": 100000}, {"n": 19},
         "The last permutation has one-based index n!; compare the entire exact decimal result."),
        ("front_three_reversed", {"n": 100000}, {"n": 24},
         "The first three values reversed have just two nonzero Lehmer digits and rank 1+2(n-1)!+(n-2)!.")),
}
VALIDITY = {
    "subset_xor_square_range_yuki3792": "1<=N,Q<=500000; values in [0,2^30); point sets and nonempty range queries.",
    "josephus_survivor_aoj0085": "At most 50 datasets, 1<=n,m<=999, followed by 0 0.",
    "dyadic_interval_cover_abc349_d": "0<=L<R<=2^60.",
    "divisibility_chain_ends_arc116_c": "1<=N,M<=200000.",
    "pairwise_coprime_progression_yuki3832": "1<=T<=100000, each 1<=N,D<=10^18.",
    "tree_subset_medians_yuki3834": "1<=K<=N<=200000, and N-1 unordered edges form a tree.",
    "weighted_taylor_shifts_yuki3793": "1<=N,M<=100000, and every coefficient is a residue modulo 998244353.",
    "dice_orientation_yuki1572": "2<=H,W<=1000, distinct open start and goal, other cells are open or blocked.",
    "line_chip_firing_yuki3801": "1<=N<=200000, and 0<=A[i]<=10^9.",
    "hub_sparse_distances_yuki3810": "2<=N<=200000, 1<=M,Q<=200000, costs in [0,10^18], and U<V, a<b.",
    "histogram_rook_cover_agc041_f": "1<=N<=400, and 1<=h[i]<=N.",
    "ferrers_rook_abc272_ex": "1<=N<=200000, and 0<=A[i]<N.",
    "krawtchouk_transform_arc133_f": "1<=N<=100000, 1<=K<=10^9, and N+1 nonnegative weights summing to 10^9.",
    "polynomial_differential_operator_yuki2583": "0<=n<=100000, 1<=m<=100000, residues in [0,998244353).",
    "reeds_sloane_abc129_f": "1<=L,A,B<10^18, A+B(L-1)<10^18, 2<=M<=10^9.",
    "short_simple_path_yuki408": "2<=n<=20000, simple undirected graph with <=50000 edges.",
    "round_robin_schedule_arc208_d": "One case, 1<=n<=500 and each prescribed first opponent in [1,n].",
    "xor_convolution_without_division_yuki1901": "0<=n<=17, two arrays of 2^n polynomials each encoded by 32 bits.",
    "exponential_coefficient_transform_agc076_d": "1<=n,m<=250000, all m integral red offsets in [0,100000000).",
    "degree_sequence_yahoo2018_e": "1<=n<=100000, n nondecreasing degrees each in [0,n-1].",
    "polynomial_moments_yuki1875": "1<=n,moves<=100000 and 1<=A_i,B_i,C_i<=998244352.",
    "prefix_dominant_compositions_yuki3480": "1<=n,p,q<=500000, gcd(p,q)=1, exactly n output lines.",
    "domino_tableau_yuki2149": "1<=height<=1000, 1<=a_1<=...<=a_height<=1000.",
    "interval_matrix_rank_abc399_g": "2<=n<=150, n-1<=m<=min(c*n*(n-1)/2,500000), 1<=c<=300, sum of color capacities <=300, graph connected, no duplicate edge within one color.",
    "multiset_permutation_abc202_d": "1<=a,b<=30 and 1<=k<=binomial(a+b,a).",
    "subarray_aggregate_runs_yuki1036": "1<=n<=500000, every value lies in [1,10^18].",
    "polynomial_recurrence_abc222_h": "One integer n satisfying 1<=n<=10000000.",
    "polynomial_matrix_product_arc174_c": "One integer n satisfying 1<=n<=1000000.",
    "adjacent_swap_parity_arc214_e": "T=1; 2<=n<=300000; 1<=k<=1e9; both arrays have the same multiset over [1,n].",
    "matrix_pencil_determinant_yuki1303": "2<=n<=100, simple undirected graph with each edge stored u<v.",
    "tridiagonal_matrix_power_arc228_e": "2<=n<=1e9; 1<=m<=125000.",
    "modular_shift_sums_abc483_f": "1<=n<=500000; 1<=queries<=200000; 0<=offset<modulus<=500000; 0<=x<=500000.",
    "modular_shift_mex_abc465_g": "1<=n,q<=200000; 1<=m,k<=1e9; 0<=c<m; all offsets/updates lie in [0,m).",
    "modular_shift_minimum_arc182_e": "1<=n<=1e5; 1<=m,k<=1e9; 0<=c<m; 0<=each offset<m.",
    "lcm_spanning_tree_yuki3827": "2<=n<=200000; 1<=each positive label<=200000.",
    "permutation_comparison_count_dp_t": "2<=n<=3000; comparison string of length n-1, containing only < and >.",
    "alien_trick_abc462_f": "3<=length<=300000; uppercase letters; 1<=extra<=10; one test.",
    "subtraction_game_dp_k": "1<=moves<=100; 1<=stones<=100000; distinct positive removals no larger than stones.",
    "xor_two_independent_sets_game_x": "1<=t<=10; 1<=n<=200; 0<=a_i<2^60; 1<=target<2^60.",
    "linear_matroid_parity_abc482_g": "1 test; 2<=n<=500; simple graph; sum(capacities)<=500; all capacities in [1,n-1].",
    "disk_lattice_points_abc191_d": "|X|,|Y|<=1e5; 0<R<=1e5; at most 4 fractional decimal digits.",
    "hamiltonian_route_abc180_e": "2<=n<=17; all 3D points distinct; each coordinate in [-1e6,1e6].",
    "nonadjacent_selection_abc218_h": "2<=n<=200000; 1<=red<n; 1<=each edge reward<=1e9.",
    "matrix_permanent_dp_o": "1<=n<=21; square binary matrix.",
    "exact_cover_abc345_d": "1<=n<=7; 1<=all board and tile dimensions<=10.",
    "maximum_submatrix_awc0097_d": "1<=rows,columns<=500; -10000<=a_ij<=10000.",
    "digit_dp_dp_s": "1<=K<10^10000; 1<=D<=100.",
    "sparse_knapsack_yuki626": "2<=n<=5000; 1<=capacity,weight,value<=2^31; deterministic workloads.",
    "bounded_composition_counts_dp_m": "1<=n<=100; 0<=k<=100000; 0<=a_i<=k.",
    "subarray_extrema_agc005_b": "1<=n<=200000; permutation of 1..n.",
    "largest_histogram_rectangle_abc189_c": "1<=n<=10000; 1<=height<=100000 for every bar.",
    "knapsack_by_value_dp_e": "1<=n<=100; 1<=budget<=1e9; 1<=weight<=budget and 1<=value<=1000 for all items.",
    "top_k_cartesian_sums_abc123_d": "1<=X,Y,Z<=1000; 1<=K<=min(3000,X*Y*Z); 1<=scores<=1e10.",
    "optimal_adjacent_merge_dp_n": "Library workload: n up to 200000, 0<=a_i<=1e9, optimal cost fits uint64_t.",
    "floor_sum_moments_abc402_g": "1<=q<=100000; 1<=n,m<=1000000; 0<=a,b1,b2<m.",
    "floor_path_product_abc429_g": "1<=q<=100; 1<=n,m,r<=1e9; 0<=a,b<m; 1<=x<r.",
    "relaxed_convolution_abc315_ex": "1<=n<=200000; 0<=A_i<998244353.",
    "permutation_rank_decimal_yuki696": "1<=n<=100000; a permutation of 1..n.",
}

PROFILES["polynomial_recurrence_fit_abc222_h"] = PROFILES["polynomial_recurrence_abc222_h"]
VALIDITY["polynomial_recurrence_fit_abc222_h"] = VALIDITY["polynomial_recurrence_abc222_h"]


def _sum_indices(n):
    return n * (n - 1) // 2


def _prefix_products(length, modulus, difference):
    """Sum r * ((r + difference) % modulus) for 0<=r<length."""
    linear = _sum_indices(length)
    squared = length * (length - 1) * (2 * length - 1) // 6
    wrapped = linear - _sum_indices(min(length, modulus - difference))
    return squared + difference * linear - modulus * wrapped


def _floor_moments(plan, inp, out):
    dimensions = plan["dimensions"]
    queries, n, modulus, slope = (dimensions[name] for name in ("q", "n", "m", "a"))
    inp.line(queries)
    for i in range(queries):
        first = (i * 123457 + modulus // 7) % modulus
        second = first if i % 15 == 0 else (i * 65537 + modulus // 3) % modulus
        inp.line(n, modulus, slope, first, second)
        lower, upper = sorted((first, second))
        difference = upper - lower
        if plan["scenario"] == "fibonacci_full_period":
            answer = _prefix_products(modulus, modulus, difference)
        else:
            # The n residues start at lower and decrease by one, possibly wrapping once.
            boundary = lower + 1
            begin = max(0, boundary - n)
            answer = _prefix_products(boundary, modulus, difference) - _prefix_products(begin, modulus, difference)
            if n > boundary:
                answer += _prefix_products(modulus, modulus, difference)
                answer -= _prefix_products(modulus - (n - boundary), modulus, difference)
        out.line(answer)


def _geometric(base, count, modulus):
    if base == 1:
        return count % modulus
    return (pow(base, count, modulus) - 1) * pow(base - 1, -1, modulus) % modulus


def _floor_path(plan, inp, out):
    dimensions = plan["dimensions"]
    queries, n, modulus, slope, ring = (dimensions[name] for name in ("q", "n", "m", "a", "r"))
    inp.line(queries)
    for i in range(queries):
        intercept = (i * 29173 + modulus // 7) % modulus
        if plan["scenario"] == "weighted_fibonacci_periods":
            base = 1 if i % 10 == 0 else 2 + (i * 198293) % (ring - 2)
            answer = _geometric(base, modulus, ring) * (n // modulus) % ring
        else:
            base = 1 << (1 + i % 9)
            boundary = intercept + 1
            begin = max(0, boundary - n)
            answer = pow(base, begin, ring) * _geometric(base, boundary - begin, ring)
            if n > boundary:
                answer += pow(base, modulus - (n - boundary), ring) * _geometric(base, n - boundary, ring)
            answer %= ring
        inp.line(n, modulus, slope, intercept, base, ring)
        out.line(answer)


def _relaxed(plan, inp, out):
    prime = 998244353
    n = plan["dimensions"]["n"]
    inp.line(n)
    if plan["scenario"] == "constant_dense_feedback":
        inp.line(*(pow(i * (i + 1) // 2, -1, prime) for i in range(1, n + 1)))
        out.line(*([1] * n))
    else:
        inp.line(*(-pow((i + 1) // 2, -1, prime) % prime for i in range(1, n + 1)))
        out.line(*(prime - 1 if i % 2 else 1 for i in range(1, n + 1)))


def _mixed_radix(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    if plan["scenario"] == "descending_exact_factorial":
        inp.line(*range(n, 0, -1))
        answer = factorial(n)
    else:
        inp.line(3, 2, 1, *range(4, n + 1))
        answer = (2 * (n - 1) + 1) * factorial(n - 2) + 1
    # Decimal converts Python integers without changing the process-wide digit limit.
    out.line(Decimal(answer))


def _adjacent_merge(plan, inp, out):
    n, weight = (plan["dimensions"][name] for name in ("n", "weight"))
    inp.line(n)
    if plan["scenario"] == "equal_weights":
        inp.line(*([weight] * n))
        depth = (n - 1).bit_length()
        out.line(weight * (n * (depth + 1) - (1 << depth)))
    else:
        row = [0] * n
        row[n // 2] = weight
        inp.line(*row)
        out.line(2 * weight)


def _cartesian_sums(plan, inp, out):
    n, k, score = (plan["dimensions"][name] for name in ("n", "k", "score"))
    inp.line(n, n, n, k)
    if plan["scenario"] == "equal_large_scores":
        for _ in range(3):
            inp.line(*([score] * n))
        for _ in range(k):
            out.line(3 * score)
    else:
        for weight in (1, n, n * n):
            inp.line(*(score - i * weight for i in range(n - 1, -1, -1)))
        for i in range(k):
            out.line(3 * score - i)


def _knapsack_by_value(plan, inp, out):
    n, value, weight = (plan["dimensions"][name] for name in ("n", "value", "weight"))
    if plan["scenario"] == "maximum_value_table":
        inp.line(n, n * weight)
        for _ in range(n):
            inp.line(weight, value)
        out.line(n * value)
    else:
        selected = n // 2
        target = selected * (2 * value + 1 - selected) // 2
        inp.line(n, weight * (target + 1) - 1)
        for score in range(value, value - n, -1):
            inp.line(weight * score, score)
        out.line(target)


def _histogram_rectangle(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    if plan["scenario"] == "full_height_plateau":
        height = plan["dimensions"]["height"]
        inp.line(*([height] * n))
        out.line(n * height)
    else:
        inp.line(*range(n, 0, -1))
        out.line((n + 1) ** 2 // 4)


def _subarray_extrema(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    if plan["scenario"] == "increasing_permutation":
        inp.line(*range(1, n + 1))
        out.line(n * (n + 1) * (n + 2) // 6)
    else:
        half = n // 2
        inp.line(*(value for index in range(half) for value in (n - index, index + 1)))
        highs = half * (3 * half + 1) // 2
        lows = sum(2 * index * (2 * (half - index) + 1) for index in range(1, half + 1))
        out.line(highs + lows)


def _bounded_compositions(plan, inp, out):
    n, k, bound = (plan["dimensions"][name] for name in ("n", "k", "bound"))
    inp.line(n, k)
    inp.line(*([bound] * n))
    answer = comb(n + k - 1, n - 1) if plan["scenario"] == "uncapped_stars_and_bars" else 1
    out.line(answer % 1000000007)


def _sparse_knapsack(plan, inp, out):
    dimensions = plan["dimensions"]
    n, limit = dimensions["n"], dimensions["limit"]
    scores = [(982451653 * i + 86028121 * i * i) % (1 << 31) + 1 for i in range(n)]
    if plan["scenario"] == "quantized_large_weights":
        scale = dimensions["scale"]
        weights = [1 + (9973 * i + 17 * i * i) % limit for i in range(n)]
        inp.line(n, limit * scale)
        best = [0] * (limit + 1)
        for value, weight in zip(scores, weights):
            inp.line(value, weight * scale)
            for budget in range(limit, weight - 1, -1):
                best[budget] = max(best[budget], best[budget - weight] + value)
        out.line(best[-1])
    else:
        minimum = limit // 3 + 1
        weights = [minimum + (31627 * i + 137567 * i * i) % (limit - minimum + 1) for i in range(n)]
        inp.line(n, limit)
        for value, weight in zip(scores, weights):
            inp.line(value, weight)
        pairs = sorted(zip(weights, scores))
        ordered_weights = [weight for weight, _ in pairs]
        best = max(scores)
        prefix = [0]
        for i, (weight, value) in enumerate(pairs):
            left = bisect_right(ordered_weights, limit - weight, 0, i)
            best = max(best, prefix[left] + value)
            prefix.append(max(prefix[-1], value))
        out.line(best)


def _digit_counter_power(digits, residues):
    modulus = 1000000007
    def multiply(left, right):
        return [sum(left[j] * right[(i - j) % residues] for j in range(residues)) % modulus
                for i in range(residues)]
    power = [0] * residues
    for digit in range(10):
        power[digit % residues] += 1
    answer = [1] + [0] * (residues - 1)
    while digits:
        if digits % 2:
            answer = multiply(answer, power)
        power = multiply(power, power)
        digits //= 2
    return answer[0]


def _digit_dp(plan, inp, out):
    length, divisor = (plan["dimensions"][key] for key in ("digits", "divisor"))
    if plan["scenario"] == "full_nines":
        inp.line("9" * length)
        result = _digit_counter_power(length, divisor)
    else:
        inp.line("1" + "0" * (length - 1))
        result = _digit_counter_power(length - 1, divisor) + (divisor == 1)
    inp.line(divisor)
    out.line((result - 1) % 1000000007)


def _maximum_submatrix(plan, inp, out):
    rows, columns, value = (plan["dimensions"][key] for key in ("rows", "columns", "value"))
    inp.line(rows, columns)
    for _ in range(rows):
        inp.line(*([value] * columns))
    out.line(value if value < 0 else rows * columns * value)


def _exact_cover(plan, inp, out):
    side, h, w, n = (plan["dimensions"][key] for key in ("side", "tile_height", "tile_width", "tiles"))
    inp.line(n, side, side)
    fits = plan["scenario"] == "tiling_with_unused_pieces"
    strips = side // h if fits else n
    for _ in range(strips):
        inp.line(h, w)
    for _ in range(n - strips):
        inp.line(1, 1)
    out.line("Yes" if fits else "No")


def _matrix_permanent(plan, inp, out):
    n = plan["dimensions"]["n"]
    complete = plan["scenario"] == "complete_bipartite"
    inp.line(n)
    for row in range(n):
        inp.line(*(int(complete or row != column) for column in range(n)))
    if complete:
        answer = factorial(n)
    else:
        previous, answer = 1, 0
        for i in range(2, n + 1):
            previous, answer = answer, (i - 1) * (previous + answer)
    out.line(answer % 1000000007)


def _nonadjacent_selection(plan, inp, out):
    n, weight = (plan["dimensions"][key] for key in ("n", "weight"))
    if plan["scenario"] == "equal_edges_balanced_colors":
        inp.line(n, n // 2)
        inp.line(*([weight] * (n - 1)))
        out.line((n - 1) * weight)
    else:
        inp.line(n, n // 2 - 1)
        strong_edges = [weight - (i * 1_042_481 + i * i * 29) % (weight // 2) for i in range(1, n - 1)]
        inp.line(1, *strong_edges)
        out.line(sum(strong_edges))


def _hamiltonian_route(plan, inp, out):
    n, radius = (plan["dimensions"][key] for key in ("n", "span"))
    inp.line(n)
    diagonal = plan["scenario"] == "inclined_diagonal_cities"
    for i in list(range(1, n, 2)) + list(range(0, n, 2)):
        x = -radius + 2 * radius * i // (n - 1)
        if diagonal:
            inp.line(x, x, -x)
        else:
            inp.line(x, 0, 0)
    out.line(radius * (10 if diagonal else 4))


def _disk_lattice_points(plan, inp, out):
    unit = 10000
    x, y, radius = (plan["dimensions"][key] for key in ("x", "y", "radius"))

    def decimal_token(value):
        whole, fraction = divmod(abs(value), unit)
        sign = "-" if value < 0 else ""
        return f"{sign}{whole}.{fraction:04d}"

    inp.line(decimal_token(x), decimal_token(y), decimal_token(radius))
    total = 0
    for column in range(-((radius - x) // unit), (x + radius) // unit + 1):
        height = isqrt(radius * radius - (unit * column - x) ** 2)
        total += (y + height) // unit + (height - y) // unit + 1
    out.line(total)


def _linear_matroid_parity(plan, inp, out):
    dims = plan["dimensions"]
    n, capacity = dims["n"], dims["capacity"]
    inp.line(1)
    if plan["scenario"] == "complete_graph_unit_capacity":
        inp.line(n, n * (n - 1) // 2)
        inp.line(*([capacity] * n))
        for u in range(1, n + 1):
            for v in range(u + 1, n + 1):
                inp.line(u, v)
        out.line(n // 2)
    else:
        left = dims["left"]
        inp.line(n, left * (n - left))
        inp.line(*([capacity] * n))
        for u in range(1, left + 1):
            for v in range(left + 1, n + 1):
                inp.line(u, v)
        out.line(left * capacity)


def _xor_two_independent_sets(plan, inp, out):
    dims = plan["dimensions"]
    n, bits, tests = dims["n"], dims["bits"], dims["t"]
    always_wins = plan["scenario"] == "two_complete_bases"
    inp.line(tests)
    for case in range(tests):
        basis = [(1 << i) | (((i + 7) * 0x5BF03635 + case * 0x9E3779B9) & ((1 << i) - 1))
                 for i in range(bits)]
        target = basis[-1] ^ basis[case % (bits - 1)] if always_wins else (1 << bits)
        inp.line(n, target)
        filler = [basis[(i + case) % bits] ^ basis[(3 * i + 1) % bits]
                  for i in range(n - 2 * bits)]
        inp.line(*filler, *basis, *reversed(basis))
        out.line("Alice" if always_wins else "Bob")


def _subtraction_game(plan, inp, out):
    count, stones, stride = (plan["dimensions"][key] for key in ("moves", "stones", "stride"))
    inp.line(count, stones)
    inp.line(*(stride * i for i in range(1, count + 1)))
    out.line("First" if (stones // stride) % (count + 1) else "Second")


def _alien_trick(plan, inp, out):
    n, extra = (plan["dimensions"][key] for key in ("n", "extra"))
    inp.line(1)
    if plan["scenario"] == "dense_equal_edit_ties":
        inp.line("Z" * n)
        out.line(3 * extra)
    else:
        inp.line("X" + "ABC" * (n // 3 - 1) + "YZ")
        out.line(n)
    inp.line(extra)


def _permutation_comparisons(plan, inp, out):
    n = plan["dimensions"]["n"]
    modulus = 1_000_000_007
    inp.line(n)
    if plan["scenario"] == "alternating_comparisons_at_maximum":
        inp.line(("<>" * (n // 2))[:n - 1])
        triangle = [1]
        for _ in range(n):
            row = [0]
            for count in reversed(triangle):
                row.append((row[-1] + count) % modulus)
            triangle = row
        out.line(triangle[-1])
    else:
        middle = n // 2
        inp.line("<" * (middle - 1) + ">" + "<" * (n - middle - 1))
        out.line((comb(n, middle) - 1) % modulus)


def _lcm_spanning_tree(plan, inp, out):
    n, upper = (plan["dimensions"][key] for key in ("n", "upper"))
    if plan["scenario"] == "dividing_minimum_at_maximum":
        support = list(range(upper, 1, -2))
        extra = 0
    else:
        support = [value for value in range(upper, 1, -1) if value % 2 == 0 or value % 3 == 0]
        extra = 3
    labels = [support[index % len(support)] for index in range(n)]
    inp.line(n)
    inp.line(*labels)
    out.line(sum(labels) - 2 + extra)


def _modular_shift_minimum(plan, inp, out):
    n, modulus, step, count, perturbation = (plan["dimensions"][key] for key in ("n", "m", "c", "k", "offset"))
    gap = modulus // n
    stride = gcd(modulus, step)
    period = modulus // stride
    offsets = [i * gap + (i & 1) * perturbation for i in range(n)]
    inp.line(n, modulus, step, count)
    inp.line(*(offsets[i * 137 % n] for i in range(n)))
    lower, higher = (gap - perturbation) // stride, (gap + perturbation) // stride
    per_period = n // 2 * (lower * (lower - 1) + higher * (higher - 1)) * stride // 2
    out.line(count // period * per_period)


def _modular_shift_mex(plan, inp, out):
    n, modulus, step, count, queries = (plan["dimensions"][key] for key in ("n", "m", "c", "k", "q"))
    inp.line(n, modulus, step, count)
    if plan["scenario"] == "sparse_isolated_residue_updates":
        inp.line(*(4 * i + 2 for i in range(n)))
        inp.line(queries)
        for i in range(queries):
            index = i * 137 % n
            inp.line(index + 1, 4 * index + (3 if i // n % 2 == 0 else 2))
            out.line(n)
    else:
        inp.line(*range(n))
        inp.line(queries)
        for i in range(queries):
            index = i // 2 * 7919 % n
            if i % 2 == 0:
                inp.line(index + 1, (index + 1) % modulus)
                out.line(count * (modulus - 1) // 2)
            else:
                inp.line(index + 1, index)
                out.line(count * modulus)


def _modular_shift_sums(plan, inp, out):
    n, upper, queries = (plan["dimensions"][key] for key in ("n", "upper", "queries"))
    shifts = (upper if i == 0 else i * 97 % (upper + 1) for i in range(queries))
    inp.line(n)
    if plan["scenario"] == "dense_moduli_many_periods":
        mod = 1
        remaining = n
        constant = 0
        drops = [0] * (upper + 1)
        while remaining >= mod + 1:
            for offset in range(mod):
                inp.line(offset, mod)
            inp.line(0, mod)
            constant += mod * (mod - 1) // 2
            for x in range(mod, upper + 1, mod):
                drops[x] += mod
            remaining -= mod + 1
            mod += 1
        for _ in range(remaining):
            inp.line(0, 1)
        results = [constant]
        growth = mod - 1
        for x in range(1, upper + 1):
            results.append(results[-1] + growth - drops[x])
        inp.line(queries)
        for x in shifts:
            inp.line(x)
            out.line(results[x])
    else:
        groups = n // 2
        lower = upper - groups + 1
        for mod in range(lower, upper + 1):
            inp.line(0, mod)
            inp.line(mod - 1, mod)
        inp.line(queries)
        def prefix(x):
            return (lower + x) * max(0, x - lower + 1) // 2

        for x in shifts:
            inp.line(x)
            out.line(groups * (2 * x - 1) - prefix(x) - prefix(x - 1) if x else prefix(upper) - groups)


def _tridiagonal_matrix_power(plan, inp, out):
    n, m = (plan["dimensions"][key] for key in ("n", "m"))
    inp.line(n, m)
    modulus = 998244353
    if n == m:
        previous, derangements = 1, 0
        permutations = 1
        for k in range(2, n + 1):
            previous, derangements = derangements, (k - 1) * (previous + derangements) % modulus
            permutations = permutations * k % modulus
        out.line(permutations * derangements % modulus)
    elif n == 10**9 and m == 125000:
        out.line(357543611)  # AtCoder ARC228 E official sample 3.
    else:
        current = [1] + [0] * m
        for _ in range(n - m):
            after = [current[1]] + [0] * m
            for j in range(1, m + 1):
                after[j] = (m - j + 1) * (m - j + 2) * current[j - 1] + 2 * j * (m - j + 1) * current[j]
                if j < m:
                    after[j] += (j + 1)**2 * current[j + 1]
            current = [value % modulus for value in after]
        tail = (sum((-1)**i * comb(m - j, i) * factorial(m - i) for i in range(m - j + 1))
                for j in range(m + 1))
        out.line(factorial(m) * sum(a * b for a, b in zip(current, tail)) % modulus)


def _matrix_pencil_determinant(plan, inp, out):
    n = plan["dimensions"]["n"]
    modulus = 998244353
    if plan["scenario"] == "complete_graph_constant_matrix":
        inp.line(n, n * (n - 1) // 2)
        for u in range(1, n):
            for v in range(u + 1, n + 1):
                inp.line(u, v)
        answer = pow(n, n - 2, modulus)
    else:
        inp.line(n, n - 1)
        for u in range(1, n):
            inp.line(u, u + 1)
        answer = (1 + sum((n - distance) * distance for distance in range(2, n))) % modulus
    out.line(0)
    out.line(answer)


def _adjacent_swap_parity(plan, inp, out):
    n, group = (plan["dimensions"][key] for key in ("n", "k"))
    inp.line(1)
    inp.line(n, group)
    if plan["scenario"] == "one_far_apart_equal_pair":
        values = list(range(1, n)) + [1]
        inp.line(*values)
        where = n // 2
        values[where - 1], values[where] = values[where], values[where - 1]
        inp.line(*values)
        out.line(n - 1)
    else:
        half = n // 2
        inp.line(*(list(range(1, half + 1)) * 2))
        inp.line(*(list(range(half, 0, -1)) * 2))
        stable = half * (half - 1)
        even = (stable + group - 1) // group
        odd = (stable + group) // group
        out.line(min(even + even % 2, odd + (odd + 1) % 2))


def _polynomial_matrix_product(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    modulus = 998244353
    inverse = array("I", [0]) * (2 * n)
    inverse[1] = 1
    for i in range(2, 2 * n):
        inverse[i] = modulus - inverse[modulus % i] * (modulus // i) % modulus
    total = difference = 0
    for new in range(1, n + 1):
        total = (total + (n - new) * inverse[new]) % modulus
        difference = (n - new - new * difference) * inverse[2 * n - new] % modulus
    half = (modulus + 1) // 2
    out.line((total + difference) * half % modulus, (total - difference) * half % modulus)


def _polynomial_recurrence(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    if n == 1:
        out.line(1)
        return
    modulus = 998244353
    power = 2 * n
    before, last = 1, 3 * power
    inverse = array("I", [0]) * n
    inverse[1] = 1
    for rank in range(2, n):
        inverse[rank] = modulus - inverse[modulus % rank] * (modulus // rank) % modulus
        following = ((3 * (power - rank + 1) * last + (2 * power - rank + 2) * before) * inverse[rank]) % modulus
        before, last = last, following
    out.line(last * pow(n, -1, modulus) % modulus)


def _aggregate_runs(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    if plan["scenario"] == "many_distinct_gcds":
        bits = plan["dimensions"]["bits"]
        groups, tail = divmod(n, bits + 1)
        inp.line(*(([1 << exponent for exponent in range(1, bits + 1)] + [1]) * groups
                   + [1 << exponent for exponent in range(1, 1 + tail)]))
        nonunit = groups * bits * (bits + 1) // 2 + tail * (tail + 1) // 2
        out.line(n * (n + 1) // 2 - nonunit)
        return
    before, next_value = 0, 1
    for _ in range(86):
        before, next_value = next_value, before + next_value
    inp.line(*(before if i % 2 == 0 else next_value for i in range(n)))
    out.line(n * (n - 1) // 2)


def _multiset_permutation(plan, inp, out):
    a, b = (plan["dimensions"][name] for name in ("a", "b"))
    if plan["scenario"] == "first_word_starting_with_b":
        inp.line(a, b, comb(a + b, a) // 2 + 1)
        out.line("b" + "a" * a + "b" * (b - 1))
        return
    answer = "ab" * b + "a" * (a - b)
    left_a, left_b = a, b
    rank = 1
    for letter in answer:
        if letter == "a":
            left_a -= 1
        else:
            rank += comb(left_a + left_b - 1, left_a - 1)
            left_b -= 1
    inp.line(a, b, rank)
    out.line(answer)


def _interval_matrix_rank(plan, inp, out):
    n, m, colors = (plan["dimensions"][name] for name in ("n", "m", "c"))
    inp.line(n, m, colors)
    if plan["scenario"] == "maximum_edges_with_one_full_rank_color":
        inp.line(n - 1, *([1] * (colors - 1)))
        emitted = 0
        for color in range(1, colors + 1):
            for u in range(1, n):
                for v in range(u + 1, n + 1):
                    if emitted == m:
                        out.line(colors)
                        return
                    inp.line(u, v, color)
                    emitted += 1
        assert emitted == m
        out.line(colors)
        return
    inp.line(*([1] * colors))
    base, spill = divmod(m, colors)
    path = [(u, u + 1) for u in range(1, n)]
    diagonals = [(u, v) for u in range(1, n) for v in range(u + 2, n + 1)]
    for color in range(1, colors + 1):
        count = base + (color <= spill) - len(path)
        for u, v in path + diagonals[:count]:
            inp.line(u, v, color)
    possible_starts = colors - n + 2
    out.line(possible_starts * (possible_starts + 1) // 2)


def _domino_tableau(plan, inp, out):
    height, width = (plan["dimensions"][name] for name in ("height", "width"))
    inp.line(height)
    if plan["scenario"] == "large_even_area_with_nonempty_core":
        inp.line(width - 3, width - 2, width - 1, *([width] * (height - 3)))
        out.line(0)
        return
    inp.line(*([width] * height))
    rows, columns = height // 2, width // 2
    total = height * width // 2
    modulus = 998244353
    factorials = [1]
    value = 1
    for size in range(1, total + 1):
        value = value * size % modulus
        if size < rows + columns:
            factorials.append(value)
    one_rectangle = 1
    for shift in range(rows):
        one_rectangle = one_rectangle * factorials[columns + shift] * pow(factorials[shift], -1, modulus) % modulus
    out.line(value * pow(one_rectangle, -2, modulus) % modulus)


def _prefix_dominant_compositions(plan, inp, out):
    n, p, q = (plan["dimensions"][name] for name in ("n", "p", "q"))
    inp.line(n, p, q)
    modulus = 998244353
    if plan["scenario"] == "equal_unit_totals":
        for length in range(1, n + 1):
            out.line(length * (length + 1) // 2 % modulus)
        return
    limit = max(p, q) + n - 1
    factorial = array("I", [1]) * (limit + 1)
    inverse_factorial = array("I", [1]) * (limit + 1)
    for degree in range(1, limit + 1):
        factorial[degree] = factorial[degree - 1] * degree % modulus
    inverse_factorial[limit] = pow(factorial[limit], -1, modulus)
    for degree in range(limit, 0, -1):
        inverse_factorial[degree - 1] = inverse_factorial[degree] * degree % modulus

    answer = 0
    independent = inverse_factorial[p] * inverse_factorial[q] % modulus
    for j in range(n):
        coefficient = factorial[p + j] * factorial[q + j] % modulus * independent % modulus
        answer = (answer + coefficient * inverse_factorial[j] * inverse_factorial[j + 1]) % modulus
        out.line(answer)


def _polynomial_moments(plan, inp, out):
    n, moves, residual = (plan["dimensions"][name] for name in ("n", "moves", "residual"))
    inp.line(n, moves)
    modulus = 998244353
    for rate in range(1, (n - residual) // 2 + 1):
        inp.line(1, rate, 1)
        inp.line(1, rate, modulus - 1)

    # Expand in T=exp(x), only for a few uncancelled factors.
    polynomial = [1]
    for j in range(residual):
        front, back, count = j + 2, 2 * j + 5, j % 3 + 1
        inp.line(front, back, count)
        for _ in range(count):
            expanded = [0] * (len(polynomial) + 1)
            for i, coefficient in enumerate(polynomial):
                expanded[i] = (expanded[i] + coefficient * (front - back)) % modulus
                expanded[i + 1] = (expanded[i + 1] + coefficient * back) % modulus
            polynomial = expanded

    terms = polynomial[1:]
    for _ in range(moves):
        for i in range(len(terms)):
            terms[i] = terms[i] * (i + 1) % modulus
        out.line(sum(terms) % modulus)


def _subset_xor_square_range(plan, inp, out):
    n, q = (plan["dimensions"][key] for key in ("n", "q"))
    modulus = 998244353
    mixed = plan["scenario"] == "maximum_mixed_updates_and_aggregate"
    pattern = (0, (1 << 30) - 1, 0x15555555, 0x2B333333) if mixed else tuple(1 << j for j in range(30))
    inp.line(n, q)
    base = list((pattern * ((n + len(pattern) - 1) // len(pattern)))[:n])
    inp.line(*base)
    powers = [1]
    for _ in range(n):
        powers.append(powers[-1] * 2 % modulus)
    cache = {}
    largest = (1 << 30) - 1
    all_integers_squares = largest * (largest + 1) * (2 * largest + 1) // 6 % modulus
    for i in range(q):
        if mixed and i % 2 == 0:
            position = 4 * ((i * 19073 + 7) % (n // 4))
            changed = pattern[i // 2 & 1]
            base[position] = changed
            inp.line(1, position + 1, changed)
            continue
        left = (i * 1031 + 1) % (n // 3)
        right = left + 1 if not mixed and i % 32 == 0 else n - (i * 1201 + 3) % (n // 4)
        inp.line(2, left + 1, right)
        length = right - left
        if not mixed and length >= 30:
            out.line(all_integers_squares * powers[length - 30] % modulus)
            continue
        # Mixed updates affect only positions congruent to zero; the other three generators never disappear.
        kinds = pattern[1:] if mixed and length >= 4 else tuple(sorted(set(base[left:right])))
        if kinds not in cache:
            values = [0]
            for value in kinds:
                values += [x ^ value for x in values]
            cache[kinds] = sum(x * x for x in values) % modulus
        out.line(cache[kinds] * powers[length - len(kinds)] % modulus)


def _josephus_survivor(plan, inp, out):
    n, t = (plan["dimensions"][key] for key in ("n", "t"))
    for i in range(t):
        count = n - i % 4
        step = n - 1 - i % 5 if plan["scenario"] == "maximum_linear_fallback" else 2 + i % 2
        inp.line(count, step)
        survivor = 0
        for size in range(2, count + 1):
            survivor = (survivor + step) % size
        out.line(survivor + 1)
    inp.line(0, 0)


def _dyadic_interval_cover(plan, inp, out):
    height = plan["dimensions"]["h"]
    limit = 1 << height
    if plan["scenario"] == "maximum_aligned_interval":
        inp.line(0, limit)
        out.line(1)
        out.line(0, limit)
        return
    inp.line(1, limit - 1)
    out.line(2 * (height - 1))
    for i in range(height - 1):
        out.line(1 << i, 1 << (i + 1))
    for i in range(height - 2, -1, -1):
        out.line(limit - (1 << (i + 1)), limit - (1 << i))


def _divisibility_chain_ends(plan, inp, out):
    n, m = (plan["dimensions"][key] for key in ("n", "m"))
    modulus = 998244353
    inp.line(n, m)
    if plan["scenario"] == "maximum_end_value_length_two":
        out.line(sum(m // start for start in range(1, m + 1)) % modulus)
        return
    if (n, m) == (200000, 200000):
        out.line(835917264)  # ARC116 C, official sample 3.
        return
    total = 0
    for end in range(1, m + 1):
        rest, paths = end, 1
        for factor in range(2, end + 1):
            exponent = 0
            while rest % factor == 0:
                rest //= factor
                exponent += 1
            paths *= comb(n - 1 + exponent, exponent)
        total += paths
    out.line(total % modulus)


def _pairwise_coprime_progression(plan, inp, out):
    cases = plan["dimensions"]["t"]
    inp.line(cases)
    if plan["scenario"] == "maximum_first_missing_prime_boundary":
        for i in range(cases):
            inp.line(105 + i % 2, 614889782588491410)
            out.line("Yes" if i % 2 == 0 else "No")
        return
    for i in range(cases):
        inp.line(10**18, 10**18 - i)
        out.line("No")


def _tree_subset_medians(plan, inp, out):
    n, k = (plan["dimensions"][key] for key in ("n", "k"))
    modulus = 998244353
    inp.line(n, k)
    if plan["scenario"] == "maximum_star_pair_medians":
        for v in range(2, n + 1):
            inp.line(1, v)
        out.line((3 - 2 * pow(n, -1, modulus)) % modulus)
        return
    for v in range(1, n):
        inp.line(v, v + 1)
    out.line((1 + (n + 1) * pow(k + 1, -1, modulus)) % modulus)


def _weighted_taylor_shifts(plan, inp, out):
    n, m = (plan["dimensions"][key] for key in ("n", "m"))
    mod = 998244353
    inp.line(n, m)
    if plan["scenario"] == "maximum_full_order_finite_difference":
        order = m
        coefficients = [0] * n + [1]
    else:
        order = m - 1
        coefficients = [(65537 * i + 7) % mod for i in range(n + 1)]
    inp.line(*coefficients)
    fact = [1] * (order + 1)
    inv_fact = [1] * (order + 1)
    for i in range(1, order + 1):
        fact[i] = fact[i - 1] * i % mod
    inv_fact[-1] = pow(fact[-1], -1, mod)
    for i in range(order, 0, -1):
        inv_fact[i - 1] = inv_fact[i] * i % mod
    weights = [(-1)**((order - i) % 2) * fact[-1] * inv_fact[i] * inv_fact[order - i] % mod
               for i in range(order + 1)]
    inp.line(*weights, *([0] * (m - order)))
    signed_factorial = (-1)**(order % 2) * fact[-1]
    result = [0] * (n + 1)
    result[0] = signed_factorial * coefficients[order] % mod
    if order < n:
        result[0] = (result[0] - signed_factorial * coefficients[n] * (order * (order + 1) // 2)) % mod
        result[1] = signed_factorial * coefficients[n] * n % mod
    out.line(*result)


def _dice_orientation(plan, inp, out):
    height, width = (plan["dimensions"][key] for key in ("h", "w"))
    inp.line(height, width)
    inp.line(1, 1)
    inp.line(height, width)
    open_row = "." * width
    if plan["scenario"] == "maximum_open_corner_to_corner":
        for _ in range(height):
            inp.line(open_row)
        out.line(height + width - 2)
        return
    for _ in range(height - 2):
        inp.line(open_row)
    inp.line("." * (width - 1) + "#")
    inp.line("." * (width - 2) + "#.")
    out.line(-1)


def _line_chip_firing(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    half = n // 2
    if plan["scenario"] == "maximum_single_pile":
        inp.line(*([10**9] * n))
        out.line(half * (half + 1) * (2 * half + 1) // 6)
        return
    inp.line(*(999850000 + ((37 * i) % n) // 2 for i in range(n)))
    left = -(half // 2)
    before = 2 * sum(i * i for i in range(half))
    after = sum(i * i for i in range(left, left + n))
    out.line((after - before) // 2)


def _hub_sparse_distances(plan, inp, out):
    n, m, queries = (plan["dimensions"][key] for key in ("n", "m", "q"))
    warp = 10**18
    inp.line(n, m, queries)
    if plan["scenario"] == "maximum_zero_contractions":
        half = n // 2
        inp.line(3, *([warp] * (n - 2)), 5)
        for u in range(1, n):
            inp.line(u, u + 1, warp if u == half else 0)
        inp.line(1, n, warp)
        for q in range(queries):
            if q % 3 == 0:
                inp.line(1, q % (half - 1) + 2)
                out.line(0)
            elif q % 3 == 1:
                inp.line(q % (n - half - 1) + half + 1, n)
                out.line(0)
            else:
                inp.line(q % half + 1, q % (n - half) + half + 1)
                out.line(8)
        return
    inp.line(*([warp] * n))
    core_edges = min(60, n // 2 - 1)
    for i in range(core_edges):
        inp.line(2 * i + 1, 2 * i + 2, 1)
    for u in range(1, n - core_edges + 1):
        inp.line(u, n, warp)
    for q in range(queries):
        if q % 4 == 0:
            i = q // 4 % core_edges
            inp.line(2 * i + 1, 2 * i + 2)
            out.line(2 << i)
        else:
            inp.line(*((1, 3), (1, n), (n - 1, n))[q % 4 - 1])
            out.line(2 * warp)


def _histogram_rook_cover(plan, inp, out):
    n = plan["dimensions"]["n"]
    modulus = 998244353
    inp.line(n)
    if plan["scenario"] == "maximum_full_histogram":
        inp.line(*([n] * n))
        no_empty_rows = pow(pow(2, n, modulus) - 1, n, modulus)
        no_empty_lines = sum((-1)**i * comb(n, i) * pow(pow(2, n - i, modulus) - 1, n, modulus)
                             for i in range(n + 1))
        out.line((2 * no_empty_rows - no_empty_lines) % modulus)
        return
    tall = (n + 1) // 2
    inp.line(*(n if i % 2 == 0 else 1 for i in range(n)))
    all_tall_nonempty = pow(pow(2, n, modulus) - 1, tall, modulus)
    all_with_empty_base = pow(pow(2, n - 1, modulus) - 1, tall, modulus)
    out.line((all_tall_nonempty * pow(2, n - tall, modulus) - all_with_empty_base) % modulus)


def _ferrers_rook(plan, inp, out):
    n = plan["dimensions"]["n"]
    tall_count = n // 2 + 1
    modulus = 998244353
    inp.line(n)
    if plan["scenario"] == "maximum_sparse_and_full_columns":
        inp.line(*(n - 1 if i % 2 == 0 or i == n - 1 else 0 for i in range(n)))
        # All tall columns are hits. A short column adds one hit only if it occupies the first row.
        out.line(tall_count if tall_count % 2 == 0 else n - tall_count)
        return
    short_height = n // 3
    inp.line(*(short_height + 1 if i < tall_count else short_height - 1 for i in range(n)))
    even_pairs = sum(comb(tall_count, count) * comb(n - tall_count, 2 - count)
                     for count in range(3) if (short_height + count) % 2 == 0)
    out.line(n * even_pairs * pow(comb(n, 2), -1, modulus) % modulus)


def _krawtchouk_transform(plan, inp, out):
    n, k = (plan["dimensions"][key] for key in ("n", "k"))
    modulus = 998244353
    inp.line(n, k)
    if plan["scenario"] == "maximum_dense_one_flip":
        weights = [(7 * i) % 9999 + 1 for i in range(n + 1)]
        weights[0] += 10**9 - sum(weights)
        inp.line(*weights)
        divisor = pow(n * 10**9, -1, modulus)
        result = [0] * (n + 1)
        for i, weight in enumerate(weights):
            if i > 0:
                result[i - 1] += i * weight
            if i < n:
                result[i + 1] += (n - i) * weight
        out.line(*(value * divisor % modulus for value in result))
        return
    inp.line(10**9, *([0] * n))
    half = n // 2
    factorial = [1] * (n + 1)
    for i in range(1, n + 1):
        factorial[i] = factorial[i - 1] * i % modulus
    inverse_factorial = [1] * (n + 1)
    inverse_factorial[-1] = pow(factorial[-1], -1, modulus)
    for i in range(n, 0, -1):
        inverse_factorial[i - 1] = inverse_factorial[i] * i % modulus
    amplitude = factorial[-1] * inverse_factorial[half]**2 * pow(2, -n, modulus) % modulus
    result = [0] * (n + 1)
    for i in range(half + 1):
        binomial = factorial[half] * inverse_factorial[i] * inverse_factorial[half - i]
        result[2 * i] = (amplitude if i % 2 else -amplitude) * binomial % modulus
    result[0] = (result[0] + 1) % modulus
    out.line(*result)


def _polynomial_differential_operator(plan, inp, out):
    n, m, degree, constant = (plan["dimensions"][key] for key in ("n", "m", "degree", "constant"))
    mod = 998244353
    inp.line(n, m)
    inp.line(constant, mod - degree, *([0] * (m - 2)))

    # When a(x)=constant-degree*x, all monomials through x^degree stay in this finite space.
    size = degree + 1
    matrix = [[0] * size for _ in range(size)]
    for i in range(size):
        matrix[i][i] = constant
        if i != 0:
            matrix[i - 1][i] = -i % mod
        if i != degree:
            matrix[i + 1][i] = (i - degree) % mod
    values = [0, 1] + [0] * (degree - 1)
    power = n
    while power:
        if power & 1:
            values = [sum(a * b for a, b in zip(row, values, strict=True)) % mod for row in matrix]
        columns = list(zip(*matrix))
        matrix = [[sum(x * y for x, y in zip(row, col, strict=True)) % mod for col in columns] for row in matrix]
        power //= 2
    out.line(*((values + [0] * (n + 2))[:n + 2]))


def _reeds_sloane(plan, inp, out):
    dims = plan["dimensions"]
    n, first, step, modulus = (dims[key] for key in ("l", "a", "b", "m"))
    inp.line(n, first, step, modulus)

    def matmul(left, right):
        return [[sum(left[i][k] * right[k][j] for k in range(3)) % modulus
                 for j in range(3)] for i in range(3)]

    answer = 0
    for digits in range(1, 19):
        limit = 10 ** digits
        count = min(n, max(0, (limit + step - 1 - first) // step))
        if count == 0:
            continue
        base = [[limit % modulus, 1, 0], [0, 1, step % modulus], [0, 0, 1]]
        result = [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
        exponent = count
        while exponent:
            if exponent & 1:
                result = matmul(result, base)
            base = matmul(base, base)
            exponent //= 2
        answer = (result[0][0] * answer + result[0][1] * first + result[0][2]) % modulus
        first += count * step
        n -= count
        if n == 0:
            break
    out.line(answer)


def _short_simple_path(plan, inp, out):
    size = plan["dimensions"]
    n, left, right, extra, m = (size[key] for key in ("n", "l", "r", "extra", "m"))
    pentagon = plan["scenario"] == "maximum_sole_rooted_pentagon"
    assert m == left * right + extra + 5 * int(pentagon)
    inp.line(n, m)
    for u in range(1, left + 1):
        for v in range(left + 1, left + right + 1):
            inp.line(u, v)
    for u in range(1, extra + 1):
        inp.line(u, left + right + 1)
    if pentagon:
        cycle = [1, n - 3, n - 2, n - 1, n, 1]
        for a, b in zip(cycle, cycle[1:]):
            inp.line(a, b)
    out.line("YES" if pentagon else "NO")


def _round_robin_schedule(plan, inp, out):
    n = plan["dimensions"]["n"]
    number_of_pairs = plan["dimensions"]["pairs"]
    requested = list(range(n))
    for i in range(number_of_pairs):
        j = n - 1 - i
        requested[i], requested[j] = j, i
    inp.line(1)
    inp.line(n)
    inp.line(*(i + 1 for i in requested))
    out.line("Yes")
    fixed = [i for i in range(n) if requested[i] == i]
    normalized = [0] * n
    cycle = n - 1
    first_round = [(0, cycle), *((i, cycle - i) for i in range(1, cycle // 2 + 1))]
    reordered = list(zip(range(number_of_pairs), range(n - 1, n - number_of_pairs - 1, -1), strict=True))
    reordered.extend(zip(fixed[::2], fixed[1::2], strict=True))
    for (u, v), (x, y) in zip(reordered, first_round, strict=True):
        normalized[u], normalized[v] = x, y
    for u in range(n):
        row = []
        for v in range(n):
            if u == v:
                color = 0 if requested[u] == u else cycle
            elif normalized[u] == cycle or normalized[v] == cycle:
                color = min(normalized[u], normalized[v])
            else:
                color = (normalized[u] + normalized[v]) * ((cycle + 1) // 2) % cycle
            if u != v and color == 0 and requested[u] == u:
                color = cycle
            row.append(color + 1)
        out.line(*row)


def _xor_convolution_without_division(plan, inp, out):
    dimension = plan["dimensions"]["n"]
    length = 1 << dimension
    inp.line(dimension)
    state = 189137299
    packed = []
    for _ in range(length):
        state = (1664525 * state + 1013904223) & 0xffffffff
        packed.append(state)
        inp.line(*(state >> bit & 1 for bit in range(32)))

    if plan["scenario"] == "maximum_last_index_polynomial_impulse":
        for i in range(length):
            inp.line(*([0] * 31), int(i == length - 1))
        for value in reversed(packed):
            out.line(*([0] * 31), *(value >> bit & 1 for bit in range(32)))
        return

    def carryless(a, b):
        result = 0
        while b:
            if b & 1:
                result ^= a
            a <<= 1
            b >>= 1
        return result

    periodic = (0xeea394cd, 0x815ef302, 0x17cefe12, 0xcbed79a4,
                0xa81478fe, 0xefca3418, 0x1a588034, 0xfde12a9c)
    for i in range(length):
        inp.line(*(periodic[i & 7] >> bit & 1 for bit in range(32)))
    folded = [0] * 8
    for i, value in enumerate(packed):
        folded[i & 7] ^= value
    rows = []
    for i in range(8):
        result = 0
        for j, value in enumerate(folded):
            result ^= carryless(value, periodic[i ^ j])
        rows.append(tuple(result >> bit & 1 for bit in range(63)))
    for i in range(length):
        out.line(*rows[i & 7])


def _exponential_coefficient_transform(plan, inp, out):
    n = plan["dimensions"]["n"]
    m = plan["dimensions"]["m"]
    offset = plan["dimensions"]["offset"]
    mod = 998244353
    delta = (100000000 - offset) * pow(100000000, -1, mod) % mod
    inp.line(n, m)
    inp.line(*([offset] * m))
    for i in range(1, n + 1):
        out.line(delta * pow(delta + i, i - 1, mod) * pow(i, -i, mod) % mod)


def _degree_sequence(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    changed = plan["scenario"] == "one_missing_degree_to_complete_graph"
    inp.line(n - 2 if changed else n - 1)
    for _ in range(n - 1):
        inp.line(n - 1)
    out.line("NO" if changed else "YES")


def generate(plan, inp, out):
    generators = {
        "subset_xor_square_range_yuki3792": _subset_xor_square_range,
        "josephus_survivor_aoj0085": _josephus_survivor,
        "dyadic_interval_cover_abc349_d": _dyadic_interval_cover,
        "divisibility_chain_ends_arc116_c": _divisibility_chain_ends,
        "pairwise_coprime_progression_yuki3832": _pairwise_coprime_progression,
        "tree_subset_medians_yuki3834": _tree_subset_medians,
        "weighted_taylor_shifts_yuki3793": _weighted_taylor_shifts,
        "dice_orientation_yuki1572": _dice_orientation,
        "line_chip_firing_yuki3801": _line_chip_firing,
        "hub_sparse_distances_yuki3810": _hub_sparse_distances,
        "histogram_rook_cover_agc041_f": _histogram_rook_cover,
        "ferrers_rook_abc272_ex": _ferrers_rook,
        "krawtchouk_transform_arc133_f": _krawtchouk_transform,
        "polynomial_differential_operator_yuki2583": _polynomial_differential_operator,
        "reeds_sloane_abc129_f": _reeds_sloane,
        "short_simple_path_yuki408": _short_simple_path,
        "round_robin_schedule_arc208_d": _round_robin_schedule,
        "xor_convolution_without_division_yuki1901": _xor_convolution_without_division,
        "exponential_coefficient_transform_agc076_d": _exponential_coefficient_transform,
        "degree_sequence_yahoo2018_e": _degree_sequence,
        "polynomial_recurrence_fit_abc222_h": _polynomial_recurrence,
        "polynomial_moments_yuki1875": _polynomial_moments,
        "prefix_dominant_compositions_yuki3480": _prefix_dominant_compositions,
        "domino_tableau_yuki2149": _domino_tableau,
        "interval_matrix_rank_abc399_g": _interval_matrix_rank,
        "multiset_permutation_abc202_d": _multiset_permutation,
        "subarray_aggregate_runs_yuki1036": _aggregate_runs,
        "polynomial_recurrence_abc222_h": _polynomial_recurrence,
        "polynomial_matrix_product_arc174_c": _polynomial_matrix_product,
        "adjacent_swap_parity_arc214_e": _adjacent_swap_parity,
        "matrix_pencil_determinant_yuki1303": _matrix_pencil_determinant,
        "tridiagonal_matrix_power_arc228_e": _tridiagonal_matrix_power,
        "modular_shift_sums_abc483_f": _modular_shift_sums,
        "modular_shift_mex_abc465_g": _modular_shift_mex,
        "modular_shift_minimum_arc182_e": _modular_shift_minimum,
        "lcm_spanning_tree_yuki3827": _lcm_spanning_tree,
        "permutation_comparison_count_dp_t": _permutation_comparisons,
        "alien_trick_abc462_f": _alien_trick,
        "subtraction_game_dp_k": _subtraction_game,
        "xor_two_independent_sets_game_x": _xor_two_independent_sets,
        "linear_matroid_parity_abc482_g": _linear_matroid_parity,
        "disk_lattice_points_abc191_d": _disk_lattice_points,
        "hamiltonian_route_abc180_e": _hamiltonian_route,
        "nonadjacent_selection_abc218_h": _nonadjacent_selection,
        "matrix_permanent_dp_o": _matrix_permanent,
        "exact_cover_abc345_d": _exact_cover,
        "maximum_submatrix_awc0097_d": _maximum_submatrix,
        "digit_dp_dp_s": _digit_dp,
        "sparse_knapsack_yuki626": _sparse_knapsack,
        "bounded_composition_counts_dp_m": _bounded_compositions,
        "subarray_extrema_agc005_b": _subarray_extrema,
        "largest_histogram_rectangle_abc189_c": _histogram_rectangle,
        "knapsack_by_value_dp_e": _knapsack_by_value,
        "top_k_cartesian_sums_abc123_d": _cartesian_sums,
        "optimal_adjacent_merge_dp_n": _adjacent_merge,
        "floor_sum_moments_abc402_g": _floor_moments,
        "floor_path_product_abc429_g": _floor_path,
        "relaxed_convolution_abc315_ex": _relaxed,
        "permutation_rank_decimal_yuki696": _mixed_radix,
    }
    generators[plan["verify_id"]](plan, inp, out)
