#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""原本のガードを保ったまま、同梱版の公開ソースを生成する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source


BUNDLES = {
    "Algorithm/Josephus.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Geometry/Circle_Tangents.cpp": ("Geometry/Circle.cpp", "円と平面ベクトル"),
    "Math/Labeled_Tree_Matching_Counts.cpp": ("Math/Formal_Power_Series.cpp", "形式的冪級数"),
    "Graph/Tournament_Strong_Counts.cpp": ("Math/Formal_Power_Series.cpp", "形式的冪級数"),
    "Graph/Hub_Sparse_Distances.cpp": ("DataStructure/union_find.cpp", "UnionFind"),
    "DataStructure/Binary_Range_Statistics.cpp": ("DataStructure/Lazy_Segment_Tree.cpp", "遅延セグメント木"),
    "Algorithm/Weighted_Interval_Stabbing.cpp": ("DataStructure/Lazy_Segment_Tree.cpp", "遅延セグメント木"),
    "DataStructure/Range_Add_Weighted_Minimum.cpp": ("DataStructure/Lazy_Segment_Tree.cpp", "遅延セグメント木"),
    "DataStructure/Successor_Set.cpp": ("DataStructure/union_find.cpp", "UnionFind"),
    "String/Three_Symbol_Frequency_Counts.cpp": ("DataStructure/Hash_map.cpp", "HashMap"),
    "Algorithm/Subarray_Frequency_Windows.cpp": ("DataStructure/Hash_map.cpp", "HashMap"),
    "Algorithm/Forbidden_Sum_Partitions.cpp": ("DataStructure/Hash_map.cpp", "HashMap"),
    "Algorithm/Target_Sum_Subrectangles.cpp": ("DataStructure/Hash_map.cpp", "HashMap"),
    "Algorithm/Adjacent_Swap_Distance.cpp": ("Algorithm/Range_Inversions.cpp", "転倒数"),
    "Algorithm/Disjoint_Subarray_Max_Sum.cpp": ("Algorithm/Nonadjacent_Selection.cpp", "非隣接選択"),
    "Algorithm/Bounded_Zigzag_Subsequence.cpp": ("DataStructure/Segment_Tree.cpp", "Segment Tree"),
    "Algorithm/Cyclic_Inversion_Statistics.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Algorithm/Interval_Cover_Subsets.cpp": ("DataStructure/Lazy_Segment_Tree.cpp", "遅延セグメント木"),
    "Algorithm/Interval_LIS.cpp": ("DataStructure/implicit_treap.cpp", "遅延伝播付き暗黙Treap"),
    "Algorithm/Optimal_Adjacent_Merge.cpp": ("DataStructure/implicit_treap.cpp", "遅延伝播付き暗黙Treap"),
    "Algorithm/Longest_Bitonic_Subsequence.cpp": ("Algorithm/LIS.cpp", "最長増加部分列"),
    "Algorithm/Inversion_Limited_Partitions.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Algorithm/Subarray_Sum_Counts.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Algorithm/Dominance_Counts.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Algorithm/Permutation_Order.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Algorithm/Weighted_Monotone_Matching.cpp": ("DataStructure/Segment_Tree.cpp", "Segment Tree"),
    "DataStructure/Persistent_Weighted_Union_Find.cpp": ("DataStructure/Persistent_Array.cpp", "永続配列"),
    "DataStructure/Point_Set_Mex.cpp": ("DataStructure/Hierarchical_Bitset.cpp", "階層化ビット集合"),
    "DataStructure/Monotone_Value_Shift.cpp": ("DataStructure/Segment_Tree.cpp", "Segment Tree"),
    "DataStructure/Range_Equal_Runs.cpp": ("DataStructure/Segment_Tree.cpp", "Segment Tree"),
    "DataStructure/Range_Nondecreasing_Subsequence.cpp": ("DataStructure/Segment_Tree.cpp", "Segment Tree"),
    "DataStructure/Range_Fenwick_Tree.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Geometry/Chord_Crossing_Queries.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Geometry/Orthogonal_Polygon_Coverage.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "Geometry/L1_Origin_Line_Fit.cpp": ("Math/Fraction.cpp", "既約分数"),
    "Geometry/Integer_Geometry_3D.cpp": ("Math/Fraction.cpp", "既約分数と固定幅整数"),
    "Geometry/Maximum_Weight_Window.cpp": ("DataStructure/Lazy_Segment_Tree.cpp", "遅延セグメント木"),
    "Geometry/Static_Cuboid_Sum.cpp": ("DataStructure/Offline_Point_Add_Rectangle_Sum.cpp", "点加算・長方形和"),
    "Graph/LCM_Spanning_Tree.cpp": ("DataStructure/union_find.cpp", "UnionFind"),
    "String/Cyclic_Rotation_Order.cpp": ("String/Suffix_Array.cpp", "接尾辞配列"),
    "String/Lexicographic_String_Shuffle.cpp": ("String/Suffix_Array.cpp", "接尾辞配列"),
    "String/Multi_String_Longest_Common_Substring.cpp": ("String/Suffix_Array.cpp", "接尾辞配列"),
    "String/Offline_Prefix_Lcp_Sum.cpp": ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    "String/Bracket_Sequence.cpp": ("DataStructure/Segment_Tree.cpp", "Segment Tree"),
    "String/Cyclic_Xor_Matches.cpp": ("String/Z_Algorithm.cpp", "Z Algorithm"),
    "String/Run_Length_Z.cpp": ("String/Z_Algorithm.cpp", "Z Algorithm"),
    "String/Dictionary_Segmenter.cpp": ("String/Aho_Corasick.cpp", "Aho–Corasick"),
    "String/Pattern_Range_Occurrences.cpp": ("String/Prefix_Function.cpp", "接頭辞関数"),
    "String/Periodic_Pattern_Runs.cpp": ("String/Prefix_Function.cpp", "接頭辞関数"),
    "String/Pattern_Erasure.cpp": ("String/Prefix_Function.cpp", "Prefix Function"),
    "String/Sorted_Substring_Index.cpp": ("DataStructure/Segment_Tree.cpp", "Segment Tree"),
    "String/Shortest_Common_Superstring.cpp": ("String/Prefix_Function.cpp", "Prefix Function"),
    "String/String_Periods.cpp": ("String/Prefix_Function.cpp", "Prefix Function"),
    "String/Substring_Index.cpp": ("String/Suffix_Array.cpp", "接尾辞配列"),
    "String/Suffix_Tree.cpp": ("String/Suffix_Array.cpp", "接尾辞配列"),
    "Math/Set_Series_Composition.cpp": ("Math/Set_Power_Series.cpp", "集合冪級数"),
    "Math/Linear_Matroid_Parity.cpp": ("Math/Gaussian_Elimination.cpp", "掃き出し法"),
    "Math/Matrix_Pencil_Determinant.cpp": ("Math/Characteristic_Polynomial.cpp", "特性多項式"),
    "Math/Modular_Shift_Minimum.cpp": ("Math/Floor_Sum.cpp", "Floor Sum"),
    "Math/Modular_Shift_Mex.cpp": ("Math/Floor_Sum.cpp", "Floor Sum"),
    "Math/Permutation_Comparison_Count.cpp": ("Math/Convolution.cpp", "CRT畳み込み"),
}

COMPOSITE_BUNDLES = {
    "Geometry/Delaunay_Triangulation.cpp": (
        ("Geometry/Euclidean_MST.cpp", "ユークリッド最小全域木"),
        ("Math/Fraction.cpp", "固定幅整数"),
    ),
    "Math/Divisibility_Chains.cpp": (
        ("Math/Zeta/Sieve.cpp", "最小素因数表"),
        ("Math/Pollard_Rho.cpp", "64ビット素因数分解"),
    ),
    "Math/Pairwise_Coprime_Progression.cpp": (
        ("Math/Big_Integer.cpp", "多倍長整数"),
    ),
    "Math/Weighted_Taylor_Shifts.cpp": (
        ("Math/Polynomial_Moments.cpp", "重み付き冪和"),
    ),
    "Math/Ferrers_Rook.cpp": (
        ("Math/Newton_Basis.cpp", "Newton基底変換"),
    ),
    "String/Lexicographic_Concatenation.cpp": (
        ("String/Substring_Comparison.cpp", "部分文字列比較"),
    ),
    "Math/Xor_Convolution_Without_Division.cpp": (
        ("Math/Zeta/Bitwise_Convolution.cpp", "集合の畳み込み"),
    ),
    "DataStructure/Weighted_L1_Statistics.cpp": (
        ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
        ("Math/Fraction.cpp", "既約分数と固定幅整数"),
    ),
    "Math/Exponential_Coefficient_Transform.cpp": (
        ("Math/FPS_Composition.cpp", "形式的冪級数の合成"),
    ),
    "Math/Polynomial_Moments.cpp": (
        ("Math/Polynomial.cpp", "多項式・形式的冪級数"),
    ),
    "Math/Krawtchouk_Transform.cpp": (
        ("Math/Polynomial.cpp", "Taylor Shiftと多項式演算"),
    ),
    "Math/Polynomial_Differential_Operator.cpp": (
        ("Math/FPS_Composition.cpp", "形式的冪級数の合成"),
    ),
    "String/Pattern_Avoidance.cpp": (
        ("String/Aho_Corasick.cpp", "Aho–Corasick"),
        ("Math/Linear_Recurrence.cpp", "線形漸化式"),
    ),
    "Algorithm/Multiset_Permutation.cpp": (
        ("Math/Big_Integer.cpp", "多倍長整数"),
        ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    ),
    "DataStructure/Range_Add_Range_Gcd.cpp": (
        ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
        ("DataStructure/Segment_Tree.cpp", "セグメント木"),
    ),
    "Algorithm/Adjacent_Swap_Parity.cpp": (
        ("Algorithm/Adjacent_Swap_Distance.cpp", "隣接交換距離"),
        ("DataStructure/Fenwick_Tree.cpp", "Fenwick木"),
    ),
    "Math/Tridiagonal_Matrix_Power.cpp": (
        ("Math/Consecutive_Linear_Recurrence.cpp", "連続線形漸化式"),
    ),
    "Math/Polynomial_Matrix_Product.cpp": (
        ("Math/Consecutive_Polynomial_Evaluation.cpp", "等間隔多項式評価"),
    ),
    "Math/Polynomial_Recurrence.cpp": (
        ("Math/Polynomial_Matrix_Product.cpp", "多項式行列積"),
    ),
    "Math/Polynomial_Recurrence_Fit.cpp": (
        ("Math/Polynomial_Recurrence.cpp", "多項式係数の漸化式"),
    ),
    "DataStructure/Union_Find_Order_Statistics.cpp": (
        ("DataStructure/union_find.cpp", "UnionFind"),
    ),
    "Math/Cyclic_Multivariate_Convolution.cpp": (
        ("Math/Convolution.cpp", "CRT畳み込み"),
        ("Math/Primitive_Root.cpp", "原始根"),
    ),
}


def render(root: Path) -> dict[Path, bytes]:
    output = {}
    for target, (dependency, title) in BUNDLES.items():
        parts = (root / target).with_suffix("") / "components"
        header = (parts / "header.inc").read_text().rstrip()
        includes, bodies = [], []
        for path in (root / dependency, parts / "body.inc"):
            source = unwrap_source(path.read_text())
            include_start = source.index("#include ")
            guard_start = source.index("#ifndef SIRKIT_", include_start)
            includes.extend(source[include_start:guard_start].strip().splitlines())
            bodies.append(source[guard_start:].rstrip())
            if path == root / dependency:
                overview = source[:include_start].split("// API一覧\n", 1)[1].strip()
                header += f"\n//\n// [同梱の{title}]\n{overview}"
        include_block = "\n".join(dict.fromkeys(includes))
        code = "\n\n".join(bodies)
        output[root / target] = wrap_source(f"{header}\n\n{include_block}\n\n{code}\n").encode()
    for target, dependencies in COMPOSITE_BUNDLES.items():
        parts = (root / target).with_suffix("") / "components"
        header = (parts / "header.inc").read_text().rstrip()
        bodies = []
        for dependency, title in dependencies:
            source = unwrap_source((root / dependency).read_text())
            include_start = source.index("#include ")
            overview = source[:include_start].split("// API一覧\n", 1)[1].strip()
            header += f"\n//\n// [同梱の{title}]\n{overview}"
            bodies.append(source[include_start:].rstrip())
        bodies.append((parts / "body.inc").read_text().rstrip())
        output[root / target] = wrap_source("\n\n".join([header, *bodies]) + "\n").encode()
    return output


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    for path, data in render(root).items():
        if args.check:
            if not path.is_file() or path.read_bytes() != data:
                raise ValueError(f"{path.relative_to(root)}が共通部品と同期していません")
        else:
            path.write_bytes(data)
    print("共通部品の同梱版: " + ("同期確認済み" if args.check else "生成済み"))


if __name__ == "__main__":
    main()
