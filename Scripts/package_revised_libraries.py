#!/usr/bin/env python3
"""整備済みの本体・テンプレートと使用例から、同期した配布ZIPを作る。"""

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
    "angle_sort_example.cpp": "Geometry/偏角ソート.cpp",
    "node_example.cpp": "DataStructure/Node.cpp",
    "trie_example.cpp": "String/Trie.cpp",
    "hash_set_example.cpp": "DataStructure/Hash_set.cpp",
    "line_container_example.cpp": "DataStructure/line-container.cpp",
    "line_container_2d_example.cpp": "DataStructure/line-container-2d.cpp",
    "union_find_example.cpp": "DataStructure/union_find.cpp",
    "union_find_w_example.cpp": "DataStructure/union_find_w.cpp",
    "rollback_union_find_example.cpp": "DataStructure/rollback-union-find.cpp",
    "persistent_union_find_example.cpp": "DataStructure/persistent-union-find.cpp",
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
}


def check_generated(root: Path) -> None:
    for generator in ("generate_geometry.py", "generate_zeta.py", "generate_combination.py", "generate_formal_power_series.py", "generate_linear_recurrence.py", "generate_gaussian_elimination.py", "generate_binary_matrix.py", "generate_convolution.py", "generate_polynomial.py", "generate_fast_sources.py", "generate_variants.py", "prepare_variant_verify.py"):
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
    for template in ("temp.cpp", "temp2.cpp"):
        entries[template] = (root / template).read_bytes()
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
    for name in ("Readme.md", "AGENTS.md", "CLAUDE.md", ".github/workflows/verify.yml", "Scripts/package_revised_libraries.py", "Scripts/example_dependencies.py"):
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
    for source_path in sorted((root / "Verify" / "optimization").rglob("*")):
        if source_path.is_file() and source_path.suffix in (".md", ".json"):
            entries[source_path.relative_to(root).as_posix()] = source_path.read_bytes()
    license_path = root / "LICENSE"
    if license_path.is_file():
        entries["LICENSE"] = license_path.read_bytes()
    for license_path in sorted((root / "LICENSES").glob("*.txt")):
        entries[license_path.relative_to(root).as_posix()] = license_path.read_bytes()
    for audit_path in sorted((root / "Audit").rglob("*")):
        if (audit_path.is_file() and "__pycache__" not in audit_path.parts and
                audit_path.suffix in (".json", ".md", ".png", ".cpp", ".hpp", ".py", ".inc", ".gz", ".zip", ".bin")):
            entries[audit_path.relative_to(root).as_posix()] = audit_path.read_bytes()
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
