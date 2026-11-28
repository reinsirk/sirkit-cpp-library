# SPDX-License-Identifier: CC0-1.0
"""Deterministic whole-program workloads for non-Library Checker graph Verify entries."""
from __future__ import annotations

from array import array
from itertools import islice, permutations
from math import comb, factorial
import random

LIBRARY_VERIFY_IDS = {
    "Algorithm/Interval_Cover_Subsets.cpp": "interval_cover_subsets",
    "DataStructure/Component_Add_Union_Find.cpp": "component_add_union_find",
    "Graph/Affine_Walk_Ranges.cpp": "affine_walk_ranges",
    "Graph/Automaton_Shortest_Path.cpp": "automaton_shortest_path_hopscotch",
    "Graph/Binary_Tree_Traversal.cpp": "binary_tree_traversal",
    "Graph/Bipartite_Coloring.cpp": "bipartite_coloring",
    "Graph/Bipartite_Matching_Analysis.cpp": "bipartite_matching_allowed_edges",
    "Graph/Bitwise_Path.cpp": "bitwise_path",
    "Graph/Block_Cut_Forest.cpp": "block_cut_forest",
    "Graph/Bottleneck_Pair_Histogram.cpp": "bottleneck_pair_histogram",
    "Graph/Bridge_Block_Orientation.cpp": "bridge_block_orientation",
    "Graph/Chinese_Postman_Route.cpp": "chinese_postman_route",
    "Graph/Complete_Binary_Tree.cpp": "complete_binary_tree",
    "Graph/Core_Decomposition.cpp": "core_decomposition",
    "Graph/Complement_Shortest_Paths.cpp": "complement_shortest_paths",
    "Graph/DAG_Critical_Path_Schedule.cpp": "dag_critical_path_schedule",
    "Graph/DAG_Grundy.cpp": "dag_grundy",
    "Graph/DSU_On_Tree.cpp": "dsu_on_tree_mode",
    "Graph/Difference_Constraints.cpp": "difference_constraints",
    "Graph/Directed_Game.cpp": "directed_game",
    "Graph/Directed_Return_Distances.cpp": "directed_return_distances",
    "Graph/Dynamic_Color_Component_Sum.cpp": "dynamic_color_component_sum",
    "Graph/Dynamic_Edge_Products.cpp": "dynamic_edge_products",
    "Graph/Dynamic_Tree_Distances.cpp": "dynamic_tree_distances",
    "Graph/Edge_Removal_Distances.cpp": "edge_removal_distances",
    "Graph/Enumerate_Topological_Orders.cpp": "enumerate_topological_orders",
    "Graph/Equal_Tree_Path_Partition.cpp": "equal_tree_path_partition",
    "Graph/Eulerian_Trail_Decomposition.cpp": "eulerian_trail_decomposition",
    "Graph/Exact_Length_Walk_Fold.cpp": "exact_length_walk_bottleneck",
    "Graph/Forest_Coloring.cpp": "forest_edge_coloring",
    "Graph/Forest_Independent_Set.cpp": "forest_independent_set",
    "Graph/Forest_Subset_Components.cpp": "forest_subset_components",
    "Graph/Functional_Graph_Fold.cpp": "functional_graph_fold",
    "Graph/Graph_Metric_Realization.cpp": "graph_metric_realization",
    "Graph/Grid_Slide_Distances.cpp": "grid_slide_distances",
    "Graph/Grid_Stride_Distances.cpp": "grid_stride_distances",
    "Graph/Hypergraph_Shortest_Paths.cpp": "hypergraph_shortest_paths_merge",
    "Graph/Incident_Label_Spanning_Tree.cpp": "incident_label_spanning_tree",
    "Graph/Incremental_APSP.cpp": "incremental_apsp",
    "Graph/Incremental_Bridges.cpp": "incremental_bridges",
    "Graph/Incremental_Weighted_Forest.cpp": "incremental_weighted_forest",
    "Graph/Johnson_Shortest_Paths.cpp": "johnson_shortest_paths",
    "Graph/K_Shortest_Simple_Paths.cpp": "k_shortest_simple_paths",
    "Graph/Kruskal_Reconstruction_Tree.cpp": "kruskal_reconstruction_tree",
    "Graph/Lexicographic_Shortest_Path.cpp": "lexicographic_shortest_path",
    "Graph/Lower_Bound_Flow.cpp": "lower_bound_flow",
    "Graph/MST_Sensitivity.cpp": "mst_sensitivity_delete",
    "Graph/Near_Tree_Distances.cpp": "near_tree_distances",
    "Graph/Neighborhood_Assignment.cpp": "neighborhood_assignment",
    "Graph/Offline_DAG_Reachability.cpp": "offline_dag_reachability",
    "Graph/Path_Matching.cpp": "path_matching_candies",
    "Graph/Palindromic_Shortest_Paths.cpp": "palindromic_shortest_paths",
    "Graph/Parity_Orientation.cpp": "parity_orientation",
    "Graph/Vertex_Deletion_Components.cpp": "vertex_deletion_components",
    "Graph/Virtual_Tree.cpp": "virtual_tree_distance_product",
    "Graph/Range_Edge_Graph.cpp": "range_edge_graph_express",
    "Graph/Reachability_Chain_Cover.cpp": "reachability_chain_cover",
    "Graph/Resource_Constrained_Paths.cpp": "resource_constrained_paths",
    "Graph/Shortest_Nonzero_Paths.cpp": "shortest_nonzero_paths",
    "Graph/Shortest_Odd_Cycle.cpp": "shortest_odd_cycle",
    "Graph/Shortest_Path_Edge_Usage.cpp": "shortest_path_edge_usage",
    "Graph/Shortest_Walk_Counts.cpp": "shortest_walk_counts",
    "Graph/Shortest_Weighted_Cycle.cpp": "shortest_weighted_cycle",
    "Graph/Subsequence_Shortest_Paths.cpp": "subsequence_shortest_paths",
    "Graph/Strong_Connectivity_Augmentation.cpp": "strong_connectivity_augmentation",
    "Graph/Subcubic_Tree_Biconnectivity.cpp": "subcubic_tree_biconnectivity",
    "Graph/Superincreasing_Path_Queries.cpp": "superincreasing_path_queries",
    "Graph/Subtree_Edge_Add_Distances.cpp": "subtree_edge_add_distances",
    "Graph/Time_Dependent_Paths.cpp": "time_dependent_paths_latest",
    "Graph/Topological_Order_Counter.cpp": "topological_order_counter",
    "Graph/Tree_Distance_Layers.cpp": "tree_distance_layers",
    "Graph/Tree_Edge_Color_Path_Counts.cpp": "tree_edge_color_path_counts",
    "Graph/Tournament_Strong_Counts.cpp": "tournament_strong_counts",
    "Graph/Tournament_Component_Sums.cpp": "tournament_component_sums",
    "Graph/Tree_Backedge_Absorption.cpp": "tree_backedge_absorption",
    "Graph/Tree_Distance_Sums.cpp": "tree_distance_sums",
    "Graph/Static_Top_Tree.cpp": "static_top_tree",
    "Graph/Tree_Marked_Diameter.cpp": "tree_marked_diameter",
    "Graph/Tree_Walk_Orientation.cpp": "tree_walk_orientation",
    "Graph/Tree_Heap_Orderings.cpp": "tree_heap_orderings",
    "Graph/Tree_Metric.cpp": "tree_metric",
    "Graph/Tree_Mo.cpp": "tree_mo",
    "Graph/Tree_Nearest_Marked.cpp": "tree_nearest_marked",
    "Graph/Tree_Path_Bitwise_Counts.cpp": "tree_path_bitwise_counts_and",
    "Graph/Tree_Path_Difference.cpp": "tree_path_difference_road",
    "Graph/Tree_Permutation_Distance.cpp": "tree_permutation_distance",
    "Graph/Two_Nearest_Source_Groups.cpp": "two_nearest_source_groups",
    "Graph/Undirected_Pseudoforest.cpp": "undirected_pseudoforest",
    "Graph/Undirected_Walk_Lengths.cpp": "undirected_walk_lengths",
    "Graph/Undirected_Xor_Walks.cpp": "undirected_xor_walks",
    "Graph/Unit_Step_Forest_Labels.cpp": "unit_step_forest_labels",
    "Graph/Unrooted_Tree_Isomorphism.cpp": "unrooted_tree_leaf_deletion",
    "Graph/Walk_Weight_Congruence.cpp": "walk_weight_congruence_return",
    "Graph/Walk_Weight_Ranges.cpp": "walk_weight_ranges",
    "Graph/Xor_Walk_Reachability.cpp": "xor_walk_reachability",
    "Graph/Dynamic_Steiner_Subtree.cpp": "dynamic_steiner_subtree",
}

PROFILES = {
    "tree_heap_orderings": (
        ("maximum_path_binomial_interleavings", {"n": 200000}, {"n": 11},
         "For root i, the left and right paths are fixed chains; their interleavings form one binomial row."),
        ("maximum_star_center_and_leaf_factorials", {"n": 200000}, {"n": 12},
         "With a center root all leaves can be permuted; a leaf root fixes the center as the second vertex.")),
    "difference_constraints": (
        ("maximum_deep_equalities_pin_target_far_away", {"n": 2000}, {"n": 8},
         "Zero-length links force all points to equal the fixed endpoint at distance one hundred million."),
        ("maximum_conflicting_pins_at_opposite_chain_ends", {"n": 2000}, {"n": 9},
         "All points must coincide, but the fixed first and last points disagree.")),
    "tree_walk_orientation": (
        ("maximum_chain_repeated_full_diameter", {"n": 2000, "m": 2000}, {"n": 8, "m": 6},
         "Orient two endpoint-to-endpoint requests oppositely to cover all directions on every edge."),
        ("maximum_star_leaf_cycle_and_extra_copy", {"n": 2000, "m": 2000}, {"n": 7, "m": 7},
         "A cyclic sequence of requests visits each leaf once from both directions. The additional request changes no optimum.")),
    "vertex_deletion_components": (
        ("maximum_solid_grid_no_articulation", {"n": 1000000, "h": 1000, "w": 1000}, {"n": 30, "h": 5, "w": 6},
         "A rectangular grid with both sides at least two stays connected after any single vertex deletion."),
        ("maximum_serpentine_path_every_internal_vertex_articulation", {"n": 1000000, "h": 1000, "w": 1000}, {"n": 42, "h": 6, "w": 7},
         "Full rows joined alternately at the left and right form an induced path. Only its two endpoints leave a connected graph.")),
    "subsequence_shortest_paths": (
        ("maximum_path_in_schedule_order", {"n": 200000}, {"n": 9},
         "The only route to the final vertex uses all forward edges in order and has a 64-bit cost."),
        ("maximum_same_graph_reversed_schedule_is_unreachable", {"n": 200000}, {"n": 10},
         "The first forward edge from the source is only offered at the final occurrence.")),
    "tournament_strong_counts": (
        ("maximum_uniform_tournament_sink_size", {"n": 200000}, {"n": 5},
         "Exact finite-field mean from an independent CDQ reciprocal recurrence with ACL convolution."),
        ("precision_crosses_power_of_two", {"n": 131073}, {"n": 6},
         "The same independent CDQ recurrence checks a precision just over a power of two.")),
    "tournament_component_sums": (
        ("maximum_central_orientation_coefficient", {"n": 30}, {"n": 6},
         "The sum of vertex labels in each closed prefix fixes the number of forward cross edges independently of all internal edge orientations."),
        ("maximum_one_reversed_edge_component_lengths", {"n": 30}, {"n": 5},
         "Reversing a nonadjacent pair joins its whole label interval into one SCC. An adjacent reversal preserves a total order.")),
    "path_matching_candies": (
        ("maximum_equal_weight_all_ties", {"n": 200000}, {"n": 12},
         "Every valid collection of j candies weighs j billion."),
        ("maximum_odd_size_last_cardinality_cliff", {"n": 199999}, {"n": 11},
         "Up to floor(N/2) all selected candies can be heavy. The unique ceil(N/2) choice selects only the light candies.")),
    "core_decomposition": (
        ("maximum_edges_4_regular_with_64_bit_scores", {"n": 100000, "m": 200000}, {"n": 9, "m": 18},
         "All vertices initially cost four billion; every deletion thereafter can only lower the costs."),
        ("maximum_star_with_extra_leaf_edge_low_weight_leaves", {"n": 200000, "m": 200000}, {"n": 11, "m": 11},
         "The center costs the total leaf weight and every leaf initially costs at least one billion; after removing the center, only the extra leaf edge remains.")),
    "forest_independent_set": (
        ("maximum_path_fibonacci", {"n": 100000}, {"n": 12},
         "Binary strings without consecutive ones are counted by the (N+2)-th Fibonacci number."),
        ("maximum_star_dominating_center", {"n": 100000}, {"n": 11},
         "All leaf subsets are allowed with the center excluded, and only the center otherwise.")),
    "forest_edge_coloring": (
        ("maximum_path_alternating_colors", {"n": 100000}, {"n": 12},
         "An internal path vertex needs two colors; alternating them suffices."),
        ("maximum_star_distinct_colors", {"n": 100000}, {"n": 11},
         "All edges meet the center and must receive different colors.")),
    "forest_subset_components": (
        ("maximum_chain_by_alternating_run_boundaries", {"n": 5000}, {"n": 12},
         "A subset with k runs of selected vertices is specified by choosing 2k of the N+1 boundaries."),
        ("maximum_star_by_center_selected_or_not", {"n": 5000}, {"n": 11},
         "A chosen center joins all chosen leaves. Without it, each chosen leaf is a separate component.")),
    "incident_label_spanning_tree": (
        ("maximum_single_seed_propagates_along_entire_path", {"n": 200000}, {"n": 12},
         "The unique admissible witness keeps the first N−1 edges. The extra blue edge cannot cover the first vertex."),
        ("maximum_every_vertex_has_an_eligible_edge_but_no_seed", {"n": 200000}, {"n": 13},
         "Every edge can cover at most one endpoint, so N−1 selected edges cannot cover all N vertices.")),
    "interval_cover_subsets": (
        ("maximum_distinct_endpoints_all_adjacencies_required", {"n": 200000, "m": 200000}, {"n": 8, "m": 8},
         "All unit intervals are mandatory except for the first duplicated interval, which has three nonempty choices."),
        ("maximum_two_half_covers_with_many_duplicate_options", {"n": 200000, "m": 200000}, {"n": 10, "m": 12},
         "Each half must choose a nonempty subset from its own group of copies independently.")),
    "strong_connectivity_augmentation": (
        ("maximum_deep_dag_with_one_source_and_one_sink", {"n": 10000, "m": 100000}, {"n": 11, "m": 25},
         "The chain supplies reachability from the first city to every city and from every city to the last."),
        ("maximum_unequal_bilayer_dag", {"n": 10000, "m": 100000}, {"n": 12, "m": 18},
         "Each sink needs a new outgoing arc and there are more sinks than sources.")),
    "affine_walk_ranges": (
        ("maximum_alternating_chain_with_one_finite_reflection", {"n": 200000}, {"n": 9},
         "The chain appends one symbol per step. A self-loop at its end adds one symbol or cancels that last symbol."),
        ("maximum_alternating_chain_with_drift_cycle_and_long_suffix", {"n": 200000}, {"n": 10},
         "An alternating two-edge cycle pumps the word length from its first vertex through the rest of the chain.")),
    "superincreasing_path_queries": (
        ("maximum_one_long_cycle_every_edge_excluded_once", {"n": 200000, "m": 200000, "q": 200000},
         {"n": 9, "m": 9, "q": 9},
         "Removing a chain edge forces the closing edge; otherwise the chain is shorter than the closing edge."),
        ("maximum_local_chords_many_distinct_replacement_edges", {"n": 100000, "m": 200000, "q": 200000},
         {"n": 8, "m": 15, "q": 36},
         "The first span-two chord crossing a failed chain edge is its replacement; removing a chord leaves the chain intact.")),
    "chinese_postman_route": (
        ("maximum_eulerian_cycle_with_no_repeated_edges", {"n": 100000, "weight": 1000000, "leaves": 0},
         {"n": 8, "weight": 7, "leaves": 0},
         "A connected even-degree cycle needs to traverse every edge just once."),
        ("maximum_sixteen_odds_many_bridges_and_parallel_edge", {"n": 100000, "weight": 1000000, "leaves": 14},
         {"n": 9, "weight": 3, "leaves": 4},
         "Every bridge is crossed twice; the only cycle consists of two parallel edges used once each.")),
    "tree_backedge_absorption": (
        ("maximum_chain_and_repeated_root_resets", {"n": 250000}, {"n": 12},
         "A reset starts another attempt; the survival probability is the product of reciprocal outdegrees."),
        ("maximum_two_long_arms_with_disjoint_ancestor_coefficients", {"n": 250000}, {"n": 17},
         "A reset to either half of the initial stem costs its remaining length before another attempt at the fork.")),
    "tree_edge_color_path_counts": (
        ("maximum_chain_alternates_two_colors", {"n": 200000}, {"n": 14},
         "Only the adjacent vertex pairs have one color; every longer path contains both alternating colors."),
        ("maximum_hub_two_edge_arms_distinct_colors", {"n": 200000}, {"n": 14},
         "All colors differ, so exactly the pairs at distance two contribute. They meet at the hub or at an arm's midpoint.")),
    "dynamic_edge_products": (
        ("maximum_star_heavy_cache_with_last_deletion", {"n": 100000, "q": 100000}, {"n": 24, "q": 24},
         "A star with e active edges has total degree product e²; the last operation deletes the first edge."),
        ("maximum_balanced_bipartite_rows_stress_light_scans", {"n": 100000, "q": 100000, "columns": 400}, {"n": 24, "q": 24, "columns": 6},
         "Completed left rows own all column incidences, and the partial row uses the higher-degree columns.")),
    "directed_return_distances": (
        ("maximum_heavy_functional_cycle", {"n": 2000, "weight": 100000}, {"n": 10, "weight": 100},
         "With one outgoing edge everywhere, the only nonempty simple return takes all N edges."),
        ("maximum_branching_scc_with_one_shortcut", {"n": 2000, "weight": 100000}, {"n": 10, "weight": 100},
         "The shortcut avoids only vertex 2; vertex 2 must take the full ring, and the isolated vertex cannot return.")),
    "tree_distance_sums": (
        ("maximum_long_chain_large_repeated_leaf_updates", {"n": 200000, "q": 200000}, {"n": 14, "q": 24},
         "The leaf weight is multiplied by the arithmetic-series sum of distances on the entire chain."),
        ("maximum_star_two_leaf_endpoints_and_center", {"n": 200000, "q": 200000}, {"n": 14, "q": 24},
         "On the three-vertex query path, either leaf contributes sum 3 and the center contributes sum 2.")),
    "bitwise_path": (
        ("maximum_chain_forces_all_thirty_bits", {"n": 200000, "bits": 30}, {"n": 40, "bits": 6},
         "Every path crosses the final independent label bits; the sole parallel edge cannot bypass them."),
        ("maximum_sparse_graph_avoids_high_bit_shortcut", {"n": 200000, "bits": 30}, {"n": 40, "bits": 6},
         "The direct edge contains only the largest bit; the spanning path uses bits 0 and 1 only.")),
    "graph_metric_realization": (
        ("maximum_complete_metric_every_edge_required", {"n": 300}, {"n": 7},
         "All nonzero distances are 10^9, so the sum via any third vertex is strictly larger."),
        ("maximum_path_metric_only_consecutive_edges_required", {"n": 300}, {"n": 7},
         "Consecutive vertices force one unit edge and every longer interval can be split along the path.")),
    "tree_permutation_distance": (
        ("maximum_even_path_two_centroids", {"n": 5000}, {"n": 8},
         "Every assignment crosses the central edge, leaving two independent permutations of size n/2."),
        ("maximum_star_many_centroid_branches", {"n": 5000}, {"n": 8},
         "No leaf may be fixed. Count complete derangements plus those on the leaves with the center fixed.")),
    "shortest_odd_cycle": (
        ("maximum_odd_ring_below_bridge_budget", {"n": 2000, "length": 1998}, {"n": 8, "length": 6},
         "The endpoints form their own edge. Both equal-length detours of the large odd ring require two bridge crossings too."),
        ("maximum_bipartite_ring_before_remote_triangle", {"n": 2000, "length": 10**18}, {"n": 12, "length": 6},
         "A bridge to the independent triangle gives an even six-step route, with longer even lengths by backtracking.")),
    "binary_tree_traversal": (
        ("maximum_left_chain", {"n": 200000}, {"n": 9},
         "Ascending preorder and descending inorder force a left child at every step."),
        ("maximum_right_chain_with_contradictory_tail", {"n": 200000}, {"n": 9},
         "After the right chain, its final root must visit the last label's subtree before the next preorder label.")),
    "tree_metric": (
        ("maximum_consistent_path_matrix", {"n": 3000}, {"n": 9},
         "Absolute differences of vertex indices are the distances of a unit-weight path."),
        ("maximum_star_matrix_corrupted_at_last_pair", {"n": 3000}, {"n": 9},
         "Distance one to the center forces all star edges. The last leaf pair may only have distance two.")),
    "exact_length_walk_bottleneck": (
        ("maximum_complete_graph_fills_matrix_powers", {"n": 100, "length": 1000000000}, {"n": 8, "length": 19},
         "The first incoming edge of each vertex follows the source loop in the activation order."),
        ("maximum_length_cycle_with_single_endpoint", {"n": 100, "length": 1000000000}, {"n": 8, "length": 19},
         "A directed cycle fixes the final vertex by modular arithmetic, and every edge must activate.")),
    "exact_length_walk_weighted_count": (
        ("maximum_all_to_all_loopless_graph", {"n": 100, "length": 1000000000}, {"n": 8, "length": 19},
         "The loopless complete graph has eigenvalues n-1 and -1, giving the closed-form count to a different vertex."),
        ("maximum_length_uniform_weight_cycle", {"n": 100, "length": 1000000000}, {"n": 8, "length": 19},
         "The cycle forces a unique walk to the target; its uniform weight is raised to the exact walk length.")),
    "undirected_walk_lengths": (
        ("maximum_even_cycle_wrong_parity", {"n": 100000, "length": 1000000000}, {"n": 8, "length": 20},
         "An even cycle alternates colors, so adjacent endpoints cannot be joined by an even number of edges."),
        ("maximum_long_tail_with_odd_cycle", {"n": 100000, "length": 1000000000}, {"n": 8, "length": 20},
         "Every vertex is on the spanning path, and the source triangle changes parity before entering the path.")),
    "subcubic_tree_biconnectivity": (
        ("maximum_path_closure_with_expensive_ends", {"n": 200000}, {"n": 6},
         "The two endpoints require one incidence each and the closing edge alone yields a cycle."),
        ("maximum_caterpillar_with_antipodal_leaf_edges", {"n": 200000}, {"n": 8},
         "Pairing opposite leaves meets the leaf-incidence lower bound. Removing a spine vertex leaves two sides joined by a separate pair.")),
    "resource_constrained_paths": (
        ("maximum_rail_budget_with_unit_exchanges", {"n": 50, "m": 100}, {"n": 8, "m": 15},
         "Each necessary chain hop uses 50 coins, and replacing each coin takes one minute."),
        ("maximum_initial_coins_and_expensive_rail_star", {"n": 50, "m": 100}, {"n": 8, "m": 15},
         "Every city is one direct maximum-time hop away, and the initial coins cover any fare.")),
    "unrooted_tree_leaf_deletion": (
        ("maximum_path_endpoints", {"n": 200000}, {"n": 15},
         "Deleting either endpoint gives the shorter path; there are no other leaves."),
        ("maximum_star_all_leaves", {"n": 200000}, {"n": 15},
         "Every leaf of the larger star is a valid deletion, so the output has N sorted IDs.")),
    "component_add_union_find": (
        ("growing_component_then_new_incidence", {"n": 500000, "q": 500000}, {"n": 40, "q": 44},
         "Each newcomer inherits only its own later increments, while the first vertex receives all increments."),
        ("balanced_merges_of_preincremented_vertices", {"n": 500000, "q": 500000}, {"n": 40, "q": 44},
         "Balanced merges preserve every old signed value; a final component increment affects that component only.")),
    "hypergraph_shortest_paths_merge": (
        ("long_chain_with_large_side_clique", {"n": 200000, "groups": 200000, "memberships": 500000},
         {"n": 20, "groups": 20, "memberships": 50},
         "The large clique meets the chain at only one articulation and cannot shorten it."),
        ("disjoint_dense_groups", {"n": 200000, "groups": 200000, "memberships": 500000},
         {"n": 20, "groups": 20, "memberships": 50},
         "The first and second halves are each traversable, but share no group members.")),
    "shortest_nonzero_paths": (
        ("long_heavy_parallel_chain", {"n": 100000, "m": 200000, "bits": 30},
         {"n": 12, "m": 24, "bits": 6},
         "One more unit of cost allows using a nonzero parallel edge on the unique vertex path."),
        ("articulation_forbids_nonzero_detours", {"n": 100000, "m": 200000, "bits": 30},
         {"n": 12, "m": 24, "bits": 6},
         "Only the labeled parallel edge to a leaf changes the label; detouring from any other target would revisit the articulation.")),
    "dsu_on_tree_mode": (
        ("long_given_chain_unicolor", {"n": 2500000, "given": 100000}, {"n": 28, "given": 16},
         "All vertices share one color, so the mode frequency is exactly the subtree size."),
        ("wide_given_star_two_colors", {"n": 2500000, "given": 100000}, {"n": 28, "given": 16},
         "A reverse traversal directly counts the two colors; equal positive counts give two modes.")),
    "unit_step_forest_labels": (
        ("deep_chain_forced_rising_assignment", {"n": 100000}, {"n": 14},
         "The end labels differ by the full tree diameter, forcing every unit change to increase."),
        ("broad_star_conflicting_leaf_intervals", {"n": 100000}, {"n": 14},
         "Leaves carrying zero and four impose disjoint sets on their common center.")),
    "walk_weight_congruence_return": (
        ("large_period_with_only_two_and_five", {"n": 200000, "m": 200000}, {"n": 20, "m": 20},
         "All return lengths are multiples of the cycle length, which has only factors two and five."),
        ("small_period_three_and_long_outgoing_tail", {"n": 200000, "m": 200000}, {"n": 20, "m": 20},
         "Exactly the initial cycle matters. Its period is three, even though almost every vertex is reachable.")),
    "tree_path_difference_road": (
        ("deep_chain_full_and_suffix_updates", {"n": 50000, "q": 50000, "r": 50000},
         {"n": 13, "q": 16, "r": 18},
         "With strictly increasing base weights and a second set of updates on a suffix, the first edge determines the minimum."),
        ("wide_star_shared_spoke_updates", {"n": 50000, "q": 50000, "r": 50000},
         {"n": 13, "q": 16, "r": 18},
         "Every update touches the shared spoke at vertex 2 and exactly one other spoke, which can be counted by leaf incidence.")),
    "subtree_edge_add_distances": (
        ("deep_chain_nested_updates", {"n": 100000, "q": 100000}, {"n": 17, "q": 30},
         "Each nested update contributes its value times the number of affected edges on the root path."),
        ("broad_star_root_and_leaf_updates", {"n": 100000, "q": 100000}, {"n": 17, "q": 30},
         "Leaf updates touch no edges; every root update raises every spoke by the same value.")),
    "automaton_shortest_path_hopscotch": (
        ("long_ring_revisiting_goal", {"n": 100000, "m": 100000}, {"n": 10, "m": 10},
         "The first visit to N-2 is after N-3 arcs; two full laps restore residue zero."),
        ("reachable_goal_with_wrong_residue", {"n": 100000, "m": 99999}, {"n": 10, "m": 9},
         "The only cycle has length divisible by three, preserving the wrong residue at vertex 2.")),
    "bipartite_coloring": (
        ("even_ring", {"n": 200000, "m": 200000}, {"n": 42, "m": 42},
         "Alternating labels 0 and 1 around an even cycle satisfy every constraint."),
        ("long_odd_cycle", {"n": 200000, "m": 200000}, {"n": 42, "m": 42},
         "A cycle on N-1 vertices is odd; an isolated vertex and a repeated edge do not change that.")),
    "bipartite_matching_allowed_edges": (
        ("unique_triangular_perfect_matching", {"n": 100000}, {"n": 24},
         "Only the diagonal can form a perfect matching in a strictly upper-triangular extension."),
        ("long_alternating_cycle", {"n": 100000}, {"n": 24},
         "The diagonal and the shifted diagonal are two perfect matchings and cover all edges.")),
    "block_cut_forest": (
        ("bridge_chain_with_triangle", {"n": 200000}, {"n": 48},
         "The original chain is a simple path through all query vertices; the first triangle does not remove it."),
        ("dead_end_cycle", {"n": 200000}, {"n": 48},
         "Both query endpoints are leaves of one articulation; the queried internal cycle vertex cannot lie on their path.")),
    "bottleneck_pair_histogram": (
        ("descending_input_increasing_chain", {"n": 100000, "weight": 1}, {"n": 23, "weight": 1},
         "For the i-th weighted edge, exactly i pairs have it as the largest edge on their path."),
        ("maximum_equal_weight_star", {"n": 100000, "weight": 10000000}, {"n": 23, "weight": 10000000},
         "Every pair has the common edge weight as its path maximum.")),
    "bridge_block_orientation": (
        ("complete_equal_reachability", {"n": 100}, {"n": 18},
         "A directed Hamiltonian cycle connects every vertex of the complete graph."),
        ("two_reachability_blocks", {"n": 100}, {"n": 18},
         "Each half is strongly connected and all intergroup arcs point from the first half to the second.")),
    "complete_binary_tree": (
        ("perfect_tree_across_root", {"n": (1 << 59) - 1, "tests": 100000}, {"n": 63, "tests": 41},
         "Exactly the leaves in the opposite root subtree are at the full diameter from any leaf."),
        ("largest_partial_last_level", {"n": 10**18, "tests": 100000}, {"n": 111, "tests": 45},
         "The left half of the last level is full; the rightmost leaf and the root give analytically known counts.")),
    "complement_shortest_paths": (
        ("two_hop_hub", {"n": 200000, "m": 200000}, {"n": 48, "m": 63},
         "The direct edge is missing; only vertices adjacent to both endpoints give shortest two-step paths."),
        ("isolated_terminal", {"n": 200000, "m": 200000}, {"n": 48, "m": 63},
         "Every edge incident on the destination is forbidden, so no path reaches it.")),
    "dag_critical_path_schedule": (
        ("long_critical_chain_with_shortcuts", {"n": 100000, "m": 500000, "duration": 159},
         {"n": 13, "m": 37, "duration": 159},
         "Every forward shortcut skips two or more full-duration steps, leaving the complete chain critical."),
        ("one_critical_branch_among_many", {"n": 100000, "m": 500000, "duration": 159},
         {"n": 13, "m": 37, "duration": 159},
         "Only the branch through vertex 1 uses positive-duration arcs, so exactly three vertices are critical.")),
    "dag_grundy": (
        ("pairs_of_maximum_value", {"n": 100000, "value": 100000, "prime": 99991},
         {"n": 24, "value": 180, "prime": 179},
         "An even number of identical Grundy values cancels to zero."),
        ("one_unpaired_prime", {"n": 100000, "value": 100000, "prime": 99991},
         {"n": 24, "value": 180, "prime": 179},
         "An odd number of the chosen composites and a single prime have different Grundy values.")),
    "directed_game": (
        ("long_forced_shiritori", {"n": 200000}, {"n": 54},
         "Only the next word-state is reachable; winners alternate backwards from the terminal."),
        ("long_shiritori_draw_cycle", {"n": 200000}, {"n": 54},
         "No played state can ever reach a position with no legal move.")),
    "dynamic_tree_distances": (
        ("long_path_full_distance", {"n": 200000, "q": 200000}, {"n": 40, "q": 48},
         "The queried endpoints bracket every edge, so the distance is the running total of all weights."),
        ("star_leaf_distances", {"n": 200000, "q": 200000}, {"n": 40, "q": 48},
         "Two leaf-to-center edges determine every queried leaf-to-leaf distance.")),
    "edge_removal_distances": (
        ("complete_digraph", {"n": 400, "m": 159600}, {"n": 24, "m": 552},
         "Removing the direct edge makes the distance 2; every other deletion leaves it at 1."),
        ("long_path_with_dense_dead_end", {"n": 400, "m": 39603}, {"n": 24, "m": 123},
         "Each path edge is mandatory; the reachable dense subgraph has no route back to the path.")),
    "enumerate_topological_orders": (
        ("deep_chain_with_ten_witnesses", {"n": 100000, "k": 10}, {"n": 8, "k": 10},
         "The initial chain is forced, then four unconstrained tail vertices have 4! legal orderings."),
        ("deep_chain_ending_at_cycle", {"n": 100000, "k": 10}, {"n": 8, "k": 10},
         "All vertices on the chain must be consumed before the remaining two-cycle blocks the order.")),
    "equal_tree_path_partition": (
        ("maximum_chain_groups_of_four", {"n": 200000, "length": 4}, {"n": 12, "length": 4},
         "Cut a chain into consecutive blocks of four vertices."),
        ("maximum_star_pairing_impossible", {"n": 200000, "length": 2}, {"n": 12, "length": 2},
         "At most one leaf can be paired with the center, and the other leaves have no retained edge.")),
    "eulerian_trail_decomposition": (
        ("one_long_directed_cycle", {"n": 300000}, {"n": 8},
         "All edges are already in one Eulerian circuit, so no destination needs changing."),
        ("many_parallel_arcs_and_loops", {"n": 300000}, {"n": 8},
         "Each parallel arc and each isolated loop contributes one separate trail.")),
    "functional_graph_fold": (
        ("long_tail_to_fixed_vertex", {"n": 200000, "k": 10**12}, {"n": 43, "k": 1001},
         "The first N-1 transitions add one candy; every transition on the self-loop adds N."),
        ("weighted_cycle_with_remainder", {"n": 200000, "k": 10**12 - 37}, {"n": 43, "k": 1001},
         "Every transition adds one candy, and each completed lap adds N more at the last vertex.")),
    "grid_slide_distances": (
        ("two_open_diagonal_segments", {"n": 1500}, {"n": 8},
         "The two open diagonals meet one row before the target; the endpoints do not share a diagonal."),
        ("solid_barrier_row", {"n": 1500}, {"n": 8},
         "A full wall row blocks every diagonal route between the two edges of the board.")),
    "grid_stride_distances": (
        ("open_unit_stride", {"n": 1000, "w": 1000, "k": 1}, {"n": 5, "w": 7, "k": 1},
         "An open grid with unit strides has Manhattan distance between opposite corners."),
        ("wall_with_bottom_gap", {"n": 1000, "w": 1000, "k": 1000000}, {"n": 5, "w": 7, "k": 100},
         "The wall forces a descent to the last row, one horizontal slide, and an ascent.")),
    "incremental_apsp": (
        ("dense_graph_isolating_one_city", {"n": 300, "q": 200000}, {"n": 21, "q": 75},
         "With all other roads intact, losing a direct road from city 1 costs one extra step until isolation."),
        ("deleting_edges_along_weighted_chain", {"n": 300, "q": 200000}, {"n": 21, "q": 75},
         "A left-to-right deletion prefix separates old vertices; suffix distances remain multiples of 10^9.")),
    "incremental_bridges": (
        ("last_insert_closes_entire_chain", {"n": 200000}, {"n": 34},
         "Adding the final edge closes the long cycle, making every shortest-path edge non-bridging."),
        ("triangle_with_long_bridge_tail", {"n": 200000}, {"n": 34},
         "Only the first step of the shortest route, taking the triangle shortcut, is a non-bridge.")),
    "incremental_weighted_forest": (
        ("balanced_path_merges", {"n": 200000, "q": 300000}, {"n": 40, "q": 90},
         "Equal-size path pieces merge into one uniform-weight path; remaining vertices stay isolated."),
        ("growing_weighted_star", {"n": 200000, "q": 300000}, {"n": 40, "q": 90},
         "Path cost is the sum of spoke weights; the diameter is the two heaviest spokes.")),
    "johnson_shortest_paths": (
        ("long_chain_at_negative_weight_limit", {"n": 100, "weight": 1000000000000}, {"n": 16, "weight": 1000000000000},
         "The only finite distance is to a later vertex, with the weight times the forward offset."),
        ("complete_graph_with_signed_potentials", {"n": 100, "weight": 1000000000000}, {"n": 16, "weight": 1000000000000},
         "Vertex potential terms telescope, and every extra edge adds a positive constant.")),
    "k_shortest_simple_paths": (
        ("full_edge_budget_equal_diamonds", {"n": 1501, "m": 2000, "k": 10}, {"n": 13, "m": 16, "k": 10},
         "Every stage has two equal-length routes, and all choices give the same total length."),
        ("maximum_vertex_chain_with_shortcut", {"n": 2000, "m": 2000, "k": 10}, {"n": 12, "m": 12, "k": 10},
         "There are exactly two simple routes and the shortcut has the same cost as the two edges it replaces.")),
    "kruskal_reconstruction_tree": (
        ("equal_height_plateau", {"n": 500, "w": 500, "q": 200000, "height": 1000000},
         {"n": 5, "w": 7, "q": 47, "height": 9},
         "All buildings connect at both query floors; only the height difference requires stairs."),
        ("low_crossing_column", {"n": 500, "w": 500, "q": 200000, "height": 1000000},
         {"n": 5, "w": 7, "q": 47, "height": 9},
         "Every left-to-right route crosses a low column of buildings with two floors.")),
    "lexicographic_shortest_path": (
        ("dense_monotone_ties", {"n": 200}, {"n": 8},
         "Complete graph weights are absolute label differences; each next increasing label preserves shortest length."),
        ("two_equal_routes_around_ring", {"n": 200, "weight": 10000}, {"n": 8, "weight": 10000},
         "The two shortest routes are equal around the ring; the first edge to label 1 wins.")),
    "lower_bound_flow": (
        ("all_cells_require_dominoes", {"n": 300, "w": 300}, {"n": 4, "w": 4},
         "Pair neighboring columns in every row to cover all required cells."),
        ("odd_number_of_required_cells", {"n": 300, "w": 300}, {"n": 4, "w": 4},
         "One corner requires a monomino, leaving an odd number of required domino cells.")),
    "mst_sensitivity_delete": (
        ("unit_cycle_heavy_chords", {"n": 100000}, {"n": 32},
         "Removing any one edge leaves a spanning path of unit-cost edges."),
        ("weighted_spokes", {"n": 100000}, {"n": 32},
         "The spokes form the unique cheapest tree; any missing spoke is replaced at cost 10^9.")),
    "near_tree_distances": (
        ("large_cycle", {"n": 100000, "m": 100000, "q": 10000}, {"n": 42, "m": 42, "q": 45},
         "A cycle has two arcs between each pair; the shorter one determines the distance."),
        ("eleven_chords_on_a_path", {"n": 100000, "m": 100010, "q": 10000}, {"n": 42, "m": 52, "q": 45},
         "Every chord touches vertex 1; compare the direct path against the route via vertex 1.")),
    "neighborhood_assignment": (
        ("heavy_star", {"n": 200000, "m": 199999, "q": 200000}, {"n": 31, "m": 30, "q": 43},
         "A leaf updates the center first, then the center gives that leaf's value to all vertices."),
        ("clique_below_threshold", {"n": 200000, "m": 200000, "q": 200000, "clique": 632},
         {"n": 31, "m": 43, "q": 43, "clique": 9},
         "The first query gives the whole clique a common value; isolated vertices and the extra chain stay unchanged.")),
    "offline_dag_reachability": (
        ("long_query_intervals", {"n": 100000, "m": 100000, "q": 100000}, {"n": 48, "m": 56, "q": 64},
         "A forward chain joins every query pair; many distinct endpoints cross the midpoint."),
        ("few_targets_two_chains", {"n": 100000, "m": 100000, "q": 100000}, {"n": 48, "m": 56, "q": 64},
         "Two chains separate vertex parities; every query is answered by parity alone.")),
    "palindromic_shortest_paths": (
        ("monochrome_directed_cycle", {"n": 100}, {"n": 8},
         "Every walk has a constant label string, so distances are cyclic offsets."),
        ("dense_disconnected_label_blocks", {"n": 100}, {"n": 8},
         "Two disjoint complete digraphs use one label; all their direct arcs have length one.")),
    "parity_orientation": (
        ("even_cycle", {"n": 100000}, {"n": 40},
         "Orienting the even cycle from its odd vertices to its even vertices gives even outdegrees."),
        ("odd_edge_count_path", {"n": 100000}, {"n": 40},
         "The number of edges is odd, so the sum of the outdegrees cannot be even.")),
    "virtual_tree_distance_product": (
        ("two_chains", {"n": 200000}, {"n": 31},
         "Exactly 2(n-d) ordered pairs are separated by d in both chains."),
        ("chain_and_star", {"n": 200000}, {"n": 31},
         "Twice the chain distance sum minus distances to and from the star center.")),
    "range_edge_graph_express": (
        ("wide_both_directions", {"n": 100000, "m": 100000}, {"n": 32, "m": 41},
         "A fare-1 eastbound train reaches every station; the triangle inequality gives a matching lower bound."),
        ("singleton_sequential", {"n": 100000, "m": 100000}, {"n": 32, "m": 41},
         "Every eastward path pays each adjacent train's fare in the prefix at least once.")),
    "time_dependent_paths_latest": (
        ("long_chain_of_periodic_trains", {"n": 200000, "period": 10**9, "count": 10**9},
         {"n": 34, "period": 7, "count": 35},
         "Every station before the last connection needs one more period of lead time."),
        ("last_train_misses_every_transfer", {"n": 200000, "period": 1, "count": 1},
         {"n": 34, "period": 1, "count": 1},
         "The one train on each link departs at the same time; only the final link can be taken.")),
    "tree_nearest_marked": (
        ("moving_chain_boundary", {"n": 100000, "q": 100000}, {"n": 48, "q": 59},
         "The closest vertex to every query is the rightmost marked vertex of the path prefix."),
        ("star_leaf_toggles", {"n": 100000, "q": 100000}, {"n": 48, "q": 59},
         "Marked vertices are leaves; distances are 0 to a marked leaf, 1 from the center, and 2 otherwise.")),
    "topological_order_counter": (
        ("one_precedence_at_maximum_n", {"n": 16}, {"n": 8},
         "Exactly half of all permutations put the head of a single constraint after its tail."),
        ("interleaving_two_dense_chains", {"n": 16}, {"n": 8},
         "Each chain must stay in order, so choose positions for the first chain.")),
    "tree_distance_layers": (
        ("deep_chain_endpoint_queries", {"n": 200000, "q": 200000}, {"n": 41, "q": 48},
         "From either endpoint the unique valid answer is the vertex K steps inward."),
        ("wide_star_multiple_witnesses", {"n": 200000, "q": 200000}, {"n": 41, "q": 48},
         "Leaves lie one step from the center and two steps from every other leaf.")),
    "tree_mo": (
        ("periodic_long_paths", {"n": 200000, "q": 200000}, {"n": 48, "q": 64},
         "The path is a contiguous periodic sequence, so its counts differ by at most one."),
        ("star_distinct_labels", {"n": 200000, "q": 200000}, {"n": 48, "q": 64},
         "Every label on the path occurs once; there are 2 or 3 vertices on the path.")),
    "tree_path_bitwise_counts_and": (
        ("maximum_star_same_wide_mask", {"n": 100000, "weight": 1000000000}, {"n": 16, "weight": 1000000000},
         "Every nonempty path consists only of copies of the same mask, so every pair contributes that mask."),
        ("alternating_disjoint_chain_bits", {"n": 100000, "weight": 2}, {"n": 16, "weight": 2},
         "Only adjacent vertices contribute, since consecutive weights share no set bit.")),
    "two_nearest_source_groups": (
        ("far_sources_on_weighted_ring", {"n": 100000, "weight": 1000000000}, {"n": 38, "weight": 1000000000},
         "Each resident may only use the endpoint source of the other half; the ring gives two candidate distances."),
        ("all_vertices_popular_in_star", {"n": 100000, "weight": 1000000000}, {"n": 38, "weight": 1000000000},
         "Residents of the center's country need two spokes except at the center; everyone else can reach the center directly.")),
    "undirected_pseudoforest": (
        ("long_tail_on_triangle", {"n": 200000, "q": 200000}, {"n": 46, "q": 72},
         "The tail attaches at vertex 1. From the other two triangle vertices there are two routes to the tail."),
        ("entire_graph_is_cycle", {"n": 200000, "q": 200000}, {"n": 46, "q": 72},
         "There are exactly two simple paths between any two distinct vertices on the cycle.")),
    "reachability_chain_cover": (
        ("long_chain_with_shortcut", {"n": 7000, "width": 0}, {"n": 47, "width": 0},
         "One directed chain contains every vertex; the shortcut does not change that."),
        ("many_chains_sharing_long_spine", {"n": 7000, "width": 2000}, {"n": 47, "width": 12},
         "The source leaves are an antichain, and each can pair with one sink leaf via the shared spine.")),
    "shortest_path_edge_usage": (
        ("long_unique_chain", {"n": 200000}, {"n": 48},
         "Only the full chain realizes the shortest distance; the direct edge weighs 10^9."),
        ("equal_length_layers", {"n": 100000}, {"n": 46},
         "Two vertices per distance layer and complete links between layers permit avoiding any edge.")),
    "shortest_walk_counts": (
        ("exponential_number_of_paths", {"n": 200000, "layers": 50000}, {"n": 46, "layers": 10},
         "Each shortest walk independently passes through one of two vertices in every layer."),
        ("large_disconnected_destination", {"n": 200000, "layers": 0}, {"n": 46, "layers": 0},
         "The extra edges and the chain reach every vertex except the isolated destination.")),
    "shortest_weighted_cycle": (
        ("weighted_undirected_ring", {"n": 2000, "m": 2000}, {"n": 36, "m": 36},
         "The ring is the only simple cycle; sum its varying positive edge weights."),
        ("heavy_directed_ring", {"n": 2000, "m": 2000}, {"n": 36, "m": 36},
         "Every directed traversal returns only after a full lap, adding N times 10^9.")),
    "undirected_xor_walks": (
        ("distinct_full_width_labels", {"n": 200000, "bits": 30}, {"n": 27, "bits": 5},
         "The star has distinct root XOR labels, so no pair with different vertices has XOR zero."),
        ("full_rank_closed_walks", {"n": 199971, "bits": 30}, {"n": 27, "bits": 5},
         "The chord weights generate every bit; every vertex pair admits a walk with XOR zero.")),
    "walk_weight_ranges": (
        ("alternating_label_chain", {"n": 200000}, {"n": 32},
         "The only source-to-vertex walk is a chain of alternating bits, so no letters cancel."),
        ("alternating_label_cycle", {"n": 200000}, {"n": 32},
         "The alternating cycle has even length and appends a fresh alternating block on every lap.")),
    "xor_walk_reachability": (
        ("all_states_along_chain", {"n": 1000, "m": 1000, "bits": 10}, {"n": 33, "m": 33, "bits": 4},
         "Self-loops at vertex 1 span all XOR values, and a chain reaches the destination."),
        ("large_reachable_state_space_without_target", {"n": 1000, "m": 1000, "bits": 10},
         {"n": 33, "m": 33, "bits": 4},
         "All XOR values propagate along a long chain that never reaches the destination.")),
    "dynamic_color_component_sum": (
        ("star_center_and_leaf_flips", {"n": 300000, "q": 200000}, {"n": 42, "q": 57},
         "Flip the degree n-1 vertex and mutate leaves. Color totals give exact answers."),
        ("long_mixed_color_path", {"n": 300000, "q": 200000}, {"n": 43, "q": 58},
         "Move the boundary between two long monochromatic runs while updating arbitrary vertex weights.")),
    "static_top_tree": (
        ("chain_hash_updates", {"n": 200000, "q": 200000}, {"n": 27, "q": 37},
         "Every vertex except the last has one child, so each result is a running sum."),
        ("star_hash_updates", {"n": 200000, "q": 200000}, {"n": 27, "q": 37},
         "Root plus a product of the leaves. Updates toggle a zero factor and include large residues.")),
    "tree_marked_diameter": (
        ("path_endpoint_changes", {"n": 100000, "q": 100000}, {"n": 41, "q": 59},
         "Delete a prefix of the marked path and restore it in reverse order; count edges between its endpoints."),
        ("star_leaf_changes", {"n": 100000, "q": 100000}, {"n": 41, "q": 59},
         "The last two leaves remain marked, so changing other leaves keeps the diameter equal to 2.")),
    "dynamic_steiner_subtree": (
        ("path_reroot", {"n": 200000, "q": 200000}, {"n": 42, "q": 36},
         "Intersect the surviving odd-numbered terminals with a prefix, suffix, or the full path."),
        ("star_with_cuts", {"n": 200000, "q": 200000}, {"n": 42, "q": 36},
         "Two or more selected leaves need the center; the rerooted subtree may exclude one spoke.")),
}

VALIDITY = {
    "tree_heap_orderings": "2<=N<=200000; the input is a connected undirected tree, as in ABC160 F.",
    "difference_constraints": "3<=N=M<=2000, valid distinct pair constraints and coordinates, as in PAST21 O.",
    "tree_walk_orientation": "2<=N<=2000, 1<=M<=2000; connected tree with distinct endpoints per walk, as in AGC025 E.",
    "vertex_deletion_components": "2<=H,W<=1000, at least one green cell; as in ABC334 G.",
    "subsequence_shortest_paths": "2<=N<=200000, M=K=N and all edge weights positive; as in ABC271 E.",
    "tournament_strong_counts": "1<=N<=200000; one uniformly random tournament on N labels, as in Paken Camp 2025 Day 1 P.",
    "tournament_component_sums": "1<=N<=30, 0<=M<=N(N-1)/2; as in ARC163 D.",
    "path_matching_candies": "1<=N<=200000 and 1<=weights<=1000000000, as in JOISC2018 J.",
    "core_decomposition": "1<=N<=200000, 0<=M<=200000, 1<=weights<=1000000000; simple undirected graph, as in ABC267 E.",
    "forest_independent_set": "1<=N<=100000; the input is a connected undirected tree, as in DP P.",
    "forest_edge_coloring": "2<=N<=100000; the input is a tree with 1<=u<v<=N, as in ABC146 D.",
    "forest_subset_components": "1<=N<=5000; connected input tree, as in ABC287 F.",
    "incident_label_spanning_tree": "2<=N<=200000, N-1<=M<=200000; connected undirected distinct (u,v,color) triples with R/B labels, as in AGC064 B.",
    "interval_cover_subsets": "2<=N<=200000, 1<=M<=200000; positive forward intervals and a fixed reverse chain, as in ABC450 F.",
    "strong_connectivity_augmentation": "1<=N<=10000, 0<=M<=100000; distinct directed non-loop edges, as in AOJ 0366.",
    "affine_walk_ranges": "1<=N<=200000, 0<=M<=200000; distinct directed labeled edges and all vertices reachable, as in yukicoder No.3784.",
    "superincreasing_path_queries": "2<=N<=200000, 1<=M,Q<=200000; the connected simple graph uses powers of two in input edge order, as in yukicoder No.1600.",
    "chinese_postman_route": "2<=N<=100000, N-1<=M<=100000, 1<=W<=1000000, connected loopless multigraph with <=16 odd vertices, as in AWC0073 E.",
    "tree_backedge_absorption": "2<=N<=250000, 0<=M<=250000; parents precede children, resets go from internal vertices to proper ancestors, as in JSC2024 Final E.",
    "tree_edge_color_path_counts": "1<=N,K<=200000; one colored tree with labels in [1,K], as in yukicoder No.1002.",
    "dynamic_edge_products": "2<=N<=100000, 1<=Q<=100000; unordered simple edge toggles with endpoints stored in ascending order, as in AOJ 3509.",
    "directed_return_distances": "1<=N,M<=2000, 1<=edge weight<=10^5; loops and parallel directed edges permitted per ABC191 E.",
    "tree_distance_sums": "1<=N,Q<=200000; unit tree, 1<=update<=10^9 and valid vertex IDs as in SCPC 2026 Div.1 L.",
    "bitwise_path": "2<=N<=200000, M=N<=200000, 0<=labels<2^30; connected undirected graph with parallel edges permitted, as in ABC408 E.",
    "graph_metric_realization": "1<=N<=300; zero diagonal and symmetric strictly positive off-diagonal distances at most 10^9 as in ABC074 D.",
    "tree_permutation_distance": "2<=N<=5000; unweighted simple tree as in ARC087 F.",
    "shortest_odd_cycle": "2<=N<=2000, M=N<=2000 and 1<=K<=10^18; ordered distinct simple edges and endpoints per yukicoder #3291.",
    "binary_tree_traversal": "2<=N<=200000, both traversals are permutations of [1,N] as in ABC255 F.",
    "tree_metric": "2<=N<=3000; one triangular table with positive distances at most 9999, as in ABC451 E.",
    "exact_length_walk_bottleneck": "2<=N<=100, 1<=M<=N^2, 1<=L<=10^9; directed edges distinct as in ABC236 G.",
    "exact_length_walk_weighted_count": "2<=N<=100, 0<=M<=N(N-1), 1<=K<=10^9; distinct nonloop arcs with weights in [0,10^9] as in AWC0053 E.",
    "undirected_walk_lengths": "2<=N<=100000, M=N<=100000, 1<=P<=10^9; simple undirected graph and distinct endpoints as in yukicoder #1190.",
    "subcubic_tree_biconnectivity": "One subcubic tree with 3<=N<=200000 and 1<=Wi<=10^9 as in ARC160 E.",
    "resource_constrained_paths": "2<=N<=50, N-1<=M<=100; connected simple rail graph, coins/fares/times within ABC164 E ranges.",
    "unrooted_tree_leaf_deletion": "2<=N<=200000; both endpoint-defined graphs are trees and at least one deletion is valid.",
    "component_add_union_find": "1<=N,Q<=500000; new unordered edges are distinct, increments in [-2000,2000].",
    "hypergraph_shortest_paths_merge": "1<=N<=200000, 2<=M<=200000 and 1<=sum(A_i)<=500000; no duplicates inside a group.",
    "shortest_nonzero_paths": "2<=N<=100000, 1<=M<=200000, 1<=K<=30; connected undirected multigraph, no loops, lengths in [1,10^9].",
    "dsu_on_tree_mode": "2<=N<=2500000, 2<=M<=min(N,100000), 1<=F<=N; 1<=q_i<i and 1<=d_i<=N.",
    "unit_step_forest_labels": "1<=N<=100000, roots and terminals are valid distinct constraints with fixed values in [0,10^6].",
    "walk_weight_congruence_return": "T=1, 2<=N<=200000 and 1<=M<=200000; all directed edges are distinct and not loops.",
    "tree_path_difference_road": "1<=N,Q,R<=50000; every parent index is smaller than its child, weights in [0,10^9].",
    "subtree_edge_add_distances": "2<=N<=100000 and 1<=Q<=100000; rooted tree directed away from 0, weights and increments in [0,100000].",
    "automaton_shortest_path_hopscotch": "2<=N<=100000 and 0<=M<=100000; directed simple edges and distinct terminals.",
    "bipartite_coloring": "1<=N,M<=200000; both endpoint sequences have length M and use values in [1,N].",
    "bipartite_matching_allowed_edges": "1<=N,M<=100000 per side and L<=200000; ordered cross-side pairs are distinct.",
    "block_cut_forest": "3<=N<=200000; exactly N simple edges make a connected graph, a,b,c distinct.",
    "bottleneck_pair_histogram": "2<=N<=100000; connected simple tree and all edge weights within [1,10^7].",
    "bridge_block_orientation": "1<=N<=100, all unordered pairs used once, input reachability counts are attainable.",
    "complete_binary_tree": "1<=T<=100000; 1<=X<=N<=10^18; 0<=K<=N-1 in every independent case.",
    "complement_shortest_paths": "2<=N<=200000, M<=200000; every forbidden edge is distinct with endpoints in 1..N.",
    "dag_critical_path_schedule": "3<=N<=100000, 2<=M<=500000; a DAG, every vertex lies on a 0..N-1 path, 0<=duration<160.",
    "dag_grundy": "1<=N<=100000 and all game values are in [2,100000].",
    "directed_game": "1<=N<=200000; each word has length 7 and contains only upper/lowercase English letters.",
    "dynamic_tree_distances": "1<=N,Q<=200000; connected simple tree; weights fit [1,10^9] and at least one path query.",
    "edge_removal_distances": "2<=N<=400, 1<=M<=N(N-1); every directed edge is distinct and has different endpoints.",
    "enumerate_topological_orders": "1<=N,M<=100000 and 1<=K<=10; all edges are distinct with different endpoints.",
    "equal_tree_path_partition": "1<=N,K and N*K<=200000; edges form a tree with each pair sorted.",
    "eulerian_trail_decomposition": "2<=N<=300000, T=1; both endpoint sequences use values in [1,N].",
    "functional_graph_fold": "2<=N<=200000, 1<=K<=10^12, each increment between 1 and 10^6.",
    "grid_slide_distances": "2<=N<=1500; distinct open endpoints on an N×N board of '.' and '#'.",
    "grid_stride_distances": "Positive H,W,K<=10^6 and H*W<=10^6; distinct open endpoints and a grid of '.' and '@'.",
    "incremental_apsp": "2<=N<=300, Q<=200000, at most 300 distinct road closures; simple weighted undirected graph.",
    "incremental_bridges": "2<=V=E<=200000 and S!=G; edges form a simple undirected graph.",
    "incremental_weighted_forest": "1<=N<=200000, Q<=300000; links never create cycles, edge weights in [0,10^9].",
    "johnson_shortest_paths": "2<=N<=100 and 1<=M<=9900; no loops or negative cycles, signed weights within 10^12.",
    "k_shortest_simple_paths": "2<=N<=2000, N-1<=M<=2000, K=10; a connected simple graph and integer coordinates in [-1000,1000].",
    "kruskal_reconstruction_tree": "1<=H,W<=500, 1<=Q<=200000; query floors fit their buildings and endpoints differ.",
    "lexicographic_shortest_path": "2<=N<=200; connected simple undirected graph, 1<=weights<=10000.",
    "lower_bound_flow": "1<=H,W<=300; every row contains only 1, 2 or ? and has length W.",
    "mst_sensitivity_delete": "3<=N<=100000; 2N distinct positive-weight edges, graph remains connected after any deletion.",
    "near_tree_distances": "2<=N<=100000, N-1<=M<=N+10, 1<=Q<=10000; a simple connected graph and ordered distinct query pairs.",
    "neighborhood_assignment": "1<=N,Q<=200000, 0<=M<=200000; every undirected edge is distinct and operations use valid vertices.",
    "offline_dag_reachability": "2<=N<=100000, M,Q<=100000; every edge and query is a strictly increasing pair.",
    "palindromic_shortest_paths": "1<=N<=100; each of N^2 entries is '-' or a lowercase letter.",
    "parity_orientation": "2<=N<=100000 and N-1<=M<=100000; distinct nonloop edges form a connected graph.",
    "virtual_tree_distance_product": "2<=N<=200000; both inputs are connected simple trees on labels 1..N.",
    "range_edge_graph_express": "2<=N<=100000, 1<=M<=100000; positions and fares within 10^12, nonoverlapping closed intervals.",
    "reachability_chain_cover": "1<=N<=7000 and 0<=M<=7000; distinct directed nonloop arcs.",
    "time_dependent_paths_latest": "2<=N<=200000, M=N; all four timetable parameters in [1,10^9]; no loop edges.",
    "tree_nearest_marked": "2<=N<=100000, 1<=Q<=100000; every query sees at least one marked vertex.",
    "topological_order_counter": "2<=N<=16; distinct acyclic ordered constraints, 1<=M<=N(N-1)/2.",
    "tree_distance_layers": "2<=N<=200000 and 1<=Q<=200000; connected simple tree; 1<=U,K<=N.",
    "dynamic_color_component_sum": (
        "1<=N<=300000, 1<=Q<=200000; a simple tree, colors in {0,1}, initial weights and increments in [1,10^9]."
    ),
    "static_top_tree": "2<=N<=200000, 1<=Q<=200000; 1<=p_i<i and vertex values in [0,998244353).",
    "tree_marked_diameter": "3<=N<=100000, 1<=Q<=100000; at least two vertices remain marked after each toggle.",
    "tree_mo": "2<=N<=200000, 1<=Q<=200000; endpoints strictly increasing, labels and frequency bounds in [1,N].",
    "tree_path_bitwise_counts_and": "2<=N<=100000; connected simple tree, valid weights within 1..10^9.",
    "two_nearest_source_groups": "2<=N<=100000, M=N, K=2; 1<=L<=N distinct sources and 1<=weight<=10^9.",
    "undirected_pseudoforest": "3<=N<=200000 and 1<=Q<=200000; N distinct undirected edges make a connected graph.",
    "undirected_xor_walks": "One simple connected graph, 2<=N<=200000, N-1<=M<=200000, all weights and K in [0,2^30).",
    "walk_weight_ranges": "N,M<=200000, arc triples distinct, labels 0 or 1; every vertex is reachable from 1.",
    "xor_walk_reachability": "2<=N<=1000, M<=1000; the directed graph may have loops, weights lie in [0,2^10).",
    "shortest_path_edge_usage": "2<=N<=200000 and 1<=M<=200000; simple connected graph with positive weights through 10^9.",
    "shortest_walk_counts": "2<=N<=200000 and M<=200000; distinct undirected edges have ordered endpoints in [1,N].",
    "shortest_weighted_cycle": "3<=N<=2000, M=N; no loops or shared unordered pairs, positive edge weights through 10^9.",
    "dynamic_steiner_subtree": "1<=N<=200000, 1<=Q<=200000; endpoints in 1..N, initial colors are 0 or 1.",
}


def _tree(inp, n: int, star: bool) -> None:
    for vertex in range(2, n + 1):
        inp.line(1 if star else vertex - 1, vertex)


def _color_component_star(n, q, weights, colors, inp, out):
    center = 1
    totals = [0, 0]
    for vertex in range(n):
        totals[colors[vertex]] += weights[vertex]
        if vertex != center:
            a, b = sorted((center + 1, vertex + 1))
            inp.line(a, b)
    for i in range(q):
        turn, phase = divmod(i, 6)
        leaf = turn * 104729 % (n - 1)
        if leaf >= center:
            leaf += 1
        if phase in (0, 4):
            v = center if phase == 0 else leaf
            inp.line(1, v + 1)
            totals[colors[v]] -= weights[v]
            colors[v] ^= 1
            totals[colors[v]] += weights[v]
        elif phase == 2:
            inp.line(2, leaf + 1, 10**9)
            weights[leaf] += 10**9
            totals[colors[leaf]] += 10**9
        else:
            v = leaf if phase == 3 else center
            inp.line(3, v + 1)
            out.line(totals[colors[v]] if colors[v] == colors[center] else weights[v])


def _color_component_path(n, q, weights, split, inp, out):
    for i in range(1, n):
        inp.line(i, i + 1)
    totals = [sum(weights[:split]), sum(weights[split:])]
    for i in range(q):
        turn, phase = divmod(i, 6)
        if phase == 0:
            split -= 1
            inp.line(1, split + 1)
            totals[0] -= weights[split]
            totals[1] += weights[split]
        elif phase == 4:
            inp.line(1, split + 1)
            totals[0] += weights[split]
            totals[1] -= weights[split]
            split += 1
        elif phase == 2:
            v = turn * 104729 % n
            inp.line(2, v + 1, 10**9)
            weights[v] += 10**9
            totals[v >= split] += 10**9
        else:
            v = n - 1 if phase == 3 else 0
            inp.line(3, v + 1)
            out.line(totals[v >= split])


def _dynamic_color_component_sum(inp, out, n, q, star):
    split = n // 2
    weights = [10**9 - i % 71 for i in range(n)]
    colors = [i % 2 for i in range(n)] if star else [int(i >= split) for i in range(n)]
    inp.line(n, q)
    inp.line(*weights)
    inp.line(*colors)
    if star:
        _color_component_star(n, q, weights, colors, inp, out)
    else:
        _color_component_path(n, q, weights, split, inp, out)


def _static_top_tree(inp, out, n, q, star):
    modulus = 998244353
    values = [2 + i % 9 for i in range(n)]
    inp.line(n, q)
    inp.line(*([1] * (n - 1) if star else range(1, n)))
    inp.line(*values)
    if not star:
        total = sum(values)
        for i in range(q):
            vertex = (i * 104729 + n - 1) % n
            value = modulus - 1 - i % 11 if i % 5 else 0
            inp.line(vertex + 1, value)
            total += value - values[vertex]
            values[vertex] = value
            out.line(total % modulus)
        return

    inverse = {value: pow(value, modulus - 2, modulus) for value in range(1, 11)}
    inverse[modulus - 1] = modulus - 1
    product = 1
    for value in values[1:]:
        product = product * value % modulus
    zeroes = 0
    for i in range(q):
        turn, kind = divmod(i, 4)
        leaf = 1 + turn * 104729 % (n - 1)
        if kind == 0:
            vertex, value = 0, modulus - 1 - turn % 101
        elif kind == 1:
            vertex, value = leaf, 0
        elif kind == 2:
            vertex, value = leaf, modulus - 1
        else:
            vertex, value = 1 + (turn * 4439 + 19) % (n - 1), 1 + turn % 7
        inp.line(vertex + 1, value)
        if vertex:
            old = values[vertex]
            if old:
                product = product * inverse[old] % modulus
            else:
                zeroes -= 1
            if value:
                product = product * value % modulus
            else:
                zeroes += 1
        values[vertex] = value
        out.line((values[0] + (0 if zeroes else product)) % modulus)



def _tree_marked_diameter(inp, out, n, q, star):
    inp.line(n)
    _tree(inp, n, star)
    inp.line(q)
    if star:
        for i in range(q):
            inp.line(2 + i % (n - 3))
            out.line(2)
        return
    limit = n // 2
    for i in range(q):
        step = i % (2 * limit)
        vertex = step + 1 if step < limit else 2 * limit - step
        inp.line(vertex)
        out.line(n - vertex - (step < limit))


def _bipartite_coloring(inp, out, n: int, odd: bool) -> None:
    inp.line(n, n)
    if odd:
        inp.line(*range(1, n), 1)
        inp.line(*range(2, n), 1, 2)
    else:
        inp.line(*range(1, n + 1))
        inp.line(*range(2, n + 1), 1)
    out.line("No" if odd else "Yes")


def _block_cut_forest(inp, out, n: int, cycle: bool) -> None:
    inp.line(n, n)
    if cycle:
        inp.line(n - 1, 2, n)
        _tree(inp, n - 2, False)
        inp.line(1, n - 2)
        inp.line(1, n - 1)
        inp.line(1, n)
    else:
        inp.line(1, n // 2, n)
        _tree(inp, n, False)
        inp.line(1, 3)
    out.line("No" if cycle else "Yes")


def _complement_shortest_paths(inp, out, n: int, m: int, isolated: bool) -> None:
    inp.line(n, m)
    if isolated:
        for v in range(1, n):
            inp.line(v, n)
        used = n - 1
        answer = -1
    else:
        middle, right = n // 2, 3 * n // 4
        inp.line(1, n)
        for v in range(2, middle + 1):
            inp.line(1, v)
        for v in range(middle + 1, right + 1):
            inp.line(v, n)
        used = right
        answer = n - 1 - right
    for v in range(2, m - used + 2):
        inp.line(v, v + 1)
    out.line(answer)


def _dag_reachability(inp, out, n: int, m: int, q: int, two_chains: bool) -> None:
    inp.line(n, m, q)
    step = 2 if two_chains else 1
    for v in range(1, n - step + 1):
        inp.line(v, v + step)
    for _ in range(m - n + step):
        inp.line(1, 1 + step)
    half = n // 2
    for i in range(q):
        if two_chains:
            start = 1 + (7919 * i) % (n - 2)
            target = n - (i // 3) % 2
        else:
            start = 1 + i % half
            target = 1 + half + (i + 7 * (i // half)) % half
        inp.line(start, target)
        out.line("Yes" if (target - start) % step == 0 else "No")


def _functional_graph_fold(inp, out, n: int, k: int, cyclic: bool) -> None:
    inp.line(n, k)
    inp.line(*([1] * (n - 1)), n + int(cyclic))
    if cyclic:
        out.line(k + n * (k // n))
    else:
        tail = min(k, n - 1)
        out.line(tail + n * (k - tail))


def _near_tree_distances(inp, out, n: int, m: int, q: int, seed: int) -> None:
    inp.line(n, m)
    _tree(inp, n, False)
    ends = range(2 * n - m, n + 1)
    for v in ends:
        inp.line(1, v)
    inp.line(q)

    def from_first(v):
        return min(v - 1, 1 + min(abs(v - w) for w in ends))

    rng = random.Random(seed)
    for i in range(q):
        u = 1 + i % (n // 2)
        v = rng.randint(n // 2 + 1, n)
        inp.line(u, v)
        out.line(min(v - u, from_first(u) + from_first(v)))


def _lower_bound_flow(inp, out, h: int, w: int, odd: bool) -> None:
    inp.line(h, w)
    for i in range(h):
        inp.line("1" + "2" * (w - 1) if i == 0 and odd else "2" * w)
    out.line("No" if odd else "Yes")


def _kruskal_reconstruction_tree(inp, out, h: int, w: int, q: int, height: int, cut: bool, seed: int) -> None:
    inp.line(h, w)
    middle = w // 2
    gate = 2 if cut else height
    floors = [height] * middle + [gate] + [height] * (w - middle - 1)
    for _ in range(h):
        inp.line(*floors)
    inp.line(q)
    rng = random.Random(seed)
    for i in range(q):
        r1 = 1 + i % h
        r2 = rng.randint(1, h)
        c1 = rng.randint(1, middle)
        c2 = rng.randint(middle + 2, w)
        y = rng.randint(1, height)
        z = rng.randint(1, height)
        inp.line(r1, c1, y, r2, c2, z)
        out.line(y + z - 2 * min(y, z, gate))


def _grid_stride_distances(inp, out, h: int, w: int, k: int, wall: bool) -> None:
    inp.line(h, w, k)
    inp.line(1, 1, 1 if wall else h, w)
    empty = "." * w
    barrier = "." * (w // 2) + "@" + "." * (w - w // 2 - 1)
    for row in range(h):
        inp.line(barrier if wall and row != h - 1 else empty)
    out.line(3 if wall else h + w - 2)


def _grid_slide_distances(inp, out, n: int, wall: bool) -> None:
    inp.line(n)
    inp.line(1, 1)
    inp.line(n, n if wall else n - 2)
    empty, barrier = "." * n, "#" * n
    for row in range(n):
        inp.line(barrier if wall and row == n // 2 else empty)
    out.line(-1 if wall else 2)


def _edge_removal_distances(inp, out, n: int, m: int, dense_dead_end: bool) -> None:
    inp.line(n, m)
    if not dense_dead_end:
        for u in range(1, n + 1):
            for v in range(1, n + 1):
                if u != v:
                    inp.line(u, v)
                    out.line(2 if u == 1 and v == n else 1)
        return
    half = n // 2
    for v in range(1, half):
        inp.line(v, v + 1)
        out.line(-1)
    inp.line(half, n)
    out.line(-1)
    inp.line(1, half + 1)
    out.line(half)
    for u in range(half + 1, n):
        for v in range(half + 1, n):
            if u != v:
                inp.line(u, v)
                out.line(half)


def _neighborhood_assignment(inp, out, n: int, m: int, q: int, clique: int) -> None:
    inp.line(n, m, q)
    if clique == 0:
        _tree(inp, n, True)
        inp.line(2, 1, *(1 + (i * 1699) % n for i in range(q - 2)))
        out.line(*(2 for _ in range(n)))
    else:
        for u in range(1, clique + 1):
            for v in range(u + 1, clique + 1):
                inp.line(u, v)
        extra = m - clique * (clique - 1) // 2
        for u in range(clique + 1, clique + 1 + extra):
            inp.line(u, u + 1)
        inp.line(clique - 1, *(1 + (i * 773) % clique for i in range(q - 1)))
        out.line(*([clique - 1] * clique), *range(clique + 1, n + 1))


def _shortest_weighted_cycle(inp, out, n: int, directed: bool) -> None:
    inp.line(int(directed))
    inp.line(n, n)
    total = 0
    for v in range(1, n + 1):
        weight = 10**9 if directed else 1 + 17 * v
        inp.line(v, v % n + 1, weight)
        total += weight
    out.line(total)


def _undirected_pseudoforest(inp, out, n: int, q: int, cycle: bool) -> None:
    inp.line(n)
    if cycle:
        _tree(inp, n, False)
        inp.line(1, n)
    else:
        inp.line(1, 2)
        inp.line(1, 3)
        inp.line(2, 3)
        inp.line(1, 4)
        for v in range(4, n):
            inp.line(v, v + 1)
    inp.line(q)
    for i in range(q):
        u = 1 + (i % 3)
        v = 4 + (i * 7879) % (n - 3)
        inp.line(u, v)
        out.line("Yes" if not cycle and u == 1 else "No")


def _shortest_path_edge_usage(inp, out, n: int, layered: bool) -> None:
    if not layered:
        inp.line(n, n)
        for v in range(1, n):
            inp.line(v, v + 1, 1)
            out.line("Yes")
        inp.line(1, n, 10**9)
        out.line("No")
        return
    inp.line(n, 2 * n)
    inp.line(1, 2, 1)
    inp.line(1, 3, 1)
    for layer in range(2, n - 3, 2):
        for v in (layer, layer + 1):
            for u in (layer + 2, layer + 3):
                inp.line(v, u, 1)
    inp.line(n - 2, n, 1)
    inp.line(n - 1, n, 1)
    for v in range(4, 8):
        inp.line(1, v, 10**9)
    for _ in range(2 * n):
        out.line("No")


def _incremental_weighted_forest(inp, out, n: int, q: int, star: bool) -> None:
    maximum = 10**9
    inp.line(n, 0, q)
    if star:
        for v in range(1, n):
            inp.line(1, v, maximum - v)
        diameter = (maximum - 1) + (maximum - 2)
        for i in range(q - n + 1):
            u, v = 13 * i % n, (29 * i + 7) % n
            if i % 3 == 0:
                inp.line(3, u)
                out.line(diameter)
            else:
                inp.line(2, u, v)
                distinct_spokes = {u, v} - {0} if u != v else set()
                out.line(sum(maximum - w for w in distinct_spokes))
        return
    size = 1 << (min(n, q // 2 + 1).bit_length() - 1)
    x = 0
    half = 1
    while half < size:
        for start in range(0, size, 2 * half):
            tail = start + half - 1
            inp.line(4, (tail - x) % n)
            inp.line(1, tail + 1, maximum)
            x = tail
        half *= 2
    for i in range(q - 2 * (size - 1)):
        u, v = 13 * i % n, (29 * i + 7) % n
        if i % 3 == 0:
            inp.line(3, v)
            out.line((size - 1) * maximum if v < size else 0)
        else:
            inp.line(2, u, v)
            out.line(abs(v - u) * maximum if max(u, v) < size or u == v else -1)


def _mst_sensitivity(inp, out, n: int, spokes: bool) -> None:
    large = 10**9
    inp.line(n, 2 * n)
    if spokes:
        for v in range(2, n + 1):
            inp.line(1, v, v - 1)
        for v in range(2, n):
            inp.line(v, v + 1, large)
        inp.line(2, n, large)
        inp.line(2, 4, large)
        inp.line(3, 5, large)
        out.line(n * (n - 1) // 2 + large - 1)
    else:
        for v in range(1, n + 1):
            inp.line(v, v % n + 1, 1)
        for v in range(1, n + 1):
            inp.line(v, (v + 1) % n + 1, large)
        out.line(n - 1)


def _virtual_tree(inp, out, n: int, star: bool) -> None:
    inp.line(n)
    _tree(inp, n, False)
    _tree(inp, n, star)
    if star:
        total = 2 * (n**3 - n) // 3 - n * (n - 1)
    else:
        total = n**2 * (n**2 - 1) // 6
    out.line(total % (1 << 64))


def _range_edge_graph(inp, out, n: int, m: int, wide: bool, seed: int) -> None:
    step = 10_000_000
    inp.line(n, m)
    inp.line(*(step * v for v in range(n)))
    if wide:
        rng = random.Random(seed)
        inp.line(1, 1, 2, n, 1)
        for i in range(m - 1):
            middle = rng.randint(n // 3, 2 * n // 3)
            left = rng.randint(1, n // 10), rng.randint(n // 4, middle)
            right = rng.randint(middle + 1, 3 * n // 4), rng.randint(9 * n // 10, n)
            board, arrival = (left, right) if i % 2 == 0 else (right, left)
            inp.line(*board, *arrival, rng.randint(1, 10**12))
        out.line(*(step * i + 1 for i in range(1, n)))
        return
    distance = 0
    expected = []
    for v in range(1, n):
        fare = 900_000_000_000 + v
        inp.line(v, v, v + 1, v + 1, fare)
        distance += step + fare
        expected.append(distance)
    for i in range(m - n + 1):
        v = 1 + i % (n - 1)
        inp.line(v + 1, v + 1, v, v, 1)
    out.line(*expected)


def _nearest_marked(inp, out, n: int, q: int, star: bool) -> None:
    inp.line(n)
    _tree(inp, n, star)
    inp.line(q)
    anchor = 2 if star else 1
    inp.line(1, anchor)
    steps = (q - 1) // 4
    for i in range(1, steps + 1):
        marked = anchor + i
        inp.line(1, marked)
        if star:
            query = (marked, 1, n)[i % 3]
            answer = 0 if query == marked else 1 if query == 1 else 2
        else:
            query = n - i % (n // 4)
            answer = query - marked
        inp.line(2, query)
        out.line(answer)
    for i in range(steps, 0, -1):
        cleared = anchor + i
        inp.line(1, cleared)
        inp.line(2, cleared if star else n)
        out.line(2 if star else n - cleared + 1)
    for _ in range(q - 1 - 4 * steps):
        inp.line(2, n)
        out.line(2 if star else n - 1)


def _tree_mo(inp, out, n: int, q: int, star: bool, seed: int) -> None:
    rng = random.Random(seed)
    period = 17
    inp.line(n, q)
    inp.line(*(v + 1 if star else 1 + v % period for v in range(n)))
    _tree(inp, n, star)
    for i in range(q):
        u = rng.randrange(1, n)
        v = rng.randrange(u + 1, n + 1)
        if star:
            low = 1 + i % 3
            high = low + i % 5
            answer = (2 if u == 1 else 3) if low == 1 else 0
        else:
            frequency, extra = divmod(v - u + 1, period)
            low = max(1, frequency + i % 5 - 2)
            high = low + i % 7
            answer = (period - extra) * (low <= frequency <= high)
            answer += extra * (low <= frequency + 1 <= high)
        inp.line(u, v, low, high)
        out.line(answer)


def _path_steiner(inp, out, n: int, q: int, seed: int) -> None:
    rng = random.Random(seed)
    low, high = 1, n if n % 2 else n - 1
    inp.line(*(v % 2 for v in range(1, n + 1)))
    inp.line(q)
    for i in range(q):
        if i % 4 == 0:
            inp.line(1, low)
            low += 2
        elif i % 4 == 2:
            inp.line(1, high)
            high -= 2
        else:
            root = rng.randint(1, n)
            v = root if i % 11 == 0 else rng.randint(1, n)
            inp.line(2, root, v)
            first = (max(low, v) if root < v else low) | 1
            last = min(high, v) if v < root else high
            last -= (last + 1) % 2
            out.line(max(0, last - first + 1))


def _star_steiner(inp, out, n: int, q: int, seed: int) -> None:
    rng = random.Random(seed)
    marked = [False, False] + [v % 2 == 0 for v in range(2, n + 1)]
    marked_leaves = sum(marked)
    inp.line(*(int(marked[v]) for v in range(1, n + 1)))
    inp.line(q)
    for i in range(q):
        operation = i % 6
        if operation % 2 == 0:
            v = 1 if operation == 0 else rng.randint(2, n)
            inp.line(1, v)
            marked[v] = not marked[v]
            if v != 1:
                marked_leaves += 1 if marked[v] else -1
        elif operation == 3:
            root = rng.randint(2, n)
            inp.line(2, root, 1)
            leaves = marked_leaves - marked[root]
            out.line(leaves + int(leaves > 1 or marked[1]))
        elif operation == 5:
            v = rng.randint(2, n)
            inp.line(2, 1, v)
            out.line(int(marked[v]))
        else:
            root = rng.randint(1, n)
            inp.line(2, root, root)
            out.line(marked_leaves + int(marked_leaves > 1 or marked[1]))


def _undirected_xor_walks(inp, out, n: int, bits: int, full_rank: bool) -> None:
    inp.line(1)
    if full_rank:
        inp.line(n, n - 1 + bits, 0)
        for vertex in range(1, n):
            inp.line(vertex, vertex + 1, 0)
        for bit in range(bits):
            inp.line(1, bit + 3, 1 << bit)
        out.line(n * (n - 1) // 2)
    else:
        inp.line(n, n - 1, 0)
        mask = (1 << bits) - 1
        for vertex in range(2, n + 1):
            inp.line(1, vertex, ((vertex - 1) * 2654435761) & mask)
        out.line(0)


def _xor_walk_reachability(inp, out, n: int, m: int, bits: int, connected: bool) -> None:
    inp.line(n, m)
    for bit in range(bits):
        inp.line(1, 1, 1 << bit)
    length = m - bits
    mask = (1 << bits) - 1
    for vertex in range(1, length):
        inp.line(vertex, vertex + 1, (vertex * 613) & mask)
    inp.line(length, n if connected else length + 1, mask)
    out.line(0 if connected else -1)


def _walk_weight_ranges(inp, out, n: int, cycle: bool) -> None:
    inp.line(n, n - 1 + cycle)
    for vertex in range(1, n):
        inp.line(vertex, vertex + 1, (vertex - 1) % 2)
    if cycle:
        inp.line(n, 1, (n - 1) % 2)
    for vertex in range(n):
        out.line(-1 if cycle else vertex)


def _parity_orientation(inp, out, n: int, cycle: bool) -> None:
    inp.line(n, n - 1 + cycle)
    for vertex in range(1, n):
        inp.line(vertex, vertex + 1)
        if cycle:
            directed = (vertex, vertex + 1) if vertex % 2 else (vertex + 1, vertex)
            out.line(*directed)
    if cycle:
        inp.line(n, 1)
        out.line(1, n)
    else:
        out.line(-1)


def _time_dependent_paths_latest(inp, out, n: int, period: int, count: int) -> None:
    inp.line(n, n)
    for u in range(1, n):
        inp.line(period, period, count, 1, u, u + 1)
    inp.line(period, period, count, 1, n, 1)
    for u in range(1, n):
        train_number = count - (n - 1 - u)
        out.line(period * train_number if train_number > 0 else "Unreachable")


def _complete_binary_tree(inp, out, n: int, tests: int, partial: bool) -> None:
    depth = n.bit_length() - 1
    first_leaf = 1 << depth
    inp.line(tests)
    for i in range(tests):
        if not partial:
            inp.line(n, first_leaf + i % first_leaf, 2 * depth)
            out.line(first_leaf // 2)
        elif i % 4 == 0:
            inp.line(n, n, 2 * depth)
            out.line(first_leaf // 2)
        elif i % 4 == 1:
            inp.line(n, 1, depth)
            out.line(n - first_leaf + 1)
        elif i % 4 == 2:
            inp.line(n, n, n - 1)
            out.line(0)
        else:
            inp.line(n, n, 0)
            out.line(1)


def _shortest_walk_counts(inp, out, n: int, layers: int) -> None:
    if layers == 0:
        inp.line(n, n)
        for u in range(1, n - 1):
            inp.line(u, u + 1)
        inp.line(1, 3)
        inp.line(2, 4)
        out.line(0)
        return
    inp.line(n, 4 * layers)
    inp.line(1, 2)
    inp.line(1, 3)
    for layer in range(layers - 1):
        a = 2 + 2 * layer
        for u in (a, a + 1):
            for v in (a + 2, a + 3):
                inp.line(u, v)
    inp.line(2 * layers, n)
    inp.line(2 * layers + 1, n)
    out.line(pow(2, layers, 10**9 + 7))


def _dynamic_tree_distances(inp, out, n: int, q: int, star: bool) -> None:
    initial = 10**9
    inp.line(n)
    for v in range(2, n + 1):
        inp.line(1 if star else v - 1, v, initial)
    inp.line(q)
    weights = [initial] * (n - 1)
    total = sum(weights)
    for i in range(q // 2):
        edge = i % (n - 1)
        weight = 1 + (9176 * i) % initial
        inp.line(1, edge + 1, weight)
        total += weight - weights[edge]
        weights[edge] = weight
        if star:
            other = (edge + 1) % (n - 1)
            inp.line(2, edge + 2, other + 2)
            out.line(weight + weights[other])
        else:
            inp.line(2, 1, n)
            out.line(total)


def _bridge_block_orientation(inp, out, n: int, mixed: bool) -> None:
    reach = [n] * n
    if mixed:
        reach[n // 2:] = [n // 2] * (n // 2)
    inp.line(n, n * (n - 1) // 2)
    for u in range(n):
        for v in range(u + 1, n):
            inp.line(u + 1, v + 1)
            out.line("->" if reach[u] > reach[v] or v == u + 1 else "<-")
    inp.line(*reach)


def _reachability_chain_cover(inp, out, n: int, width: int) -> None:
    if width == 0:
        inp.line(n, n)
        for u in range(1, n):
            inp.line(u, u + 1)
        inp.line(1, 3)
        out.line(*([1] * n))
        return
    first, last = width + 1, n - width
    inp.line(n, n - 1)
    for u in range(1, first):
        inp.line(u, first)
    for u in range(first, last):
        inp.line(u, u + 1)
    for u in range(last + 1, n + 1):
        inp.line(last, u)
    out.line(*range(1, width + 1), *([1] * (last - first + 1)), *range(1, width + 1))


def _directed_game(inp, out, n: int, cycle: bool) -> None:
    letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    alphabet = len(letters)

    def word_state(index):
        return letters[index // (alphabet * alphabet)] + letters[index // alphabet % alphabet] + letters[index % alphabet]

    length = n // 2
    inp.line(n)
    for i in range(length):
        next_state = (i + 1) % length if cycle else i + 1
        for middle in "AB":
            inp.line(word_state(i) + middle + word_state(next_state))
            out.line("Draw" if cycle else "Aoki" if (length - next_state) % 2 else "Takahashi")


def _dag_grundy(inp, out, n: int, value: int, prime: int, unique: bool) -> None:
    inp.line(n)
    inp.line(*([value] * (n - 1)), prime if unique else value)
    out.line("Anna" if unique else "Bruno")


def _tree_distance_layers(inp, out, n: int, q: int, star: bool) -> None:
    inp.line(n)
    _tree(inp, n, star)
    inp.line(q)
    for i in range(q):
        if star:
            leaf = 2 + 911 * i % (n - 1)
            match i % 4:
                case 0:
                    inp.line(1, 1)
                    out.line(2)
                case 1:
                    inp.line(leaf, 2)
                    out.line(3 if leaf == 2 else 2)
                case 2:
                    inp.line(leaf, 1)
                    out.line(1)
                case _:
                    inp.line(leaf, 3)
                    out.line(-1)
        else:
            k = 1 + 911 * i % n
            inp.line(n if i % 2 else 1, k)
            out.line(-1 if k == n else n - k if i % 2 else k + 1)


def _incremental_apsp(inp, out, n: int, q: int, dense: bool) -> None:
    edge_count = n * (n - 1) // 2 if dense else n - 1
    inp.line(n, edge_count, q)
    for u in range(1, n):
        if dense:
            for v in range(u + 1, n + 1):
                inp.line(u, v, 1)
        else:
            inp.line(u, u + 1, 10**9)
    removed = 0
    limit = n if dense else n - 1
    for i in range(q):
        if i % 2 and removed < limit:
            removed += 1
            inp.line(1, removed)
            continue
        vertex = 1 + (i * 911) % (n - 1)
        if dense:
            inp.line(2, 1, vertex + 1)
            out.line(1 if vertex > removed else 2 if removed < n - 1 else -1)
        else:
            inp.line(2, vertex, n)
            out.line((n - vertex) * 10**9 if vertex > removed else -1)


def _bipartite_matching_allowed_edges(inp, out, n: int, cycle: bool) -> None:
    inp.line(n, n, 2 * n)
    for u in range(1, n + 1):
        inp.line(u, u)
        out.line("Yes")
    for u in range(1, n):
        inp.line(u, u + 1)
        out.line("Yes" if cycle else "No")
    last_edge = (n, 1) if cycle else (1, n)
    inp.line(*last_edge)
    out.line("Yes" if cycle else "No")


def _eulerian_trail_decomposition(inp, out, n: int, parallel: bool) -> None:
    inp.line(1)
    inp.line(n)
    if parallel:
        half = n // 2
        inp.line("1 " * half + " ".join(map(str, range(half + 1, n + 1))))
        inp.line("2 " * half + " ".join(map(str, range(half + 1, n + 1))))
        out.line(0)
    else:
        inp.line(" ".join(map(str, range(1, n + 1))))
        inp.line(" ".join(map(str, range(2, n + 1))) + " 1")
        out.line(n)


def _two_nearest_source_groups(inp, out, n: int, weight: int, star: bool) -> None:
    mid = n // 2
    inp.line(n, n, 2, n if star else 2)
    inp.line("1 " * mid + " ".join(["2"] * (n - mid)))
    if star:
        inp.line(" ".join(map(str, range(1, n + 1))))
        for v in range(2, n + 1):
            inp.line(1, v, weight)
        inp.line(n - 1, n, weight)
        out.line(" ".join(str(weight * (2 if 2 <= v <= mid else 1)) for v in range(1, n + 1)))
    else:
        inp.line(1, n)
        for v in range(1, n):
            inp.line(v, v + 1, weight)
        inp.line(1, n, weight)
        distances = (min(n - v, v) if v <= mid else min(v - 1, n - v + 1) for v in range(1, n + 1))
        out.line(" ".join(str(weight * distance) for distance in distances))


def _palindromic_shortest_paths(inp, out, n: int, blocks: bool) -> None:
    inp.line(n)
    for u in range(n):
        if blocks:
            same_block = [(u < n // 2) == (v < n // 2) for v in range(n)]
            inp.line("".join("a" if same and u != v else "-" for v, same in enumerate(same_block)))
            out.line(" ".join(str(0 if u == v else (1 if same else -1)) for v, same in enumerate(same_block)))
        else:
            inp.line("".join("a" if v == (u + 1) % n else "-" for v in range(n)))
            out.line(" ".join(str((v - u) % n) for v in range(n)))


def _topological_order_counter(inp, out, n: int, chains: bool) -> None:
    if chains:
        half = n // 2
        constraints = [(u, v) for start, stop in ((1, half + 1), (half + 1, n + 1))
                       for u in range(start, stop) for v in range(u + 1, stop)]
        inp.line(n, len(constraints))
        for u, v in constraints:
            inp.line(u, v)
        out.line(comb(n, half))
    else:
        inp.line(n, 1)
        inp.line(1, 2)
        out.line(factorial(n) // 2)


def _k_shortest_simple_paths(inp, out, n: int, m: int, k: int, diamonds: bool) -> None:
    inp.line(n, m, k)
    inp.line(1, n)
    if diamonds:
        stages = (n - 1) // 3
        assert n == 3 * stages + 1 and m == 4 * stages and (1 << stages) >= k
        inp.line(-stages, 0)
        for i in range(stages):
            inp.line(2 * i + 1 - stages, 0)
            inp.line(2 * i + 1 - stages, 0)
            inp.line(2 * i + 2 - stages, 0)
        for start in range(1, n, 3):
            inp.line(start, start + 1)
            inp.line(start, start + 2)
            inp.line(start + 1, start + 3)
            inp.line(start + 2, start + 3)
        for _ in range(k):
            out.line(2 * stages)
    else:
        assert n == m and k >= 2
        for v in range(n):
            inp.line(v - n // 2, 0)
        for v in range(1, n):
            inp.line(v, v + 1)
        inp.line(1, 3)
        out.line(n - 1)
        out.line(n - 1)
        for _ in range(k - 2):
            out.line(-1)


def _johnson_shortest_paths(inp, out, n: int, weight: int, complete: bool) -> None:
    if complete:
        inp.line(n, n * (n - 1))
        base = 1000000
        potential = [v * (weight // n) for v in range(n)]
        potential_sum = sum(potential)
        for u in range(n):
            for v in range(n):
                if u != v:
                    inp.line(u + 1, v + 1, base + potential[v] - potential[u])
            out.line((n - 1) * base + potential_sum - n * potential[u])
    else:
        inp.line(n, n - 1)
        for u in range(1, n):
            inp.line(u, u + 1, -weight)
        for u in range(n):
            span = n - u - 1
            out.line(-weight * span * (span + 1) // 2)


def _enumerate_topological_orders(inp, out, n: int, k: int, cycle: bool) -> None:
    inp.line(n, n, k)
    if cycle:
        for u in range(1, n - 2):
            inp.line(u, u + 1)
        inp.line(n - 2, n - 1)
        inp.line(n - 1, n)
        inp.line(n, n - 1)
        out.line(-1)
    else:
        for u in range(1, n - 4):
            inp.line(u, u + 1)
        for v in range(n - 3, n + 1):
            inp.line(n - 4, v)
        inp.line(1, 3)
        prefix = " ".join(map(str, range(1, n - 3)))
        for suffix in islice(permutations(range(n - 3, n + 1)), k):
            out.line(prefix + " " + " ".join(map(str, suffix)))


def _dag_critical_path_schedule(inp, out, n: int, m: int, duration: int, wide: bool) -> None:
    inp.line(n, m)
    if wide:
        inp.line(0, 1, duration)
        inp.line(1, n - 1, duration)
        for u in range(2, n - 1):
            inp.line(0, u, 0)
            inp.line(u, n - 1, 0)
        remaining = m - 2 * (n - 2)
        for gap in range(1, n - 3):
            take = min(remaining, n - 3 - gap)
            for u in range(2, 2 + take):
                inp.line(u, u + gap, 0)
            remaining -= take
            if remaining == 0:
                break
        assert remaining == 0
        out.line(2 * duration, f"{n - 3}/{n}")
    else:
        for u in range(n - 1):
            inp.line(u, u + 1, duration)
        remaining = m - (n - 1)
        for gap in range(2, n):
            take = min(remaining, n - gap)
            for u in range(take):
                inp.line(u, u + gap, duration)
            remaining -= take
            if remaining == 0:
                break
        assert remaining == 0
        out.line((n - 1) * duration, f"0/{n}")


def _component_add_union_find(inp, out, n: int, q: int, balanced: bool) -> None:
    delta = 2000
    inp.line(n, q)
    if balanced:
        limit = min(n, q // 3)
        m = 1 << (limit.bit_length() - 1)
        for v in range(1, m + 1):
            inp.line(2, v, delta if v % 2 else -delta)
        width = 1
        while width < m:
            for v in range(1, m + 1, 2 * width):
                inp.line(1, v + width, v)
            width *= 2
        inp.line(2, 1, -delta)
        for i in range(q - 2 * m):
            v = 1 + (i % m) if i % 3 else n
            inp.line(3, v, 0)
            out.line(0 if v > m or v % 2 else -2 * delta)
    else:
        groups = min(q // 3, n - 1)
        for i in range(1, groups + 1):
            v = i + 1
            inp.line(1, 1, v)
            inp.line(2, v, delta)
            inp.line(3, 1 if i % 2 else v, 0)
            out.line(delta * i if i % 2 else delta)
        for _ in range(q - 3 * groups):
            inp.line(3, 1, 0)
            out.line(delta * groups)


def _hypergraph_shortest_paths_merge(inp, out, n: int, groups: int, memberships: int, disconnected: bool) -> None:
    def group(vertices):
        inp.line(len(vertices))
        inp.line(*vertices)

    inp.line(groups, n)
    middle = n // 2
    if disconnected:
        group(range(1, middle + 1))
        group(range(middle + 1, n + 1))
        double = memberships - n - (groups - 2)
        for i in range(groups - 2):
            group((1, 2) if i < double else (1,))
        out.line(-1)
    else:
        for v in range(1, middle):
            group((v, v + 1))
        group((middle, n))
        group(range(middle, n))
        remaining = groups - middle - 1
        extra = memberships - (n + middle) - 2 * remaining
        group(range(middle, middle + 2 + extra))
        for _ in range(remaining - 1):
            group((middle, middle + 1))
        out.line(middle - 1)


def _shortest_nonzero_paths(inp, out, n: int, m: int, bits: int, articulation: bool) -> None:
    zero, nonzero = "0" * bits, "1" * bits
    inp.line(n, m, bits)
    if articulation:
        for v in range(1, n - 2):
            inp.line(v, v + 1, 1, zero)
            inp.line(v, v + 1, 2, zero)
        inp.line(1, n - 1, 1, zero)
        inp.line(1, n - 1, 3, nonzero)
        for _ in range(m - 2 * (n - 3) - 2):
            inp.line(1, n, 1, zero)
        for _ in range(n - 2):
            out.line(-1)
        out.line(4)
    else:
        cost = 10**9 - 1
        for v in range(1, n):
            inp.line(v, v + 1, cost, zero)
            inp.line(v, v + 1, cost + 1, nonzero)
        for _ in range(m - 2 * (n - 1)):
            inp.line(n - 1, n, cost, zero)
        for v in range(1, n):
            out.line((n - v) * cost + 1)


def _dsu_on_tree_mode(inp, out, n: int, given: int, star: bool) -> None:
    seed = 45417
    palette = 2 if star else 1
    inp.line(n, seed, given, palette)
    inp.line(*(1 if star else v for v in range(1, given)))
    inp.line(*(1 + (v % 2 if star else 0) for v in range(given)))
    parents = array("I", [0]) * n
    state = seed
    for v in range(1, n):
        if v < given:
            parents[v] = 0 if star else v - 1
        else:
            parents[v] = state % v
            state = (state * 1103515245 + 12345) % (1 << 31)
    first = array("I", [1]) * n
    second = array("I", [0]) * n
    if star:
        for v in range(n):
            if v < given:
                color = v % 2
            else:
                color = state % palette
                state = (state * 1103515245 + 12345) % (1 << 31)
            if color:
                first[v] = 0
                second[v] = 1
    total = 0
    for v in range(n - 1, -1, -1):
        a, b = first[v], second[v]
        maximum = max(a, b)
        ties = 2 if a == b else 1
        total += (maximum ^ (v + 1)) * (ties ^ (v + 1))
        if v:
            parent = parents[v]
            first[parent] += a
            second[parent] += b
    out.line(total % 998244353)


def _tree_path_difference_road(inp, out, n: int, updates: int, queries: int, star: bool) -> None:
    base = 10**9 - n
    middle = n // 2
    inp.line(n)
    for v in range(2, n + 1):
        inp.line(1 if star else v - 1, base + v)
    inp.line(updates)
    incidence = [0] * (n + 1)
    for i in range(updates):
        if star:
            v = 3 + i % (n - 2)
            inp.line(2, v)
            incidence[v] += 1
        else:
            inp.line(1 if i % 2 == 0 else middle, n)
    incidence[2] = updates
    inp.line(queries)
    for i in range(queries):
        v = 3 + (i * 7) % (n - 2)
        if i % 3 == 0:
            inp.line(v, v)
            out.line(0)
        elif star:
            u = 2 if i % 3 == 1 else 1
            inp.line(u, v)
            other = base + v + incidence[v]
            out.line(min(base + 2 + updates, other) if u == 2 else other)
        else:
            u = 1 if i % 3 == 1 else middle
            inp.line(n, u)
            out.line(base + u + 1 + (updates if u == middle else (updates + 1) // 2))


def _subtree_edge_add_distances(inp, out, n: int, q: int, star: bool) -> None:
    inp.line(n)
    weight = 100000
    for v in range(1, n):
        inp.line(0 if star else v - 1, v, weight)
    inp.line(q)
    middle = n // 2
    root_increase = 0
    nested_increase = 0
    for i in range(q):
        if i % 4 == 0:
            inp.line(1, middle, weight)
            nested_increase += weight
        elif i % 4 == 2:
            inp.line(1, 0, weight)
            root_increase += weight
        else:
            v = (3 * i // 2) % n if i % 4 == 1 else n - 1
            inp.line(2, v)
            if star:
                out.line((weight + root_increase) if v else 0)
            else:
                out.line((weight + root_increase) * v + nested_increase * max(0, v - middle))


def _tree_heap_orderings(inp, out, n, star):
    modulus = 1000000007
    inp.line(n)
    for v in range(2, n + 1):
        inp.line(1 if star else v - 1, v)
    if star:
        rest = 1
        for k in range(2, n - 1):
            rest = rest * k % modulus
        out.line(rest * (n - 1) % modulus)
        for _ in range(n - 1):
            out.line(rest)
    else:
        fact = [1] * n
        for k in range(1, n):
            fact[k] = fact[k - 1] * k % modulus
        inverse_fact = [1] * n
        inverse_fact[-1] = pow(fact[-1], -1, modulus)
        for k in range(n - 1, 0, -1):
            inverse_fact[k - 1] = inverse_fact[k] * k % modulus
        for v in range(n):
            out.line(fact[-1] * inverse_fact[v] * inverse_fact[n - 1 - v] % modulus)


def _difference_constraints(inp, out, n, impossible):
    high = 100000000
    inp.line(n, n, 1)
    inp.line(0, 0, 0 if impossible else high)
    for _ in range(2, n):
        inp.line(0, 0, high)
    inp.line(high, 0, 0)
    for u in range(1, n):
        inp.line(u, u + 1, 0)
    inp.line(1, 3, 0)
    out.line(-1 if impossible else high)


def _tree_walk_orientation(inp, out, n, m, star):
    inp.line(n, m)
    out.line(2 * (n - 1))
    for v in range(2, n + 1):
        inp.line(1 if star else v - 1, v)
    if star:
        for v in range(2, n + 1):
            to = 2 if v == n else v + 1
            inp.line(v, to)
            out.line(v, to)
        inp.line(2, 3)
        out.line(2, 3)
    else:
        for i in range(m):
            inp.line(1, n)
            u, v = (n, 1) if i + 1 == m else (1, n)
            out.line(u, v)


def _vertex_deletion_components(inp, out, h, w, snake):
    inp.line(h, w)
    green = 0
    for row in range(h):
        if snake and row % 2:
            inp.line("." * (w - 1) + "#" if row % 4 == 1 else "#" + "." * (w - 1))
            green += 1
        else:
            inp.line("#" * w)
            green += w
    mod = 998244353
    out.line((2 * green - 2) * pow(green, -1, mod) % mod if snake else 1)


def _subsequence_shortest_paths(inp, out, n, reverse):
    inp.line(n, n, n)
    weight = 1000000000
    for v in range(1, n):
        inp.line(v, v + 1, weight)
    inp.line(n, 1, 1)
    forward = range(n - 1, 0, -1) if reverse else range(1, n)
    inp.line(n, *forward)
    out.line(-1 if reverse else (n - 1) * weight)


def _tournament_strong_counts(inp, out, n):
    # Exact values independently computed by solving the triangular recurrence
    # with CDQ and ACL convolution. The quick cases also enumerate all orientations.
    expected = {5: 444530692, 6: 110157829, 131073: 120609975, 200000: 945697629}
    inp.line(n)
    out.line(expected[n])


def _tournament_component_sums(inp, out, n, reversed_edge):
    total_edges = comb(n, 2)
    m = total_edges - 1 if reversed_edge else total_edges // 2
    inp.line(n, m)
    if reversed_edge:
        out.line(sum((n - gap) * (n if gap == 1 else n - gap) for gap in range(1, n)))
        return
    subset_sums = [{0: 1}, *({} for _ in range(n))]
    for label in range(n):
        for k in range(label + 1, 0, -1):
            target = subset_sums[k]
            for old, ways in subset_sums[k - 1].items():
                new = old + label
                target[new] = target.get(new, 0) + ways
    answer = comb(total_edges, m)
    for k in range(1, n):
        internal_edges = total_edges - k * (n - k)
        for labels, ways in subset_sums[k].items():
            internal_forward = m - (k * (n - 1) - labels - comb(k, 2))
            if 0 <= internal_forward <= internal_edges:
                answer += ways * comb(internal_edges, internal_forward)
    out.line(answer % 998244353)


def _path_matching_candies(inp, out, n, cliff):
    heavy = 1000000000
    inp.line(n)
    for i in range(n):
        inp.line(1 if cliff and i % 2 == 0 else heavy)
    for chosen in range(1, n // 2 + 1):
        out.line(chosen * heavy)
    if cliff:
        out.line((n + 1) // 2)


def _core_decomposition(inp, out, n, m, star):
    large = 1000000000
    inp.line(n, m)
    if star:
        weights = [1 + v % 2 for v in range(2, n + 1)]
        inp.line(large, *weights)
        for v in range(2, n + 1):
            inp.line(1, v)
        inp.line(2, 3)
        out.line(sum(weights))
    else:
        inp.line(*([large] * n))
        for v in range(n):
            inp.line(v + 1, (v + 1) % n + 1)
            inp.line(v + 1, (v + 2) % n + 1)
        out.line(4 * large)


def _forest_independent_set(inp, out, n, star):
    modulus = 1000000007
    inp.line(n)
    for v in range(2, n + 1):
        inp.line(1 if star else v - 1, v)
    if star:
        out.line((pow(2, n - 1, modulus) + 1) % modulus)
    else:
        previous, current = 1, 2
        for _ in range(1, n):
            previous, current = current, (previous + current) % modulus
        out.line(current)


def _forest_edge_coloring(inp, out, n, star):
    inp.line(n)
    out.line(n - 1 if star else 2)
    for v in range(2, n + 1):
        inp.line(1 if star else v - 1, v)
        out.line(v - 1 if star else 1 + v % 2)


def _forest_subset_components(inp, out, n, star):
    modulus = 998244353
    inp.line(n)
    for v in range(2, n + 1):
        inp.line(1 if star else v - 1, v)
    total = 1
    coefficient = [0] * (n + 1)
    if star:
        for k in range(1, n):
            total = total * (n - k) // k
            coefficient[k] = total % modulus
        coefficient[1] = (coefficient[1] + pow(2, n - 1, modulus)) % modulus
    else:
        for boundary in range(1, n + 2):
            total = total * (n + 2 - boundary) // boundary
            if boundary % 2 == 0:
                coefficient[boundary // 2] = total % modulus
    for value in coefficient[1:]:
        out.line(value)


def _incident_label_spanning_tree(inp, out, n, possible):
    inp.line(n, n)
    if possible:
        required = "R" + "".join("RB"[v % 2] for v in range(2, n + 1))
    else:
        required = "".join("RB"[v % 2] for v in range(n))
    for u in range(1, n):
        inp.line(u, u + 1, required[u])
    inp.line(1, 2, "B" if possible else "R")
    inp.line(required)
    if possible:
        out.line("Yes")
        out.line(*range(1, n))
    else:
        out.line("No")


def _interval_cover_subsets(inp, out, n, m, halves):
    inp.line(n, m)
    if halves:
        middle = (n + 1) // 2
        left = m // 2
        for i in range(m):
            if i < left:
                inp.line(1, middle)
            else:
                inp.line(middle, n)
        mod = 998244353
        out.line((pow(2, left, mod) - 1) * (pow(2, m - left, mod) - 1) % mod)
    else:
        for start in range(1, n):
            inp.line(start, start + 1)
        inp.line(1, 2)
        out.line(3)


def _strong_connectivity_augmentation(inp, out, n, m, bilayer):
    inp.line(n, m)
    if bilayer:
        sources = n // 4
        sinks = n - sources
        for edge in range(m):
            turn, sink = divmod(edge, sinks)
            inp.line((sink + turn) % sources, sources + sink)
        out.line(sinks)
        return

    count = 0
    for span in range(1, n):
        remaining = min(n - span, m - count)
        for start in range(remaining):
            inp.line(start, start + span)
        count += remaining
        if count == m:
            break
    out.line(1)


def _affine_walk_ranges(inp, out, n, pump):
    inp.line(n, n)
    for v in range(1, n):
        inp.line(v, v + 1, v % 2)
    if pump:
        begin = n // 2
        inp.line(begin + 1, begin, 1 - begin % 2)
        for v in range(1, n + 1):
            out.line(-1 if v >= begin else v - 1)
    else:
        inp.line(n, n, 1 - (n - 1) % 2)
        for v in range(1, n):
            out.line(v - 1)
        out.line(n)


def _superincreasing_path_queries(inp, out, n, m, q, cycle):
    modulus = 1000000007
    powers = [1]
    for _ in range(m):
        powers.append(2 * powers[-1] % modulus)
    inp.line(n, m)
    for v in range(1, n):
        inp.line(v, v + 1)
    if cycle:
        inp.line(1, n)
        inp.line(q)
        for i in range(q):
            excluded = i % m + 1
            inp.line(1, n, excluded)
            out.line(powers[n] if excluded != n else (powers[n] - 2) % modulus)
        return

    for u in range(1, n - 1):
        inp.line(u, u + 2)
    for u in range(1, m - (2 * n - 3) + 1):
        inp.line(u, u + 3)
    inp.line(q)
    for i in range(q):
        u = i % (n - 1) + 1
        if i % 3 == 1:
            inp.line(u, u + 1, n + i % (m - n + 1))
            out.line(powers[u])
        else:
            inp.line(u, u + 1, u)
            chord_start = max(1, u - 1)
            side_edge = 2 if u == 1 else u - 1
            out.line((powers[n - 1 + chord_start] + powers[side_edge]) % modulus)


def _chinese_postman_route(inp, out, n, weight, leaves):
    inp.line(n, n)
    stem = n - leaves
    for v in range(1, stem):
        inp.line(v, v + 1, weight)
    if leaves == 0:
        inp.line(n, 1, weight)
        out.line(n * weight)
        return
    for leaf in range(stem + 1, n + 1):
        inp.line(1, leaf, weight)
    inp.line(stem - 1, stem, weight)
    out.line((2 * n - 2) * weight)


def _tree_backedge_absorption(inp, out, n, fork):
    modulus = 998244353
    if not fork:
        inp.line(n, n)
        inp.line(*range(1, n))
        for v in range(2, n):
            inp.line(v, 1)
        inp.line(2, 1)
        inp.line(2, 1)
        out.line((10 * pow(2, n - 3, modulus) - 2) % modulus, modulus)
        return
    arm = n // 4
    stem = n - 2 * arm - 1
    branch = stem + 1
    left_first, right_first = branch + 1, branch + arm + 1
    parents = [*range(1, branch), branch, *range(left_first, left_first + arm - 1),
               branch, *range(right_first, right_first + arm - 1)]
    inp.line(n, 2 * (arm - 1))
    inp.line(*parents)
    scaled = 0
    for offset in range(arm - 1):
        left_to, right_to = offset + 1, stem - offset
        inp.line(left_first + offset, left_to)
        inp.line(right_first + offset, right_to)
        scaled = (2 * scaled + 4 + branch - left_to + branch - right_to) % modulus
    distance = stem + pow(2, arm - 1, modulus) + scaled * ((modulus + 1) // 2)
    out.line(distance % modulus, modulus)


def _tree_edge_color_path_counts(inp, out, n, hub):
    if hub:
        inp.line(n, n)
        arms, leftover = divmod(n - 1, 2)
        for index in range(arms):
            middle = 2 * index + 2
            inp.line(1, middle, middle - 1)
            inp.line(middle, middle + 1, middle)
        if leftover:
            inp.line(1, n, n - 1)
        out.line(comb(arms + leftover, 2) + arms)
    else:
        inp.line(n, 2)
        for vertex in range(1, n):
            inp.line(vertex, vertex + 1, 1 + vertex % 2)
        out.line(comb(n, 2) - (n - 1))


def _dynamic_edge_products(inp, out, n, q, columns):
    inp.line(n, q)
    if columns:
        rows = q // columns
        for i in range(q):
            row, col = divmod(i, columns)
            inp.line(row + 1, rows + col + 1)
            complete = (i + 1) // columns
            partial = (i + 1) % columns
            out.line(complete * columns * (i + 1) + partial * partial * (complete + 1))
    else:
        for leaf in range(2, n + 1):
            inp.line(1, leaf)
            out.line((leaf - 1)**2)
        inp.line(1, 2)
        out.line((n - 2)**2)


def _directed_return_distances(inp, out, n, weight, shortcut):
    inp.line(n, n)
    size = n - 1 if shortcut else n
    for u in range(1, size + 1):
        inp.line(u, u % size + 1, weight)
    if shortcut:
        inp.line(1, 3, weight)
    for u in range(1, n + 1):
        if u > size:
            out.line(-1)
        else:
            out.line((size - (shortcut and u != 2)) * weight)


def _tree_distance_sums(inp, out, n, q, star):
    inp.line(n, q)
    for v in range(1, n):
        inp.line(1 if star else v, v + 1)
    modulus = 998244353
    weight = 10**9
    answer = 0
    for i in range(q):
        if i % 2 == 0:
            if star:
                changed = (2, n, 1)[(i // 2) % 3]
                coefficient = 2 if changed == 1 else 3
            else:
                changed = n
                coefficient = n * (n - 1) // 2
            inp.line(1, changed, weight)
            answer = (answer + coefficient * weight) % modulus
        else:
            inp.line(2, 2 if star else 1, n)
            out.line(answer)


def _bitwise_path(inp, out, n, bits, forced):
    inp.line(n, n)
    if forced:
        for u in range(1, n):
            inp.line(u, u + 1, (1 << (u - (n - bits))) if u >= n - bits else 0)
        inp.line(1, 2, (1 << bits) - 1)
        out.line((1 << bits) - 1)
    else:
        for u in range(1, n):
            inp.line(u, u + 1, 1 + u % 2)
        inp.line(1, n, 1 << (bits - 1))
        out.line(3)


def _graph_metric_realization(inp, out, n, complete):
    inp.line(n)
    for u in range(n):
        if complete:
            inp.line(*(0 if u == v else 10**9 for v in range(n)))
        else:
            inp.line(*(abs(u - v) for v in range(n)))
    out.line(10**9 * n * (n - 1) // 2 if complete else n - 1)


def _tree_permutation_distance(inp, out, n, star):
    inp.line(n)
    for v in range(1, n):
        inp.line(1 if star else v, v + 1)
    mod = 1_000_000_007
    if star:
        before, current = 1, 0
        for size in range(2, n + 1):
            before, current = current, (size - 1) * (before + current) % mod
        out.line((current + before) % mod)
    else:
        half_factorial = 1
        for size in range(1, n // 2 + 1):
            half_factorial = half_factorial * size % mod
        out.line(half_factorial * half_factorial % mod)


def _shortest_odd_cycle(inp, out, n, length, triangle):
    inp.line(n, n, length, 1, 2)
    inp.line(1, 2)
    last = n - 4 if triangle else n - 1
    for v in range(3, last):
        inp.line(v, v + 1)
    inp.line(3, last)
    if triangle:
        inp.line(3, 6)
        inp.line(4, 7)
        inp.line(n - 2, n - 1)
        inp.line(n - 2, n)
        inp.line(n - 1, n)
    else:
        inp.line(3, n)
        inp.line(5, n)
    out.line("Yes" if triangle else "No")


def _binary_tree_traversal(inp, out, n, malformed):
    inp.line(n)
    inp.line(*range(1, n + 1))
    if malformed:
        inp.line(*range(1, n - 2), n, n - 2, n - 1)
        out.line(-1)
    else:
        inp.line(*range(n, 0, -1))
        for u in range(1, n + 1):
            out.line(u + 1 if u < n else 0, 0)


def _tree_metric(inp, out, n, star):
    inp.line(n)
    for u in range(n - 1):
        if not star:
            inp.line(*range(1, n - u))
        elif u == 0:
            inp.line(*(1 for _ in range(n - 1)))
        elif u == n - 2:
            inp.line(3)
        else:
            inp.line(*(2 for _ in range(n - u - 1)))
    out.line("No" if star else "Yes")


def _exact_length_walk_bottleneck(inp, out, n, length, dense):
    if dense:
        inp.line(n, n * n, length)
        for u in range(1, n + 1):
            for v in range(1, n + 1):
                inp.line(u, v)
        out.line(*range(1, n + 1))
    else:
        inp.line(n, n, length)
        for u in range(1, n + 1):
            inp.line(u, u % n + 1)
        out.line(*(n if v == length % n else -1 for v in range(n)))


def _exact_length_walk_weighted_count(inp, out, n, length, dense):
    modulus = 998244353
    if dense:
        inp.line(n, n * (n - 1), length)
        for u in range(1, n + 1):
            for v in range(1, n + 1):
                if u != v:
                    inp.line(u, v, 1)
        total = pow(n - 1, length, modulus)
        difference = -1 if length % 2 else 1
        out.line((total - difference) * pow(n, -1, modulus) % modulus)
    else:
        inp.line(n, n - 1, length)
        cycle = list(range(1, n))
        endpoint = length % len(cycle)
        if endpoint:
            cycle[endpoint] = n
        for u, v in zip(cycle, cycle[1:] + cycle[:1]):
            inp.line(u, v, 10**9)
        out.line(pow(10**9, length, modulus) if endpoint else 0)


def _undirected_walk_lengths(inp, out, n, length, odd_cycle):
    inp.line(n, n, length)
    inp.line(1, n)
    for v in range(1, n):
        inp.line(v, v + 1)
    inp.line(1, 3 if odd_cycle else n)
    if odd_cycle:
        out.line(n)
        for v in range(1, n + 1):
            out.line(v)
    else:
        out.line(-1)


def _subcubic_tree_biconnectivity(inp, out, n, caterpillar):
    inp.line(1)
    inp.line(n)
    if caterpillar:
        spine = n // 2
        inp.line(*(10 for _ in range(spine)), *(9 for _ in range(spine)))
        for v in range(1, spine):
            inp.line(v, v + 1)
        for v in range(1, spine + 1):
            inp.line(v, v + spine)
        out.line(spine // 2)
        for v in range(spine + 1, spine + spine // 2 + 1):
            out.line(v, v + spine // 2)
    else:
        inp.line(10**9, *(1 for _ in range(n - 2)), 10**9)
        for v in range(1, n):
            inp.line(v, v + 1)
        out.line(1)
        out.line(1, n)


def _resource_constrained_paths(inp, out, n, m, full_purse):
    inp.line(n, m, 10**9 if full_purse else 0)
    primary = {(1 if full_purse else v, v + 1) for v in range(1, n)}
    for u, v in sorted(primary):
        inp.line(u, v, 50, 10**9 if full_purse else 1)
    secondary = ((u, v) for u in range(1, n) for v in range(u + 1, n + 1) if (u, v) not in primary)
    for u, v in islice(secondary, m - (n - 1)):
        inp.line(u, v, 50, 10**9)
    exchange = 10**9 if full_purse else 1
    for _ in range(n):
        inp.line(exchange, exchange)
    for v in range(1, n):
        out.line(10**9 if full_purse else 51 * v)


def generate(plan: dict, inp, out) -> None:
    identifier = plan["verify_id"]
    scenario = plan["scenario"]
    dimensions = plan["dimensions"]
    n = dimensions["n"]
    if identifier == "automaton_shortest_path_hopscotch":
        wrong_residue = scenario == "reachable_goal_with_wrong_residue"
        m = dimensions["m"]
        inp.line(n, m)
        for v in range(1, m):
            inp.line(v, v + 1)
        inp.line(m, 1)
        inp.line(1, 2 if wrong_residue else n - 2)
        out.line(-1 if wrong_residue else n - 1)
    elif identifier == "subtree_edge_add_distances":
        _subtree_edge_add_distances(inp, out, n, dimensions["q"], scenario == "broad_star_root_and_leaf_updates")
    elif identifier == "dsu_on_tree_mode":
        _dsu_on_tree_mode(inp, out, n, dimensions["given"], scenario == "wide_given_star_two_colors")
    elif identifier == "shortest_nonzero_paths":
        _shortest_nonzero_paths(inp, out, n, dimensions["m"], dimensions["bits"],
                                scenario == "articulation_forbids_nonzero_detours")
    elif identifier == "hypergraph_shortest_paths_merge":
        _hypergraph_shortest_paths_merge(inp, out, n, dimensions["groups"], dimensions["memberships"],
                                          scenario == "disjoint_dense_groups")
    elif identifier == "component_add_union_find":
        _component_add_union_find(inp, out, n, dimensions["q"], scenario == "balanced_merges_of_preincremented_vertices")
    elif identifier == "tree_heap_orderings":
        _tree_heap_orderings(inp, out, n, scenario == "maximum_star_center_and_leaf_factorials")
    elif identifier == "difference_constraints":
        _difference_constraints(inp, out, n, scenario == "maximum_conflicting_pins_at_opposite_chain_ends")
    elif identifier == "tree_walk_orientation":
        _tree_walk_orientation(inp, out, n, dimensions["m"], scenario == "maximum_star_leaf_cycle_and_extra_copy")
    elif identifier == "vertex_deletion_components":
        _vertex_deletion_components(inp, out, dimensions["h"], dimensions["w"],
                                    scenario == "maximum_serpentine_path_every_internal_vertex_articulation")
    elif identifier == "subsequence_shortest_paths":
        _subsequence_shortest_paths(inp, out, n, scenario == "maximum_same_graph_reversed_schedule_is_unreachable")
    elif identifier == "tournament_strong_counts":
        _tournament_strong_counts(inp, out, n)
    elif identifier == "tournament_component_sums":
        _tournament_component_sums(inp, out, n, scenario == "maximum_one_reversed_edge_component_lengths")
    elif identifier == "path_matching_candies":
        _path_matching_candies(inp, out, n, scenario == "maximum_odd_size_last_cardinality_cliff")
    elif identifier == "core_decomposition":
        _core_decomposition(inp, out, n, dimensions["m"], scenario == "maximum_star_with_extra_leaf_edge_low_weight_leaves")
    elif identifier == "forest_independent_set":
        _forest_independent_set(inp, out, n, scenario == "maximum_star_dominating_center")
    elif identifier == "forest_edge_coloring":
        _forest_edge_coloring(inp, out, n, scenario == "maximum_star_distinct_colors")
    elif identifier == "forest_subset_components":
        _forest_subset_components(inp, out, n, scenario == "maximum_star_by_center_selected_or_not")
    elif identifier == "incident_label_spanning_tree":
        _incident_label_spanning_tree(inp, out, n, scenario == "maximum_single_seed_propagates_along_entire_path")
    elif identifier == "interval_cover_subsets":
        _interval_cover_subsets(inp, out, n, dimensions["m"], scenario == "maximum_two_half_covers_with_many_duplicate_options")
    elif identifier == "strong_connectivity_augmentation":
        _strong_connectivity_augmentation(inp, out, n, dimensions["m"], scenario == "maximum_unequal_bilayer_dag")
    elif identifier == "affine_walk_ranges":
        _affine_walk_ranges(inp, out, n, scenario == "maximum_alternating_chain_with_drift_cycle_and_long_suffix")
    elif identifier == "superincreasing_path_queries":
        _superincreasing_path_queries(inp, out, n, dimensions["m"], dimensions["q"],
                                      scenario == "maximum_one_long_cycle_every_edge_excluded_once")
    elif identifier == "chinese_postman_route":
        _chinese_postman_route(inp, out, n, dimensions["weight"], dimensions["leaves"])
    elif identifier == "tree_backedge_absorption":
        _tree_backedge_absorption(inp, out, n, scenario == "maximum_two_long_arms_with_disjoint_ancestor_coefficients")
    elif identifier == "tree_edge_color_path_counts":
        _tree_edge_color_path_counts(inp, out, n, scenario == "maximum_hub_two_edge_arms_distinct_colors")
    elif identifier == "dynamic_edge_products":
        _dynamic_edge_products(inp, out, n, dimensions["q"], dimensions.get("columns"))
    elif identifier == "directed_return_distances":
        _directed_return_distances(inp, out, n, dimensions["weight"], scenario == "maximum_branching_scc_with_one_shortcut")
    elif identifier == "tree_distance_sums":
        _tree_distance_sums(inp, out, n, dimensions["q"], scenario == "maximum_star_two_leaf_endpoints_and_center")
    elif identifier == "bitwise_path":
        _bitwise_path(inp, out, n, dimensions["bits"], scenario == "maximum_chain_forces_all_thirty_bits")
    elif identifier == "graph_metric_realization":
        _graph_metric_realization(inp, out, n, scenario == "maximum_complete_metric_every_edge_required")
    elif identifier == "tree_permutation_distance":
        _tree_permutation_distance(inp, out, n, scenario == "maximum_star_many_centroid_branches")
    elif identifier == "shortest_odd_cycle":
        _shortest_odd_cycle(inp, out, n, dimensions["length"], scenario == "maximum_bipartite_ring_before_remote_triangle")
    elif identifier == "binary_tree_traversal":
        _binary_tree_traversal(inp, out, n, scenario == "maximum_right_chain_with_contradictory_tail")
    elif identifier == "tree_metric":
        _tree_metric(inp, out, n, scenario == "maximum_star_matrix_corrupted_at_last_pair")
    elif identifier == "exact_length_walk_bottleneck":
        _exact_length_walk_bottleneck(inp, out, n, dimensions["length"], scenario == "maximum_complete_graph_fills_matrix_powers")
    elif identifier == "exact_length_walk_weighted_count":
        _exact_length_walk_weighted_count(inp, out, n, dimensions["length"], scenario == "maximum_all_to_all_loopless_graph")
    elif identifier == "undirected_walk_lengths":
        _undirected_walk_lengths(inp, out, n, dimensions["length"], scenario == "maximum_long_tail_with_odd_cycle")
    elif identifier == "subcubic_tree_biconnectivity":
        _subcubic_tree_biconnectivity(inp, out, n, scenario == "maximum_caterpillar_with_antipodal_leaf_edges")
    elif identifier == "resource_constrained_paths":
        _resource_constrained_paths(inp, out, n, dimensions["m"],
                                    scenario == "maximum_initial_coins_and_expensive_rail_star")
    elif identifier == "unrooted_tree_leaf_deletion":
        inp.line(n)
        if scenario == "maximum_star_all_leaves":
            inp.line(*(n + 1 for _ in range(n)))
            inp.line(*(n for _ in range(n - 1)))
            out.line(*range(1, n + 1))
        else:
            inp.line(*range(2, n + 2))
            inp.line(*range(2, n + 1))
            out.line(1, n + 1)
    elif identifier == "tree_path_difference_road":
        _tree_path_difference_road(inp, out, n, dimensions["q"], dimensions["r"],
                                   scenario == "wide_star_shared_spoke_updates")
    elif identifier == "walk_weight_congruence_return":
        tail = scenario == "small_period_three_and_long_outgoing_tail"
        inp.line(1)
        inp.line(n, dimensions["m"])
        for v in range(1, n):
            inp.line(v, v + 1)
        inp.line(3 if tail else n, 1)
        out.line("No" if tail else "Yes")
    elif identifier == "unit_step_forest_labels":
        star = scenario == "broad_star_conflicting_leaf_intervals"
        inp.line(n)
        for v in range(2, n + 1):
            inp.line(1 if star else v - 1, v)
        if star:
            inp.line(n - 1)
            for v in range(2, n + 1):
                inp.line(v, 4 if v == n else 0)
            out.line("No")
        else:
            inp.line(2)
            inp.line(1, 0)
            inp.line(n, n - 1)
            out.line("Yes")
            for v in range(n):
                out.line(v)
    elif identifier == "bipartite_coloring":
        _bipartite_coloring(inp, out, n, scenario == "long_odd_cycle")
    elif identifier == "bipartite_matching_allowed_edges":
        _bipartite_matching_allowed_edges(inp, out, n, scenario == "long_alternating_cycle")
    elif identifier == "block_cut_forest":
        _block_cut_forest(inp, out, n, scenario == "dead_end_cycle")
    elif identifier == "bottleneck_pair_histogram":
        inp.line(n)
        star = scenario == "maximum_equal_weight_star"
        for v in range(n, 1, -1):
            inp.line(1 if star else v - 1, v, dimensions["weight"] if star else v - 1)
        ans = n * (n - 1) // 2 * dimensions["weight"] if star else (n - 1) * n * (2 * n - 1) // 6
        out.line(ans)
    elif identifier == "bridge_block_orientation":
        _bridge_block_orientation(inp, out, n, scenario == "two_reachability_blocks")
    elif identifier == "complete_binary_tree":
        _complete_binary_tree(inp, out, n, dimensions["tests"], scenario == "largest_partial_last_level")
    elif identifier == "complement_shortest_paths":
        _complement_shortest_paths(inp, out, n, dimensions["m"], scenario == "isolated_terminal")
    elif identifier == "dag_critical_path_schedule":
        _dag_critical_path_schedule(inp, out, n, dimensions["m"], dimensions["duration"],
                                    scenario == "one_critical_branch_among_many")
    elif identifier == "dag_grundy":
        _dag_grundy(inp, out, n, dimensions["value"], dimensions["prime"], scenario == "one_unpaired_prime")
    elif identifier == "directed_game":
        _directed_game(inp, out, n, scenario == "long_shiritori_draw_cycle")
    elif identifier == "dynamic_tree_distances":
        _dynamic_tree_distances(inp, out, n, dimensions["q"], scenario == "star_leaf_distances")
    elif identifier == "edge_removal_distances":
        _edge_removal_distances(inp, out, n, dimensions["m"], scenario == "long_path_with_dense_dead_end")
    elif identifier == "enumerate_topological_orders":
        _enumerate_topological_orders(inp, out, n, dimensions["k"], scenario == "deep_chain_ending_at_cycle")
    elif identifier == "equal_tree_path_partition":
        length = dimensions["length"]
        inp.line(n // length, length)
        star = scenario == "maximum_star_pairing_impossible"
        for u in range(1, n):
            inp.line(1 if star else u, u + 1)
        out.line("No" if star else "Yes")
    elif identifier == "eulerian_trail_decomposition":
        _eulerian_trail_decomposition(inp, out, n, scenario == "many_parallel_arcs_and_loops")
    elif identifier == "functional_graph_fold":
        _functional_graph_fold(inp, out, n, dimensions["k"], scenario == "weighted_cycle_with_remainder")
    elif identifier == "grid_slide_distances":
        _grid_slide_distances(inp, out, n, scenario == "solid_barrier_row")
    elif identifier == "grid_stride_distances":
        _grid_stride_distances(inp, out, n, dimensions["w"], dimensions["k"], scenario == "wall_with_bottom_gap")
    elif identifier == "offline_dag_reachability":
        _dag_reachability(inp, out, n, dimensions["m"], dimensions["q"], scenario == "few_targets_two_chains")
    elif identifier == "palindromic_shortest_paths":
        _palindromic_shortest_paths(inp, out, n, scenario == "dense_disconnected_label_blocks")
    elif identifier == "parity_orientation":
        _parity_orientation(inp, out, n, scenario == "even_cycle")
    elif identifier == "incremental_weighted_forest":
        _incremental_weighted_forest(inp, out, n, dimensions["q"], scenario == "growing_weighted_star")
    elif identifier == "incremental_apsp":
        _incremental_apsp(inp, out, n, dimensions["q"], scenario == "dense_graph_isolating_one_city")
    elif identifier == "incremental_bridges":
        ring = scenario == "last_insert_closes_entire_chain"
        inp.line(n, n, 1, n // 2 + 1 if ring else n)
        for u in range(1, n):
            inp.line(u, u + 1)
        inp.line(1, n if ring else 3)
        out.line(n // 2 if ring else 1)
    elif identifier == "johnson_shortest_paths":
        _johnson_shortest_paths(inp, out, n, dimensions["weight"], scenario == "complete_graph_with_signed_potentials")
    elif identifier == "k_shortest_simple_paths":
        _k_shortest_simple_paths(inp, out, n, dimensions["m"], dimensions["k"], scenario == "full_edge_budget_equal_diamonds")
    elif identifier == "kruskal_reconstruction_tree":
        _kruskal_reconstruction_tree(inp, out, n, dimensions["w"], dimensions["q"], dimensions["height"],
                                    scenario == "low_crossing_column", plan["seed"])
    elif identifier == "lexicographic_shortest_path":
        if scenario == "dense_monotone_ties":
            inp.line(n, n * (n - 1) // 2, 0, n - 1)
            for u in range(n):
                for v in range(u + 1, n):
                    inp.line(u, v, v - u)
            out.line(*range(n))
        else:
            weight = dimensions["weight"]
            inp.line(n, n, 0, n // 2)
            for u in range(n - 1):
                inp.line(u, u + 1, weight)
            inp.line(0, n - 1, weight)
            out.line(*range(n // 2 + 1))
    elif identifier == "lower_bound_flow":
        _lower_bound_flow(inp, out, n, dimensions["w"], scenario == "odd_number_of_required_cells")
    elif identifier == "mst_sensitivity_delete":
        _mst_sensitivity(inp, out, n, scenario == "weighted_spokes")
    elif identifier == "near_tree_distances":
        _near_tree_distances(inp, out, n, dimensions["m"], dimensions["q"], plan["seed"])
    elif identifier == "neighborhood_assignment":
        _neighborhood_assignment(inp, out, n, dimensions["m"], dimensions["q"], dimensions.get("clique", 0))
    elif identifier == "virtual_tree_distance_product":
        _virtual_tree(inp, out, n, scenario == "chain_and_star")
    elif identifier == "range_edge_graph_express":
        _range_edge_graph(inp, out, n, dimensions["m"], scenario == "wide_both_directions", plan["seed"])
    elif identifier == "reachability_chain_cover":
        _reachability_chain_cover(inp, out, n, dimensions["width"])
    elif identifier == "time_dependent_paths_latest":
        _time_dependent_paths_latest(inp, out, n, dimensions["period"], dimensions["count"])
    elif identifier == "dynamic_color_component_sum":
        _dynamic_color_component_sum(inp, out, n, dimensions["q"], scenario == "star_center_and_leaf_flips")
    elif identifier == "static_top_tree":
        _static_top_tree(inp, out, n, dimensions["q"], scenario == "star_hash_updates")
    elif identifier == "tree_marked_diameter":
        _tree_marked_diameter(inp, out, n, dimensions["q"], scenario == "star_leaf_changes")
    elif identifier == "tree_nearest_marked":
        _nearest_marked(inp, out, n, dimensions["q"], scenario == "star_leaf_toggles")
    elif identifier == "topological_order_counter":
        _topological_order_counter(inp, out, n, scenario == "interleaving_two_dense_chains")
    elif identifier == "tree_distance_layers":
        _tree_distance_layers(inp, out, n, dimensions["q"], scenario == "wide_star_multiple_witnesses")
    elif identifier == "tree_mo":
        _tree_mo(inp, out, n, dimensions["q"], scenario == "star_distinct_labels", plan["seed"])
    elif identifier == "tree_path_bitwise_counts_and":
        weight = dimensions["weight"]
        star = scenario == "maximum_star_same_wide_mask"
        inp.line(n)
        for u in range(1, n):
            inp.line(1 if star else u, u + 1, weight if star or u % 2 == 0 else 1)
        pairs = n * (n - 1) // 2 * weight if star else (n // 2) + ((n - 1) // 2) * weight
        out.line(pairs % 1000000007)
    elif identifier == "two_nearest_source_groups":
        _two_nearest_source_groups(inp, out, n, dimensions["weight"], scenario == "all_vertices_popular_in_star")
    elif identifier == "undirected_pseudoforest":
        _undirected_pseudoforest(inp, out, n, dimensions["q"], scenario == "entire_graph_is_cycle")
    elif identifier == "undirected_xor_walks":
        _undirected_xor_walks(inp, out, n, dimensions["bits"], scenario == "full_rank_closed_walks")
    elif identifier == "walk_weight_ranges":
        _walk_weight_ranges(inp, out, n, scenario == "alternating_label_cycle")
    elif identifier == "xor_walk_reachability":
        _xor_walk_reachability(inp, out, n, dimensions["m"], dimensions["bits"],
                               scenario == "all_states_along_chain")
    elif identifier == "shortest_path_edge_usage":
        _shortest_path_edge_usage(inp, out, n, scenario == "equal_length_layers")
    elif identifier == "shortest_walk_counts":
        _shortest_walk_counts(inp, out, n, dimensions["layers"])
    elif identifier == "shortest_weighted_cycle":
        _shortest_weighted_cycle(inp, out, n, scenario == "heavy_directed_ring")
    elif identifier == "dynamic_steiner_subtree":
        inp.line(n)
        star = scenario == "star_with_cuts"
        _tree(inp, n, star)
        oracle = _star_steiner if star else _path_steiner
        oracle(inp, out, n, dimensions["q"], plan["seed"])
    else:
        raise ValueError(f"unknown graph performance Verify ID: {identifier}")
