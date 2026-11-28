#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""変更に関係する現在版のVerifyを、提出せずC++23で入力例照合する。"""
from __future__ import annotations

import argparse
import json
from pathlib import Path, PurePosixPath
import subprocess
import sys
import tempfile

try:
    from . import package_revised_libraries, prepare_verify
    from .ci_compile_profiles import VERIFY_COMPILE_PROFILES
except ImportError:
    import package_revised_libraries
    import prepare_verify
    from ci_compile_profiles import VERIFY_COMPILE_PROFILES

ROOT = Path(__file__).resolve().parents[1]
LIBRARY_DIRS = {"Algorithm", "DataStructure", "Geometry", "Graph", "Internal", "Math", "String"}
GLOBAL_INPUTS = {
    "Scripts/prepare_verify.py", "Scripts/ci_compile_profiles.py", "Scripts/verify_graph_samples.py", "Scripts/library_layout.json",
    ".github/workflows/verify.yml", "Scripts/ci_verify.py",
}
EMBEDDED_DEPENDENTS = {
    "Geometry/Circle.cpp": {"Geometry/Circle_Tangents.cpp"},
    "Geometry/Euclidean_MST.cpp": {"Geometry/Delaunay_Triangulation.cpp"},
    "Algorithm/LIS.cpp": {"Algorithm/Longest_Bitonic_Subsequence.cpp"},
    "Geometry/Point.cpp": {"Geometry/Convex_Polygon_Locator.cpp"},
    "DataStructure/Offline_Point_Add_Rectangle_Sum.cpp": {"Geometry/Static_Cuboid_Sum.cpp"},
    "DataStructure/Hierarchical_Bitset.cpp": {"DataStructure/Point_Set_Mex.cpp"},
    "DataStructure/Persistent_Array.cpp": {"DataStructure/Persistent_Weighted_Union_Find.cpp"},
    "DataStructure/Lazy_Segment_Tree.cpp": {"Algorithm/Weighted_Interval_Stabbing.cpp", "DataStructure/Binary_Range_Statistics.cpp", "DataStructure/Range_Add_Weighted_Minimum.cpp",
                                           "Geometry/Maximum_Weight_Window.cpp"},
    "Graph/Dijkstra.cpp": {"Graph/Range_Edge_Graph.cpp"},
    "Graph/Cycle_Detection.cpp": {"Graph/Shortest_Weighted_Cycle.cpp", "Graph/Bipartite_Coloring.cpp"},
    "Math/Berlekamp_Massey.cpp": {"Math/Sparse_Matrix_Determinant.cpp"},
    "Math/Pollard_Rho.cpp": {"Math/Primitive_Root.cpp", "Math/Two_Square_Sum.cpp", "Math/Divisibility_Chains.cpp"},
    "Math/Zeta/Sieve.cpp": {"Math/Divisibility_Chains.cpp"},
    "Algorithm/Monge_Optimization.cpp": {"Geometry/All_Furthest_Neighbors.cpp", "Algorithm/Concave_Min_Plus_Convolution.cpp"},
    "Algorithm/Nonadjacent_Selection.cpp": {"Algorithm/Disjoint_Subarray_Max_Sum.cpp"},
    "DataStructure/Link_Cut_Tree.cpp": {"Graph/Incremental_Minimum_Spanning_Forest.cpp"},
    "DataStructure/Fenwick_Tree.cpp": {"Algorithm/Josephus.cpp", "Algorithm/Inversion_Limited_Partitions.cpp", "Algorithm/Subarray_Sum_Counts.cpp", "Geometry/Chord_Crossing_Queries.cpp", "Algorithm/Dominance_Counts.cpp", "Algorithm/Cyclic_Inversion_Statistics.cpp", "DataStructure/Range_Fenwick_Tree.cpp", "Algorithm/Permutation_Order.cpp",
                                      "Geometry/Orthogonal_Polygon_Coverage.cpp", "String/Offline_Prefix_Lcp_Sum.cpp",
                                      "Graph/Dynamic_Color_Component_Sum.cpp", "Algorithm/Adjacent_Swap_Parity.cpp",
                                      "DataStructure/Range_Add_Range_Gcd.cpp", "Algorithm/Multiset_Permutation.cpp",
                                      "DataStructure/Weighted_L1_Statistics.cpp"},
    "Graph/Heavy_Light_Decomposition.cpp": {"Graph/Dynamic_Color_Component_Sum.cpp"},
    "DataStructure/Dynamic_Bitset.cpp": {"Math/Binary_Matrix.cpp"},
    "DataStructure/Hash_map.cpp": {"String/Three_Symbol_Frequency_Counts.cpp", "Algorithm/Subarray_Frequency_Windows.cpp", "Algorithm/Forbidden_Sum_Partitions.cpp", "Algorithm/Target_Sum_Subrectangles.cpp"},
    "DataStructure/Segment_Tree.cpp": {"DataStructure/Monotone_Value_Shift.cpp", "String/Sorted_Substring_Index.cpp", "Algorithm/Bounded_Zigzag_Subsequence.cpp", "DataStructure/Range_Nondecreasing_Subsequence.cpp", "DataStructure/Range_Equal_Runs.cpp", "String/Bracket_Sequence.cpp", "Algorithm/Weighted_Monotone_Matching.cpp", "DataStructure/Range_Add_Range_Gcd.cpp"},
    "Math/Matrix.cpp": {"Math/Gaussian_Elimination.cpp", "Math/Adjugate_Matrix.cpp"},
    "Math/Gaussian_Elimination.cpp": {"Math/Adjugate_Matrix.cpp", "Math/Linear_Matroid_Parity.cpp"},
    "Math/NTT.cpp": {"Math/Convolution.cpp", "Math/Large_Convolution.cpp", "Math/Formal_Power_Series.cpp",
                     "Math/Bivariate_Formal_Power_Series.cpp", "Math/Big_Integer.cpp", "Math/Fast_Factorial.cpp",
                     "Math/Relaxed_Convolution.cpp", "Math/Mixed_Radix.cpp", "Math/Convolution_Wrap64.cpp",
                     "Math/Stirling_Columns.cpp", "Math/Bounded_Nondecreasing_Sequences.cpp", "Math/Polynomial_GCD.cpp", "Math/Polynomial_Roots.cpp",
                     "Math/FPS_Composition.cpp", "Math/Polynomial.cpp", "Math/Linear_Recurrence.cpp", "Math/Combinatorial_Sequences.cpp",
                     "String/Wildcard_Pattern_Matching.cpp", "String/Hamming_Distance_Matching.cpp",
                     "Math/Polynomial_Prefix_Sum.cpp",
                     "Graph/Tree_Distance_Frequency.cpp", "Math/Multivariate_Convolution.cpp",
                     "Math/Multiplicative_Convolution.cpp", "Math/Consecutive_Polynomial_Evaluation.cpp",
                     "Math/Polynomial_Sequence_Product.cpp", "Math/Geometric_Polynomial.cpp",
                     "Math/Newton_Basis.cpp", "Math/Consecutive_Linear_Recurrence.cpp"},
    "Graph/SCC.cpp": {"Graph/Incremental_SCC.cpp"},
    "Graph/Max_Flow.cpp": {"Graph/Min_Cost_B_Flow.cpp", "Graph/Lower_Bound_Flow.cpp"},
    "Graph/Minimum_Spanning_Tree.cpp": {"Graph/MST_Sensitivity.cpp", "Graph/Kruskal_Reconstruction_Tree.cpp"},
    "DataStructure/implicit_treap.cpp": {"Algorithm/Interval_LIS.cpp", "Algorithm/Optimal_Adjacent_Merge.cpp"},
    "Math/Set_Power_Series.cpp": {"Graph/Chromatic_Polynomial.cpp", "Math/Set_Series_Composition.cpp"},
    "Math/Big_Integer.cpp": {"Math/Mixed_Radix.cpp", "Math/Pairwise_Coprime_Progression.cpp",
                            "Algorithm/Multiset_Permutation.cpp"},
    "Math/Convolution.cpp": {"Math/Cyclic_Multivariate_Convolution.cpp", "Math/Permutation_Comparison_Count.cpp"},
    "Math/Zeta/Bitwise_Convolution.cpp": {"Math/Xor_Convolution_Without_Division.cpp"},
    "Math/Floor_Sum.cpp": {"Math/Modular_Shift_Minimum.cpp", "Math/Modular_Shift_Mex.cpp"},
    "Math/Fraction.cpp": {"Geometry/L1_Origin_Line_Fit.cpp", "Geometry/Integer_Geometry_3D.cpp",
                          "Geometry/Delaunay_Triangulation.cpp",
                          "DataStructure/Weighted_L1_Statistics.cpp"},
    "Math/Consecutive_Linear_Recurrence.cpp": {"Math/Tridiagonal_Matrix_Power.cpp"},
    "Math/Consecutive_Polynomial_Evaluation.cpp": {"Math/Fast_Factorial.cpp", "Math/Polynomial_Matrix_Product.cpp"},
    "Math/Polynomial_Matrix_Product.cpp": {"Math/Polynomial_Recurrence.cpp"},
    "Math/Polynomial_Recurrence.cpp": {"Math/Polynomial_Recurrence_Fit.cpp"},
    "Math/Polynomial.cpp": {"Math/Polynomial_Moments.cpp", "Math/Krawtchouk_Transform.cpp"},
    "Math/Polynomial_Moments.cpp": {"Math/Weighted_Taylor_Shifts.cpp"},
    "Math/Newton_Basis.cpp": {"Math/Ferrers_Rook.cpp"},
    "Math/FPS_Composition.cpp": {"Math/Exponential_Coefficient_Transform.cpp", "Math/Polynomial_Differential_Operator.cpp"},
    "Math/Characteristic_Polynomial.cpp": {"Math/Matrix_Pencil_Determinant.cpp"},
    "Math/Primitive_Root.cpp": {"Math/Cyclic_Multivariate_Convolution.cpp"},
    "String/Suffix_Array.cpp": {"String/Substring_Index.cpp", "String/Multi_String_Longest_Common_Substring.cpp",
                                "String/Suffix_Tree.cpp", "String/Substring_Comparison.cpp", "String/Cyclic_Rotation_Order.cpp",
                                "String/Lexicographic_String_Shuffle.cpp"},
    "String/Substring_Comparison.cpp": {"String/Lexicographic_Concatenation.cpp"},
    "DataStructure/Static_Range_Minimum.cpp": {"String/Substring_Comparison.cpp"},
    "String/Aho_Corasick.cpp": {"String/Dictionary_Segmenter.cpp", "String/Pattern_Avoidance.cpp"},
    "Math/Linear_Recurrence.cpp": {"String/Pattern_Avoidance.cpp"},
    "String/Prefix_Function.cpp": {"String/Pattern_Range_Occurrences.cpp", "String/Periodic_Pattern_Runs.cpp", "String/Pattern_Erasure.cpp", "String/String_Periods.cpp", "String/Shortest_Common_Superstring.cpp"},
    "String/Z_Algorithm.cpp": {"String/Cyclic_Xor_Matches.cpp", "String/Run_Length_Z.cpp"},
    "DataStructure/union_find.cpp": {"Graph/Incremental_SCC.cpp", "Graph/Kruskal_Reconstruction_Tree.cpp",
                                      "DataStructure/Union_Find_Order_Statistics.cpp", "Graph/LCM_Spanning_Tree.cpp",
                                      "DataStructure/Successor_Set.cpp", "Graph/Hub_Sparse_Distances.cpp"},
    "Graph/Centroid_Decomposition.cpp": {"Graph/Tree_Distance_Frequency.cpp", "Graph/Tree_Nearest_Marked.cpp"},
    "Graph/Bellman-Ford.cpp": {"Graph/Difference_Constraints.cpp"},
    "Graph/Linear_LCA.cpp": {"Graph/Tree_Walk_Orientation.cpp"},
    "Graph/LCA.cpp": {"Graph/Dynamic_Steiner_Subtree.cpp", "Graph/Tree_Mo.cpp",
                      "Graph/Kruskal_Reconstruction_Tree.cpp", "Graph/Tree_Marked_Diameter.cpp"},
    "Algorithm/Mo.cpp": {"Graph/Tree_Mo.cpp"},
    "Algorithm/Range_Inversions.cpp": {"Algorithm/Adjacent_Swap_Distance.cpp"},
    "Algorithm/Adjacent_Swap_Distance.cpp": {"Algorithm/Adjacent_Swap_Parity.cpp"},
    "Graph/Low_Link.cpp": {"Graph/Block_Cut_Forest.cpp"},
    "Graph/Undirected_Pseudoforest.cpp": {"Graph/Near_Tree_Distances.cpp"},
    "Graph/Grid_Stride_Distances.cpp": {"Graph/Grid_Slide_Distances.cpp"},
    "Math/Xor_Basis.cpp": {"Algorithm/Pair_Xor_Order.cpp", "Graph/Undirected_Xor_Walks.cpp"},
    "Algorithm/Pair_Xor_Order.cpp": {"Graph/Undirected_Xor_Walks.cpp"},
    "Graph/Undirected_Xor_Walks.cpp": {"Graph/Xor_Walk_Reachability.cpp"},
    "Graph/Xor_Walk_Reachability.cpp": {"Graph/Undirected_Xor_Walks.cpp"},
    "Math/Formal_Power_Series.cpp": {"Math/Polynomial.cpp", "Math/Combinatorial_Sequences.cpp",
                                     "Math/Labeled_Tree_Matching_Counts.cpp", "Graph/Tournament_Strong_Counts.cpp",
                                     "Math/Polynomial_Prefix_Sum.cpp", "Math/Newton_Basis.cpp",
                                     "Math/Stirling_Columns.cpp", "Math/Polynomial_GCD.cpp", "Math/Polynomial_Roots.cpp",
                                     "Math/FPS_Composition.cpp"},
    "Math/Polynomial_GCD.cpp": {"Math/Polynomial_Roots.cpp"},
}


def git_changed_paths(base: str, root: Path = ROOT) -> list[str]:
    """比較元をcommitへ解決し、未コミット・新規ファイルも含める。"""
    revision = subprocess.run(
        ["git", "rev-parse", "--verify", "--end-of-options", base + "^{commit}"],
        cwd=root, check=True, capture_output=True, text=True).stdout.strip()
    changed = subprocess.run(
        ["git", "diff", "--name-only", "--no-renames", "-z", revision, "--"],
        cwd=root, check=True, capture_output=True).stdout
    untracked = subprocess.run(
        ["git", "ls-files", "--others", "--exclude-standard", "-z"],
        cwd=root, check=True, capture_output=True).stdout
    return sorted({p.decode("utf-8", "surrogateescape") for p in (changed + untracked).split(b"\0") if p})


def dependencies(entry: dict, root: Path = ROOT) -> set[str]:
    result = set(entry["libraries"]) | {entry["driver"]}
    result.add("Verify/current/submissions/" + entry["id"] + ".cpp")
    pending = [entry["driver"]]
    while pending:
        path = pending.pop()
        source = (root / path).read_text()
        for match in prepare_verify.COMMON_INCLUDE.finditer(source):
            name = match[1]
            if name not in result:
                result.add(name)
                pending.append(name)
    return result


def select_entries(entries: list[dict], changed: list[str], *, requested: list[str] | None = None,
                   all_entries: bool = False, root: Path = ROOT) -> dict:
    ids = {entry["id"] for entry in entries}
    if len(ids) != len(entries):
        raise ValueError("Verify IDが重複しています")
    if requested:
        unknown = set(requested) - ids
        if unknown:
            raise ValueError("存在しないVerify ID: " + ", ".join(sorted(unknown)))
    paths = set(changed)
    reasons: dict[str, set[str]] = {identifier: set() for identifier in ids}
    if all_entries or requested:
        for identifier in (ids if all_entries else set(requested)):
            reasons[identifier].add("--all" if all_entries else "--id")
    else:
        universal = {p for p in paths if p in GLOBAL_INPUTS or
                     p.startswith("Verify/current/vendor/") or
                     (p.startswith("Scripts/generate_") and p.endswith(".py"))}
        if universal:
            for reason in reasons.values():
                reason.update(universal)
        expanded = set(paths)
        for path in paths:
            parts = PurePosixPath(path).parts
            if len(parts) == 4 and parts[0] in LIBRARY_DIRS and parts[2] == "components":
                expanded.add(f"{parts[0]}/{parts[1]}.cpp")
        pending = list(expanded)
        while pending:
            consumers = EMBEDDED_DEPENDENTS.get(pending.pop(), set()) - expanded
            expanded.update(consumers)
            pending.extend(consumers)
        # 分割部品の参照が複数の単独版へ波及するため、同じ用途群を保守的に確認する。
        math_parts = {p for p in paths if p.startswith("Math/") and "/components/" in p}
        geometry_parts = {p for p in paths if p.startswith("Geometry/components/")}
        group_paths = {p for p in paths if p.startswith("Verify/current/groups/") and p.endswith(".json")}
        current_groups = {"Verify/current/groups/" + e["group"] + ".json" for e in entries}
        deleted_groups = group_paths - current_groups
        if deleted_groups:
            for reason in reasons.values():
                reason.update(deleted_groups)
        for entry in entries:
            relevant = dependencies(entry, root) & expanded
            if math_parts and any(p.startswith("Math/") for p in entry["libraries"]):
                relevant.update(math_parts)
            if geometry_parts and any(p.startswith("Geometry/") for p in entry["libraries"]):
                relevant.update(geometry_parts)
            group = "Verify/current/groups/" + entry["group"] + ".json"
            if group in group_paths:
                relevant.add(group)
            reasons[entry["id"]].update(relevant)
        covered = {name for entry in entries for name in entry["libraries"]}
        for path in paths:
            name = PurePosixPath(path)
            if name.parts and name.parts[0] in LIBRARY_DIRS and name.suffix == ".cpp" and path not in covered:
                # 削除済みのライブラリもあり得る。新規・現存の未割当は見逃さない。
                if (root / path).exists():
                    raise ValueError("Verifyへ未割当のライブラリ: " + path)
                for reason in reasons.values():
                    reason.add(path)
    selected = sorted(identifier for identifier, why in reasons.items() if why)
    return {"format": 1, "changed_paths": sorted(paths), "selected_ids": selected,
            "reasons": {identifier: sorted(reasons[identifier]) for identifier in selected},
            "coverage": "local_samples_checked_and_skip", "online_status": "not_evaluated"}


def write_report(path: Path, report: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    temporary.replace(path)


def run_selected(entries: list[dict], plan: dict, compiler: str, fallback: bool, output: Path,
                 *, compile_profile: str = "standard", jobs: int = 1, qemu: str | None = None,
                 compiler_args: tuple[str, ...] = ()) -> None:
    selected_ids = set(plan["selected_ids"])
    selected = [entry for entry in entries if entry["id"] in selected_ids]
    if len(selected) != len(selected_ids):
        raise ValueError("選択後にVerify IDが変化しました")
    report = {"format": 1, "plan": plan, "compiler": Path(compiler).name, "standard_fallback": fallback,
              "compile_profile": compile_profile, "jobs": jobs,
              "online_status": "not_evaluated", "status": "running", "results": []}
    if qemu:
        report["qemu"] = Path(qemu).name
    write_report(output, report)
    try:
        if selected:
            with tempfile.TemporaryDirectory(prefix="sirkit_ci_results_") as directory:
                raw = Path(directory) / "samples.json"
                try:
                    prepare_verify.run_tests(selected, compiler, fallback, raw,
                                             compile_profile=compile_profile, jobs=jobs, qemu=qemu,
                                             compiler_args=compiler_args)
                finally:
                    if raw.is_file():
                        report["results"] = json.loads(raw.read_text())["results"]
            actual = {(r["id"], r["skip_runtime_checks"]) for r in report["results"]}
            expected = {(identifier, skip) for identifier in selected_ids for skip in (False, True)}
            if actual != expected or len(report["results"]) != len(expected) or not all(
                    row.get("passed") and row.get("compile_profile", "standard") == compile_profile
                    for row in report["results"]):
                raise ValueError("Verify結果が選択対象・検査設定・コンパイル設定と一致しません")
        report["status"] = "PASS"
    except (Exception, SystemExit) as error:
        report["status"] = "FAIL"
        report["error"] = prepare_verify.relative_report_text(str(error))
        raise
    finally:
        write_report(output, report)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", default="origin/main", help="差分の比較元commit/ref")
    parser.add_argument("--id", action="append", help="Verify IDを明示指定する。複数指定可")
    parser.add_argument("--all", action="store_true", help="全件を確認する")
    parser.add_argument("--list", action="store_true", help="選択計画のJSONだけを出力する")
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--compiler-arg", action="append", default=[], help="コンパイラへの追加引数")
    parser.add_argument("--compile-profile", choices=VERIFY_COMPILE_PROFILES, default="standard")
    parser.add_argument("--qemu", help="指定したLinuxユーザーモードQEMUで静的リンクしたVerifyを実行する")
    parser.add_argument("--fallback", action="store_true")
    parser.add_argument("--jobs", type=int, default=1, help="同時にビルド・実行するVerifyの数")
    parser.add_argument("--skip-sync", action="store_true", help="既に照合済みの場合だけ同期確認を省く")
    parser.add_argument("--results", type=Path)
    args = parser.parse_args(argv)
    if args.all and args.id:
        parser.error("--allと--idは同時に指定できません")
    if args.jobs < 1:
        parser.error("jobsは正の整数で指定してください")
    if args.qemu and args.compile_profile != "standard":
        parser.error("--qemuにはstandardのコンパイル設定を指定してください")
    try:
        changed = [] if args.all or args.id else git_changed_paths(args.base)
        entries = prepare_verify.collect()
        plan = select_entries(entries, changed, requested=args.id, all_entries=args.all)
        if args.list:
            print(json.dumps(plan, ensure_ascii=False, indent=2))
            return 0
        if not args.skip_sync:
            package_revised_libraries.check_generated(ROOT)
        # render元と記録が同期していることを、テスト直前にも照合する。
        entries = prepare_verify.prepare(check=True)
        suffix = "" if args.compile_profile == "standard" else "-" + args.compile_profile
        output = args.results or Path(tempfile.gettempdir()) / "sirkit-ci-verify" / (Path(args.compiler).name + suffix + ".json")
        run_selected(entries, plan, args.compiler, args.fallback, output,
                     compile_profile=args.compile_profile, jobs=args.jobs, qemu=args.qemu,
                     compiler_args=tuple(args.compiler_arg))
        print(f"{len(plan['selected_ids'])}件のローカルVerify: PASS。結果: {output}")
        return 0
    except (OSError, ValueError, subprocess.SubprocessError, SystemExit) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
