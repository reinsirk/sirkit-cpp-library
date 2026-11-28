#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""グラフライブラリの単独貼付用ソースへ、原本の共通部品を同梱する。"""

import argparse
from dataclasses import dataclass
from pathlib import Path

if __package__:
    from .bundle_libraries import GUARD, conditional_blocks, directives
    from .public_namespace import unwrap_source
else:
    from bundle_libraries import GUARD, conditional_blocks, directives
    from public_namespace import unwrap_source


@dataclass(frozen=True)
class Component:
    origin: str
    guard: str
    blank_lines_after: int = 1


LCA_COMPONENTS = (
    Component("Graph/LCA.cpp", "LCA_SIZE_HELPERS", 0),
    Component("Graph/LCA.cpp", "LCA_CSR_GRAPH"),
    Component("Graph/LCA.cpp", "LCA_LADDER"),
    Component("Graph/LCA.cpp", "LCA"),
)

BUNDLES = {
    "Affine_Walk_Ranges": (
        Component("Graph/SCC.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/SCC.cpp", "SCC_DETAIL"),
        Component("Graph/SCC.cpp", "SCC_GRAPH"),
        Component("Graph/Walk_Weight_Ranges.cpp", "WALK_WEIGHT_RANGES"),
    ),
    "Bipartite_Coloring": (Component("Graph/Cycle_Detection.cpp", "CYCLE_DETECTION"),),
    "Bipartite_Matching_Analysis": (
        Component("Graph/SCC.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/SCC.cpp", "SCC_DETAIL"),
        Component("Graph/SCC.cpp", "SCC_GRAPH"),
        Component("Graph/Bipartite_Matching.cpp", "BIPARTITE_MATCHING"),
    ),
    "Block_Cut_Forest": (
        Component("Graph/Low_Link.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/Low_Link.cpp", "UNDIRECTED_LOW_LINK"),
    ),
    "Bottleneck_Pair_Histogram": (Component("DataStructure/union_find.cpp", "UNION_FIND"),),
    "Chinese_Postman_Route": (Component("Graph/Eulerian_Trail.cpp", "EULERIAN_TRAIL"),),
    "Chromatic_Polynomial": (Component("Math/Set_Power_Series.cpp", "SET_POWER_SERIES"),),
    "DAG_Grundy": (
        Component("Graph/Topological_sort.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/Topological_sort.cpp", "TOPOLOGICAL_RESULT"),
        Component("Graph/Topological_sort.cpp", "TOPOLOGICAL_READY_MASK"),
        Component("Graph/Topological_sort.cpp", "TOPOLOGICAL_CORE"),
        Component("Graph/Topological_sort.cpp", "TOPOLOGICAL_SORT"),
    ),
    "Difference_Constraints": (
        Component("Graph/Bellman-Ford.cpp", "INF"),
        Component("Graph/Bellman-Ford.cpp", "GRAPH_TYPES"),
        Component("Graph/Bellman-Ford.cpp", "BELLMAN_FORD"),
    ),
    "Directed_Return_Distances": (
        Component("Graph/SCC.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/SCC.cpp", "SCC_DETAIL"),
        Component("Graph/SCC.cpp", "SCC_GRAPH"),
    ),
    "DSU_On_Tree": (Component("Graph/Subtree_Intervals.cpp", "SUBTREE_INTERVALS"),),
    "Dynamic_Color_Component_Sum": (
        Component("Graph/Heavy_Light_Decomposition.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/Heavy_Light_Decomposition.cpp", "HLD_QUERY_HELPERS"),
        Component("Graph/Heavy_Light_Decomposition.cpp", "HEAVY_LIGHT_VALUES_FORWARD"),
        Component("Graph/Heavy_Light_Decomposition.cpp", "HEAVY_LIGHT_DECOMPOSITION"),
        Component("DataStructure/Fenwick_Tree.cpp", "HLD_FENWICK_ACCESS_FORWARD"),
        Component("DataStructure/Fenwick_Tree.cpp", "FENWICK_STORAGE_TYPE"),
        Component("DataStructure/Fenwick_Tree.cpp", "FENWICK_TREE"),
    ),
    "Dynamic_Steiner_Subtree": LCA_COMPONENTS,
    "Dynamic_Tree_Distances": (
        *LCA_COMPONENTS,
        Component("DataStructure/Fenwick_Tree.cpp", "HLD_FENWICK_ACCESS_FORWARD"),
        Component("DataStructure/Fenwick_Tree.cpp", "FENWICK_STORAGE_TYPE"),
        Component("DataStructure/Fenwick_Tree.cpp", "FENWICK_TREE"),
    ),
    "Eulerian_Trail_Decomposition": (Component("Graph/Eulerian_Trail.cpp", "EULERIAN_TRAIL"),),
    "Grid_Slide_Distances": (Component("Graph/Grid_Stride_Distances.cpp", "GRID_STRIDE_LAYOUT"),),
    "Incident_Label_Spanning_Tree": (Component("DataStructure/union_find.cpp", "UNION_FIND"),),
    "Incremental_APSP": (
        Component("Graph/Floyd-Warshall.cpp", "INF"),
        Component("Graph/Floyd-Warshall.cpp", "FLOYD_WARSHALL"),
    ),
    "Incremental_Bridges": (Component("DataStructure/union_find.cpp", "UNION_FIND"),),
    "Johnson_Shortest_Paths": (Component("Graph/Shortest_Path_Edge_Usage.cpp", "SHORTEST_PATH_DISTANCE"),),
    "K_Shortest_Simple_Paths": (Component("Graph/Shortest_Path_Edge_Usage.cpp", "SHORTEST_PATH_DISTANCE"),),
    "Kruskal_Reconstruction_Tree": (
        *LCA_COMPONENTS,
        Component("Graph/Minimum_Spanning_Tree.cpp", "MINIMUM_SPANNING_TREE"),
        Component("DataStructure/union_find.cpp", "UNION_FIND"),
    ),
    "Lexicographic_Shortest_Path": (Component("Graph/Shortest_Path_Edge_Usage.cpp", "SHORTEST_PATH_DISTANCE"),),
    "Lower_Bound_Flow": (
        Component("Graph/Max_Flow.cpp", "MAX_FLOW_CAPACITY", 0),
        Component("Graph/Max_Flow.cpp", "MAX_FLOW"),
    ),
    "MST_Sensitivity": (Component("Graph/Minimum_Spanning_Tree.cpp", "MINIMUM_SPANNING_TREE"),),
    "Min_Cost_B_Flow": (
        Component("Graph/Max_Flow.cpp", "MAX_FLOW_CAPACITY", 0),
        Component("Graph/Max_Flow.cpp", "MAX_FLOW", 2),
    ),
    "Near_Tree_Distances": (Component("Graph/Undirected_Pseudoforest.cpp", "UNDIRECTED_PSEUDOFOREST"),),
    "Range_Edge_Graph": (Component("Graph/Dijkstra.cpp", "GRAPH_TYPES"),),
    "Reachability_Chain_Cover": (
        Component("Graph/SCC.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/SCC.cpp", "SCC_DETAIL"),
        Component("Graph/SCC.cpp", "SCC_GRAPH"),
        Component("Graph/Max_Flow.cpp", "MAX_FLOW_CAPACITY", 0),
        Component("Graph/Max_Flow.cpp", "MAX_FLOW"),
    ),
    "Shortest_Walk_Counts": (Component("Graph/Shortest_Path_Edge_Usage.cpp", "SHORTEST_PATH_DISTANCE"),),
    "Shortest_Weighted_Cycle": (Component("Graph/Cycle_Detection.cpp", "CYCLE_DETECTION"),),
    "Subtree_Edge_Add_Distances": (
        *LCA_COMPONENTS,
        Component("DataStructure/Fenwick_Tree.cpp", "HLD_FENWICK_ACCESS_FORWARD"),
        Component("DataStructure/Fenwick_Tree.cpp", "FENWICK_STORAGE_TYPE"),
        Component("DataStructure/Fenwick_Tree.cpp", "FENWICK_TREE"),
    ),
    "Tree_Walk_Orientation": (Component("Graph/Linear_LCA.cpp", "LINEAR_LCA"),),
    "Tree_Distance_Layers": LCA_COMPONENTS,
    "Tree_Distance_Sums": (
        Component("Graph/Heavy_Light_Decomposition.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/Heavy_Light_Decomposition.cpp", "HLD_QUERY_HELPERS"),
        Component("Graph/Heavy_Light_Decomposition.cpp", "HEAVY_LIGHT_VALUES_FORWARD"),
        Component("Graph/Heavy_Light_Decomposition.cpp", "HEAVY_LIGHT_DECOMPOSITION"),
        Component("DataStructure/Lazy_Segment_Tree.cpp", "SEGMENT_ARRAY_CAPACITY"),
        Component("DataStructure/Lazy_Segment_Tree.cpp", "LAZY_SEGMENT_TREE"),
    ),
    "Tree_Marked_Diameter": LCA_COMPONENTS,
    "Tree_Mo": (
        *LCA_COMPONENTS,
        Component("Algorithm/Mo.cpp", "MO_ORDERING_HELPERS"),
        Component("Algorithm/Mo.cpp", "MO"),
    ),
    "Tree_Path_Difference": LCA_COMPONENTS,
    "Strong_Connectivity_Augmentation": (
        Component("Graph/SCC.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/SCC.cpp", "SCC_DETAIL"),
        Component("Graph/SCC.cpp", "SCC_GRAPH"),
    ),
    "Superincreasing_Path_Queries": (
        *LCA_COMPONENTS,
        Component("Graph/Minimum_Spanning_Tree.cpp", "MINIMUM_SPANNING_TREE"),
        Component("Graph/MST_Sensitivity.cpp", "MST_SENSITIVITY"),
    ),
    "Tree_Backedge_Absorption": (Component("Graph/Subtree_Intervals.cpp", "SUBTREE_INTERVALS"),),
    "Tree_Edge_Color_Path_Counts": (
        Component("Graph/Centroid_Decomposition.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/Centroid_Decomposition.cpp", "CENTROID_DECOMPOSITION"),
    ),
    "Tree_Nearest_Marked": (
        Component("Graph/Centroid_Decomposition.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/Centroid_Decomposition.cpp", "CENTROID_DECOMPOSITION"),
    ),
    "Two_Nearest_Source_Groups": (Component("Graph/Shortest_Path_Edge_Usage.cpp", "SHORTEST_PATH_DISTANCE"),),
    "Undirected_Xor_Walks": (
        Component("Math/Xor_Basis.cpp", "XOR_BASIS"),
        Component("Algorithm/Pair_Xor_Order.cpp", "PAIR_XOR_ORDER"),
    ),
    "Unrooted_Tree_Isomorphism": (Component("Graph/Rooted_Tree_Isomorphism.cpp", "ROOTED_TREE_ISOMORPHISM"),),
    "Walk_Weight_Congruence": (
        Component("Graph/SCC.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/SCC.cpp", "SCC_DETAIL"),
        Component("Graph/SCC.cpp", "SCC_GRAPH"),
    ),
    "Walk_Weight_Ranges": (
        Component("Graph/SCC.cpp", "UNWEIGHTED_GRAPH"),
        Component("Graph/SCC.cpp", "SCC_DETAIL"),
        Component("Graph/SCC.cpp", "SCC_GRAPH"),
    ),
    "Xor_Walk_Reachability": (Component("Graph/Undirected_Xor_Walks.cpp", "XOR_WALK_EDGE"),),
}


def guard_spans(source: str) -> dict[str, tuple[int, int]]:
    """コメントと文字列を除き、先頭階層の部品ガードの開始・終了位置を返す。"""
    records = directives(source)
    spans = {}
    for frame in conditional_blocks(records):
        first, last = frame["open"], frame["close"]
        opening, closing = records[first], records[last]
        guard = opening["args"]
        if opening["depth"] != 0 or opening["kind"] != "ifndef" or not GUARD.fullmatch(guard):
            continue
        define = records[first + 1]
        if frame["outer_branch"] or define["kind"] != "define" or define["args"] != guard:
            continue
        if guard in spans:
            raise ValueError(f"同じ部品ガードが複数あります: {guard}")
        spans[guard] = opening["start"], closing["end"]
    return spans


def render(root: Path) -> dict[Path, bytes]:
    targets = {}
    origins = {}
    for name, components in BUNDLES.items():
        path = root / "Graph" / name
        directory = path / "components"
        pieces = [(directory / "header.inc").read_bytes().decode()]
        for component in components:
            if component.origin not in origins:
                source = (root / component.origin).read_bytes().decode()
                origins[component.origin] = source, guard_spans(source)
            source, spans = origins[component.origin]
            guard = f"SIRKIT_{component.guard}_DEFINED"
            if guard not in spans:
                raise ValueError(f"{component.origin}に部品ガードがありません: {guard}")
            first, last = spans[guard]
            pieces.append(source[first:last] + "\n" * component.blank_lines_after)
        pieces.append((directory / "body.inc").read_bytes().decode())
        data = "".join(pieces)
        unwrap_source(data)
        targets[path.with_suffix(".cpp")] = data.encode()
    return targets


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    options = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    targets = render(root)
    if options.check:
        mismatches = [str(path.relative_to(root)) for path, data in targets.items()
                      if not path.is_file() or path.read_bytes() != data]
        if mismatches:
            parser.exit(1, "共通部品と同期していないファイル: " + ", ".join(mismatches) + "\n")
    else:
        for path, data in targets.items():
            path.write_bytes(data)
    print("共通部品を含むグラフライブラリ:", "同期確認済み" if options.check else "生成済み", len(targets))


if __name__ == "__main__":
    main()
