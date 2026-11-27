#!/usr/bin/env python3
"""明示的に必要な場合だけ、Releases添付用の全体ZIPを作る。"""

import argparse
from pathlib import Path
import subprocess
import sys

try:
    from .deterministic_zip import archive_versions, write_archive
    from .example_dependencies import expand_example_dependencies
    from .prepare_verify import expand_acl
except ImportError:
    from deterministic_zip import archive_versions, write_archive
    from example_dependencies import expand_example_dependencies
    from prepare_verify import expand_acl


LIBRARIES = {
    "bellman_ford_example.cpp": "Graph/Bellman-Ford.cpp",
    "spfa_example.cpp": "Graph/Bellman-Ford_SFPA.cpp",
    "binary_trie_example.cpp": "DataStructure/Binary_Trie.cpp",
    "compressed_binary_trie_example.cpp": "DataStructure/Compressed_Binary_Trie.cpp",
    "binomial_prime_example.cpp": "Math/Combination_AtCoderMod.cpp",
    "binomial_example.cpp": "Math/Combination_Mod.cpp",
    "binomial_atcoder_example.cpp": "Math/Combination_Mod.cpp",
    "dijkstra_example.cpp": "Graph/Dijkstra.cpp",
    "primality_example.cpp": "Math/prime_judge(Miller-Rabin).cpp",
    "dynamic_fenwick_example.cpp": "DataStructure/dynamic-binary-indexed-tree.cpp",
    "dynamic_fenwick_2d_example.cpp": "DataStructure/dynamic-binary-indexed-tree-2d.cpp",
    "dynamic_li_chao_example.cpp": "DataStructure/dynamic-li-chao-tree.cpp",
    "dynamic_union_find_example.cpp": "DataStructure/dynamic-union-find.cpp",
    "hash_map_example.cpp": "DataStructure/Hash_map.cpp",
    "internal_hash_map_example.cpp": "Internal/Nyaan_hash_map_min.cpp",
    "erasable_priority_queue_example.cpp": "DataStructure/erasable-priority-queue.cpp",
    "eratosthenes_example.cpp": "Math/Eratosthenes.cpp",
    "fraction_example.cpp": "Math/Fraction.cpp",
    "floyd_warshall_example.cpp": "Graph/Floyd-Warshall.cpp",
    "rolling_hash_example.cpp": "String/RollingHash.cpp",
    "project_selection_example.cpp": "Graph/Project_Selection.cpp",
    "zeta_mobius_example.cpp": "Math/Zeta_Mebius.cpp",
    "zeta_set_example.cpp": "Math/Zeta/Set_Transform.cpp",
    "zeta_bitwise_example.cpp": "Math/Zeta/Bitwise_Convolution.cpp",
    "zeta_sieve_example.cpp": "Math/Zeta/Sieve.cpp",
    "zeta_integer_example.cpp": "Math/Zeta/Divisor_Multiple_Transform.cpp",
    "zeta_convolution_example.cpp": "Math/Zeta/Gcd_Lcm_Convolution.cpp",
    "zeta_sparse_example.cpp": "Math/Zeta/Sparse_Transform.cpp",
    "combination_integer_example.cpp": "Math/Combination/Binomial_Mod.cpp",
    "combination_modint_example.cpp": "Math/Combination/ModInt_Binomial.cpp",
    "combination_prime_example.cpp": "Math/Combination/Prime_Binomial.cpp",
    "radix_sort_example.cpp": "Algorithm/Radix_sort.cpp",
    "lca_example.cpp": "Graph/LCA.cpp",
    "nearest_two_points_example.cpp": "Geometry/Nearest_Two_points.cpp",
    "heap_example.cpp": "DataStructure/heap.cpp",
    "implicit_treap_example.cpp": "DataStructure/implicit_treap.cpp",
    "implicit_treap_erase_between_example.cpp": "DataStructure/implicit_treap.cpp",
    "geometry_example.cpp": "Geometry/geometory.cpp",
    "rollback_weighted_union_find_example.cpp": "DataStructure/rollback-union-findw.cpp",
    "matrix_transform_example.cpp": "Algorithm/rotate.cpp",
    "angle_sort_example.cpp": "Geometry/angle_sort.cpp",
    "node_example.cpp": "DataStructure/Node.cpp",
    "trie_example.cpp": "String/Trie.cpp",
    "hash_set_example.cpp": "DataStructure/Hash_set.cpp",
    "line_container_example.cpp": "DataStructure/line-container.cpp",
    "line_container_2d_example.cpp": "DataStructure/line-container-2d.cpp",
    "union_find_example.cpp": "DataStructure/union_find.cpp",
    "union_find_w_example.cpp": "DataStructure/union_find_w.cpp",
    "rollback_union_find_example.cpp": "DataStructure/rollback-union-find.cpp",
    "persistent_union_find_example.cpp": "DataStructure/persistent-union-find.cpp",
    "persistent_array_example.cpp": "DataStructure/Persistent_Array.cpp",
    "floor_sum_example.cpp": "Math/Floor_Sum.cpp",
    "monge_optimization_example.cpp": "Algorithm/Monge_Optimization.cpp",
    "manhattan_mst_example.cpp": "Geometry/Manhattan_MST.cpp",
    "dominator_tree_example.cpp": "Graph/Dominator_Tree.cpp",
    "minimum_spanning_tree_example.cpp": "Graph/Minimum_Spanning_Tree.cpp",
    "lyndon_factorization_example.cpp": "String/Lyndon_Factorization.cpp",
    "stern_brocot_tree_example.cpp": "Math/Stern_Brocot_Tree.cpp",
    "floor_quotients_example.cpp": "Math/Floor_Quotients.cpp",
    "integer_root_example.cpp": "Math/Integer_Root.cpp",
    "discrete_logarithm_example.cpp": "Math/Discrete_Logarithm.cpp",
    "rectangle_sum_example.cpp": "Geometry/Rectangle_Sum.cpp",
    "tree_diameter_example.cpp": "Graph/Tree_Diameter.cpp",
    "offline_distinct_count_example.cpp": "Algorithm/Offline_Distinct_Count.cpp",
    "prefix_sum_example.cpp": "Algorithm/Prefix_Sum.cpp",
    "prefix_sum_nd_example.cpp": "Algorithm/Prefix_Sum_ND.cpp",
    "lis_example.cpp": "Algorithm/LIS.cpp",
    "topological_example.cpp": "Graph/Topological.cpp",
    "topological_lexicographical_example.cpp": "Graph/Topological_sort.cpp",
    "next_combination_example.cpp": "Algorithm/next_combination.cpp",
    "fenwick_tree_example.cpp": "DataStructure/Fenwick_Tree.cpp",
    "segment_tree_example.cpp": "DataStructure/Segment_Tree.cpp",
    "lazy_segment_tree_example.cpp": "DataStructure/Lazy_Segment_Tree.cpp",
    "lazy_segment_tree_add_sum_example.cpp": "DataStructure/Lazy_Segment_Tree.cpp",
    "sparse_table_example.cpp": "DataStructure/Sparse_Table.cpp",
    "disjoint_sparse_table_example.cpp": "DataStructure/Disjoint_Sparse_Table.cpp",
    "scc_example.cpp": "Graph/SCC.cpp",
    "two_sat_example.cpp": "Graph/Two_SAT.cpp",
    "heavy_light_decomposition_example.cpp": "Graph/Heavy_Light_Decomposition.cpp",
    "max_flow_example.cpp": "Graph/Max_Flow.cpp",
    "z_algorithm_example.cpp": "String/Z_Algorithm.cpp",
    "manacher_example.cpp": "String/Manacher.cpp",
    "suffix_array_example.cpp": "String/Suffix_Array.cpp",
    "ntt_example.cpp": "Math/NTT.cpp",
    "wavelet_matrix_example.cpp": "DataStructure/Wavelet_Matrix.cpp",
    "low_link_example.cpp": "Graph/Low_Link.cpp",
    "eulerian_trail_example.cpp": "Graph/Eulerian_Trail.cpp",
    "min_cost_flow_example.cpp": "Graph/Min_Cost_Flow.cpp",
    "pollard_rho_example.cpp": "Math/Pollard_Rho.cpp",
    "aho_corasick_example.cpp": "String/Aho_Corasick.cpp",
    "formal_power_series_example.cpp": "Math/Formal_Power_Series.cpp",
    "persistent_segment_tree_example.cpp": "DataStructure/Persistent_Segment_Tree.cpp",
    "pruefer_code_example.cpp": "Graph/Pruefer_Code.cpp",
    "centroid_decomposition_example.cpp": "Graph/Centroid_Decomposition.cpp",
    "segment_tree_beats_example.cpp": "DataStructure/Segment_Tree_Beats.cpp",
    "sliding_window_aggregation_example.cpp": "DataStructure/Sliding_Window_Aggregation.cpp",
    "xor_basis_example.cpp": "Math/Xor_Basis.cpp",
    "bipartite_matching_example.cpp": "Graph/Bipartite_Matching.cpp",
    "rerooting_dp_example.cpp": "Graph/Rerooting_DP.cpp",
    "offline_dynamic_connectivity_example.cpp": "Graph/Offline_Dynamic_Connectivity.cpp",
    "functional_graph_example.cpp": "Graph/Functional_Graph.cpp",
    "berlekamp_massey_example.cpp": "Math/Berlekamp_Massey.cpp",
    "linear_recurrence_example.cpp": "Math/Linear_Recurrence.cpp",
    "link_cut_tree_example.cpp": "DataStructure/Link_Cut_Tree.cpp",
    "link_cut_tree_composite_example.cpp": "DataStructure/Link_Cut_Tree.cpp",
    "link_cut_tree_lazy_example.cpp": "DataStructure/Link_Cut_Tree.cpp",
    "dual_segment_tree_example.cpp": "DataStructure/Dual_Segment_Tree.cpp",
    "slope_trick_example.cpp": "DataStructure/Slope_Trick.cpp",
    "dynamic_bitset_example.cpp": "DataStructure/Dynamic_Bitset.cpp",
    "suffix_automaton_example.cpp": "String/Suffix_Automaton.cpp",
    "palindromic_tree_example.cpp": "String/Palindromic_Tree.cpp",
    "matrix_example.cpp": "Math/Matrix.cpp",
    "gaussian_elimination_example.cpp": "Math/Gaussian_Elimination.cpp",
    "binary_matrix_example.cpp": "Math/Binary_Matrix.cpp",
    "mo_example.cpp": "Algorithm/Mo.cpp",
    "convolution_example.cpp": "Math/Convolution.cpp",
    "polynomial_example.cpp": "Math/Polynomial.cpp",
    "cartesian_tree_example.cpp": "Graph/Cartesian_Tree.cpp",
    "general_matching_example.cpp": "Graph/General_Matching.cpp",
    "modular_arithmetic_example.cpp": "Math/Modular_Arithmetic.cpp",
    "range_inversions_example.cpp": "Algorithm/Range_Inversions.cpp",
    "subset_sum_example.cpp": "Algorithm/Subset_Sum.cpp",
    "double_ended_priority_queue_example.cpp": "DataStructure/Double_Ended_Priority_Queue.cpp",
    "offline_rectangle_add_point_get_example.cpp": "DataStructure/Offline_Rectangle_Add_Point_Get.cpp",
    "range_majority_example.cpp": "DataStructure/Range_Majority.cpp",
    "furthest_pair_example.cpp": "Geometry/Furthest_Pair.cpp",
    "minimum_enclosing_circle_example.cpp": "Geometry/Minimum_Enclosing_Circle.cpp",
    "minkowski_sum_example.cpp": "Geometry/Minkowski_Sum.cpp",
    "rectangle_union_area_example.cpp": "Geometry/Rectangle_Union_Area.cpp",
    "assignment_example.cpp": "Graph/Assignment.cpp",
    "complement_components_example.cpp": "Graph/Complement_Components.cpp",
    "cycle_detection_example.cpp": "Graph/Cycle_Detection.cpp",
    "enumerate_cliques_example.cpp": "Graph/Enumerate_Cliques.cpp",
    "enumerate_triangles_example.cpp": "Graph/Enumerate_Triangles.cpp",
    "rooted_tree_isomorphism_example.cpp": "Graph/Rooted_Tree_Isomorphism.cpp",
    "steiner_tree_example.cpp": "Graph/Steiner_Tree.cpp",
    "zero_one_bfs_example.cpp": "Graph/Zero_One_BFS.cpp",
    "characteristic_polynomial_example.cpp": "Math/Characteristic_Polynomial.cpp",
    "determinant_arbitrary_mod_example.cpp": "Math/Determinant_Arbitrary_Mod.cpp",
    "prime_counting_example.cpp": "Math/Prime_Counting.cpp",
    "tetration_mod_example.cpp": "Math/Tetration_Mod.cpp",
    "edit_distance_example.cpp": "String/Edit_Distance.cpp",
    "longest_common_subsequence_example.cpp": "String/Longest_Common_Subsequence.cpp",
    "prefix_function_example.cpp": "String/Prefix_Function.cpp",
    "point_set_range_frequency_example.cpp": "DataStructure/Point_Set_Range_Frequency.cpp",
    "minimum_directed_spanning_tree_example.cpp": "Graph/Minimum_Directed_Spanning_Tree.cpp",
    "run_enumerate_example.cpp": "String/Run_Enumerate.cpp",
    "static_rectangle_add_rectangle_sum_example.cpp": "DataStructure/Static_Rectangle_Add_Rectangle_Sum.cpp",
    "set_power_series_example.cpp": "Math/Set_Power_Series.cpp",
    "knapsack_example.cpp": "Algorithm/Knapsack.cpp",
    "convex_layers_example.cpp": "Geometry/Convex_Layers.cpp",
    "k_shortest_walks_example.cpp": "Graph/K_Shortest_Walks.cpp",
    "euclidean_mst_example.cpp": "Geometry/Euclidean_MST.cpp",
    "dynamic_bipartiteness_example.cpp": "Graph/Dynamic_Bipartiteness.cpp",
    "maximum_independent_set_example.cpp": "Graph/Maximum_Independent_Set.cpp",
    "range_xor_basis_example.cpp": "Math/Range_Xor_Basis.cpp",
    "persistent_queue_example.cpp": "DataStructure/Persistent_Queue.cpp",
    "combinatorial_sequences_example.cpp": "Math/Combinatorial_Sequences.cpp",
    "range_parallel_union_find_example.cpp": "DataStructure/Range_Parallel_Union_Find.cpp",
    "count_points_in_triangle_example.cpp": "Geometry/Count_Points_In_Triangle.cpp",
    "counting_spanning_trees_example.cpp": "Graph/Counting_Spanning_Trees.cpp",
    "wildcard_pattern_matching_example.cpp": "String/Wildcard_Pattern_Matching.cpp",
    "polynomial_prefix_sum_example.cpp": "Math/Polynomial_Prefix_Sum.cpp",
    "static_range_mode_example.cpp": "DataStructure/Static_Range_Mode.cpp",
    "chordal_graph_example.cpp": "Graph/Chordal_Graph.cpp",
    "count_four_cycles_example.cpp": "Graph/Count_Four_Cycles.cpp",
    "palindromes_in_deque_example.cpp": "String/Palindromes_In_Deque.cpp",
    "tree_distance_frequency_example.cpp": "Graph/Tree_Distance_Frequency.cpp",
    "modular_kth_root_example.cpp": "Math/Modular_Kth_Root.cpp",
    "prefix_substring_lcs_example.cpp": "String/Prefix_Substring_LCS.cpp",
    "multivariate_convolution_example.cpp": "Math/Multivariate_Convolution.cpp",
    "range_linear_add_range_min_example.cpp": "DataStructure/Range_Linear_Add_Range_Min.cpp",
    "bipartite_edge_coloring_example.cpp": "Graph/Bipartite_Edge_Coloring.cpp",
    "multiplicative_convolution_example.cpp": "Math/Multiplicative_Convolution.cpp",
    "number_of_subsequences_example.cpp": "String/Number_Of_Subsequences.cpp",
    "global_min_cut_example.cpp": "Graph/Global_Min_Cut.cpp",
    "st_numbering_example.cpp": "Graph/ST_Numbering.cpp",
    "static_range_lis_example.cpp": "Algorithm/Static_Range_LIS.cpp",
    "range_sort_aggregation_example.cpp": "DataStructure/Range_Sort_Aggregation.cpp",
    "adjugate_matrix_example.cpp": "Math/Adjugate_Matrix.cpp",
    "incremental_scc_example.cpp": "Graph/Incremental_SCC.cpp",
    "consecutive_polynomial_evaluation_example.cpp": "Math/Consecutive_Polynomial_Evaluation.cpp",
    "concave_min_plus_convolution_example.cpp": "Algorithm/Concave_Min_Plus_Convolution.cpp",
    "noncommutative_weighted_union_find_example.cpp": "DataStructure/Noncommutative_Weighted_Union_Find.cpp",
    "all_furthest_neighbors_example.cpp": "Geometry/All_Furthest_Neighbors.cpp",
    "incremental_minimum_spanning_forest_example.cpp": "Graph/Incremental_Minimum_Spanning_Forest.cpp",
    "offline_point_add_rectangle_sum_example.cpp": "DataStructure/Offline_Point_Add_Rectangle_Sum.cpp",
    "common_interval_decomposition_example.cpp": "Algorithm/Common_Interval_Decomposition.cpp",
    "point_rectangle_affine_kd_tree_example.cpp": "DataStructure/Point_Rectangle_Affine_KD_Tree.cpp",
    "minimum_diameter_spanning_tree_example.cpp": "Graph/Minimum_Diameter_Spanning_Tree.cpp",
    "palindromic_factorization_example.cpp": "String/Palindromic_Factorization.cpp",
    "tree_decomposition_width_two_example.cpp": "Graph/Tree_Decomposition_Width_Two.cpp",
    "primitive_root_example.cpp": "Math/Primitive_Root.cpp",
    "sparse_affine_range_sum_example.cpp": "DataStructure/Sparse_Affine_Range_Sum.cpp",
    "chromatic_number_example.cpp": "Graph/Chromatic_Number.cpp",
    "pfaffian_example.cpp": "Math/Pfaffian.cpp",
    "sparse_matrix_determinant_example.cpp": "Math/Sparse_Matrix_Determinant.cpp",
    "two_square_sum_example.cpp": "Math/Two_Square_Sum.cpp",
    "weighted_wavelet_matrix_example.cpp": "DataStructure/Weighted_Wavelet_Matrix.cpp",
    "counting_squarefrees_example.cpp": "Math/Counting_Squarefrees.cpp",
    "nim_product_example.cpp": "Math/Nim_Product.cpp",
    "summatory_totient_example.cpp": "Math/Summatory_Totient.cpp",
    "three_edge_connected_components_example.cpp": "Graph/Three_Edge_Connected_Components.cpp",
    "gaussian_integer_gcd_example.cpp": "Math/Gaussian_Integer_GCD.cpp",
    "persistent_affine_range_sum_example.cpp": "DataStructure/Persistent_Affine_Range_Sum.cpp",
    "tree_topological_order_minimum_inversions_example.cpp": "Graph/Tree_Topological_Order_Minimum_Inversions.cpp",
    "exponential_polynomial_sum_example.cpp": "Math/Exponential_Polynomial_Sum.cpp",
    "polynomial_sequence_product_example.cpp": "Math/Polynomial_Sequence_Product.cpp",
    "range_assign_range_product_example.cpp": "DataStructure/Range_Assign_Range_Product.cpp",
    "sparse_segment_tree_example.cpp": "DataStructure/Sparse_Segment_Tree.cpp",
    "tree_contour_example.cpp": "Graph/Tree_Contour.cpp",
    "geometric_polynomial_example.cpp": "Math/Geometric_Polynomial.cpp",
    "sparse_formal_power_series_example.cpp": "Math/Sparse_Formal_Power_Series.cpp",
    "tree_b_matching_example.cpp": "Graph/Tree_B_Matching.cpp",
    "consecutive_linear_recurrence_example.cpp": "Math/Consecutive_Linear_Recurrence.cpp",
    "newton_basis_example.cpp": "Math/Newton_Basis.cpp",
    "deque_aggregation_example.cpp": "DataStructure/Deque_Aggregation.cpp",
    "xor_basis_intersection_example.cpp": "Math/Xor_Basis_Intersection.cpp",
    "euler_tour_tree_example.cpp": "DataStructure/Euler_Tour_Tree.cpp",
    "hierarchical_bitset_example.cpp": "DataStructure/Hierarchical_Bitset.cpp",
    "walsh_hadamard_example.cpp": "Math/Walsh_Hadamard.cpp",
}


def check_generated(root: Path) -> None:
    subprocess.run([sys.executable, str(root / "Scripts" / "generate_persistent_union_find.py"), "--check"], check=True)
    for generator in ("generate_geometry.py", "generate_zeta.py", "generate_combination.py", "generate_formal_power_series.py", "generate_linear_recurrence.py", "generate_gaussian_elimination.py", "generate_binary_matrix.py", "generate_convolution.py", "generate_polynomial.py", "generate_tree_distance_frequency.py", "generate_incremental_scc.py", "generate_incremental_minimum_spanning_forest.py", "generate_all_furthest_neighbors.py", "generate_concave_min_plus_convolution.py", "generate_factorization_helpers.py", "generate_sparse_matrix_determinant.py", "generate_fast_sources.py", "generate_variants.py", "prepare_variant_verify.py"):
        subprocess.run([sys.executable, str(root / "Scripts" / generator), "--check"], check=True)
    subprocess.run([sys.executable, str(root / "Scripts" / "check_source_api_docs.py"), "--check"], check=True)
    subprocess.run([sys.executable, str(root / "Scripts" / "prepare_verify.py"), "--check"], check=True)


def package(output: Path) -> None:
    root = Path(__file__).resolve().parents[1]
    check_generated(root)
    entries = {}
    for library in dict.fromkeys((*LIBRARIES.values(), "Internal/Nyaan_hash_map_min.cpp")):
        entries[library] = (root / library).read_bytes()
    known_sources = set(entries)
    entries["template.cpp"] = (root / "template.cpp").read_bytes()
    for example, library in LIBRARIES.items():
        name = "Examples/" + example
        source = (root / name).read_text(encoding="utf-8")
        include = f'#include "../{library}"'
        if source.count(include) != 1:
            raise ValueError(f"ライブラリのincludeを特定できません: {name}")
        entries[name] = source.encode("utf-8")
        standalone = expand_acl(expand_example_dependencies(
            source, name, known_sources, lambda dependency: entries[dependency].decode("utf-8")))
        entries["StandaloneExamples/" + example] = standalone.encode("utf-8")
    for name in ("Readme.md", ".clang-tidy", ".github/workflows/verify.yml", "Scripts/package_revised_libraries.py", "Scripts/example_dependencies.py"):
        entries[name] = (root / name).read_bytes()
    for generator in ("generate_geometry.py", "generate_zeta.py", "generate_combination.py"):
        entries["Scripts/" + generator] = (root / "Scripts" / generator).read_bytes()
    for name in ("Scripts/prepare_verify.py", "Scripts/library_layout.json"):
        entries[name] = (root / name).read_bytes()
    for name in ("source_api_docs.py", "check_source_api_docs.py"):
        entries["Scripts/" + name] = (root / "Scripts" / name).read_bytes()
    for source_path in sorted((root / "Scripts").rglob("*")):
        if source_path.is_file() and source_path.suffix in (".py", ".json", ".inc") and "__pycache__" not in source_path.parts:
            entries[source_path.relative_to(root).as_posix()] = source_path.read_bytes()
    for directory in ("Graph", "DataStructure", "Math", "String", "Geometry", "Algorithm", "Internal", "Document"):
        for source_path in sorted((root / directory).rglob("*")):
            if source_path.is_file() and source_path.suffix in (".cpp", ".md", ".inc", ".txt"):
                entries[source_path.relative_to(root).as_posix()] = source_path.read_bytes()
    for directory in ("Variants", "Fast"):
        for source_path in sorted((root / directory).rglob("*")):
            if source_path.is_file() and source_path.suffix in (".cpp", ".md", ".json", ".inc"):
                entries[source_path.relative_to(root).as_posix()] = source_path.read_bytes()
    for source_path in sorted((root / "Verify" / "current").rglob("*")):
        if source_path.is_file():
            entries[source_path.relative_to(root).as_posix()] = source_path.read_bytes()
    for source_path in sorted((root / "Verify" / "ci").rglob("*")):
        if source_path.is_file() and source_path.suffix in (".cpp", ".hpp", ".md", ".json"):
            entries[source_path.relative_to(root).as_posix()] = source_path.read_bytes()
    license_path = root / "LICENSE"
    if license_path.is_file():
        entries["LICENSE"] = license_path.read_bytes()
    for license_path in sorted((root / "LICENSES").glob("*.txt")):
        entries[license_path.relative_to(root).as_posix()] = license_path.read_bytes()
    write_archive(output, entries)
    print(f"{len(entries)}ファイルを格納: {output.resolve()}")
    print(f"ZIP環境: {archive_versions()}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="出力ZIPのパス")
    args = parser.parse_args()
    package(args.output)


if __name__ == "__main__":
    main()
