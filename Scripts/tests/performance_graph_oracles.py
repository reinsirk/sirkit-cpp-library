# SPDX-License-Identifier: CC0-1.0
"""Independent small-input references for graph performance workloads."""
from __future__ import annotations

from collections import Counter, deque
from functools import cache
from heapq import heappop, heappush
from itertools import combinations, permutations, product
from math import gcd, hypot


def _tree(n, edges):
    adjacency = [[] for _ in range(n)]
    assert len(edges) == n - 1
    for u, v in edges:
        assert 1 <= u <= n and 1 <= v <= n and u != v
        adjacency[u - 1].append(v - 1)
        adjacency[v - 1].append(u - 1)
    return adjacency


def _distances(adjacency, root):
    distance = [-1] * len(adjacency)
    distance[root] = 0
    pending = deque([root])
    while pending:
        u = pending.popleft()
        for v in adjacency[u]:
            if distance[v] == -1:
                distance[v] = distance[u] + 1
                pending.append(v)
    assert min(distance) >= 0
    return distance


def _distance_product(rows):
    n, = rows[0]
    first = _tree(n, rows[1:n])
    second = _tree(n, rows[n:])
    total = 0
    for u in range(n):
        a = _distances(first, u)
        b = _distances(second, u)
        total += sum(da * db for da, db in zip(a, b))
    return [total % (1 << 64)]


def _block_cut_forest(rows):
    n, m = rows[0]
    a, b, c = rows[1]
    assert len(rows) == 2 + m and len({a, b, c}) == 3
    adjacency = [[] for _ in range(n + 1)]
    for u, v in rows[2:]:
        assert 1 <= u < v <= n and v not in adjacency[u]
        adjacency[u].append(v)
        adjacency[v].append(u)
    pending = [(a, {a})]
    while pending:
        vertex, visited = pending.pop()
        if vertex == c:
            if b in visited:
                return ["Yes"]
            continue
        for neighbor in adjacency[vertex]:
            if neighbor not in visited:
                pending.append((neighbor, visited | {neighbor}))
    return ["No"]


def _complement_shortest_paths(rows):
    n, m = rows[0]
    assert m == len(rows) - 1
    blocked = set()
    for u, v in rows[1:]:
        assert 1 <= u < v <= n and (u - 1, v - 1) not in blocked
        blocked.add((u - 1, v - 1))
        blocked.add((v - 1, u - 1))
    distance = [0] + [None] * (n - 1)
    ways = [1] + [0] * (n - 1)
    pending = deque([0])
    while pending:
        u = pending.popleft()
        for v in range(n):
            if u == v or (u, v) in blocked:
                continue
            new_distance = distance[u] + 1
            if distance[v] is None:
                distance[v] = new_distance
                pending.append(v)
            if distance[v] == new_distance:
                ways[v] += ways[u]
    return [-1 if distance[-1] is None else ways[-1] % 998244353]


def _dag_reachability(rows):
    n, m, q = rows[0]
    assert len(rows) == 1 + m + q
    adjacency = [[] for _ in range(n + 1)]
    for u, v in rows[1:m + 1]:
        assert 1 <= u < v <= n
        adjacency[u].append(v)
    result = []
    for u, v in rows[m + 1:]:
        assert 1 <= u < v <= n
        seen = {u}
        pending = deque([u])
        while pending:
            for w in adjacency[pending.popleft()]:
                if w not in seen:
                    seen.add(w)
                    pending.append(w)
        result.append("Yes" if v in seen else "No")
    return result


def _weighted_distances(adjacency, root):
    distance = {root: 0}
    pending = deque([root])
    while pending:
        v = pending.popleft()
        for u, weight in adjacency[v]:
            if u not in distance:
                distance[u] = distance[v] + weight
                pending.append(u)
    return distance


def _incremental_weighted_forest(rows):
    n, x, q = rows[0]
    assert len(rows) == q + 1
    adjacency = [[] for _ in range(n)]
    answers = []
    for kind, v, *extra in rows[1:]:
        assert 0 <= v < n
        if kind == 1:
            weight, = extra
            assert 0 <= weight <= 10**9 and v not in _weighted_distances(adjacency, x)
            adjacency[v].append((x, weight))
            adjacency[x].append((v, weight))
        elif kind == 2:
            u, = extra
            distance = _weighted_distances(adjacency, v).get(u)
            answers.append(-1 if distance is None else distance)
            if distance is not None:
                x = (x + distance) % n
        elif kind == 3:
            component = _weighted_distances(adjacency, v)
            answers.append(max(max(_weighted_distances(adjacency, u).values()) for u in component))
        else:
            assert kind == 4
            x = (x + v) % n
    return answers


def _mst_sensitivity(rows):
    n, m = rows[0]
    assert len(rows) == m + 1
    edges = rows[1:]
    assert len({(min(a, b), max(a, b)) for a, b, _ in edges}) == m
    assert all(1 <= a <= n and 1 <= b <= n and a != b and 1 <= w <= 10**9 for a, b, w in edges)
    indexed = sorted(range(m), key=lambda i: edges[i][2])
    worst = 0
    for skip in range(m):
        labels = list(range(n))
        groups, cost = n, 0
        for i in indexed:
            if i == skip:
                continue
            a, b, weight = edges[i]
            before, after = labels[a - 1], labels[b - 1]
            if before != after:
                labels = [after if label == before else label for label in labels]
                cost += weight
                groups -= 1
                if groups == 1:
                    break
        assert groups == 1
        worst = max(worst, cost)
    return [worst]


def _range_edge(rows):
    n, m = rows[0]
    coordinates = rows[1]
    assert n == len(coordinates) and m == len(rows) - 2
    assert all(a < b for a, b in zip(coordinates, coordinates[1:]))
    adjacency = [[] for _ in range(n)]
    for left, right, first, last, fare in rows[2:]:
        assert 1 <= left <= right <= n and 1 <= first <= last <= n
        assert right < first or last < left
        for a in range(left - 1, right):
            for b in range(first - 1, last):
                adjacency[a].append((b, fare + abs(coordinates[a] - coordinates[b])))
    infinity = float("inf")
    distance = [0] + [infinity] * (n - 1)
    pending = [(0, 0)]
    while pending:
        total, u = heappop(pending)
        if total != distance[u]:
            continue
        for v, weight in adjacency[u]:
            candidate = total + weight
            if candidate < distance[v]:
                distance[v] = candidate
                heappush(pending, (candidate, v))
    return [d if d != infinity else -1 for d in distance[1:]]


def _nearest_marked(rows):
    n, = rows[0]
    adjacency = _tree(n, rows[1:n])
    q, = rows[n]
    assert q == len(rows) - n - 1
    marked = set()
    answers = []
    for kind, v in rows[n + 1:]:
        v -= 1
        if kind == 1:
            marked.symmetric_difference_update({v})
        else:
            assert kind == 2
            distance = _distances(adjacency, v)
            answers.append(min(distance[w] for w in marked))
    return answers


def _tree_mo(rows):
    n, q = rows[0]
    labels = rows[1]
    adjacency = _tree(n, rows[2:n + 1])
    assert n == len(labels) and q == len(rows) - n - 1
    answers = []
    for u, v, low, high in rows[n + 1:]:
        assert 1 <= u < v <= n and 1 <= low <= high <= n
        start = u - 1
        parent = {start: None}
        pending = deque([start])
        while pending:
            vertex = pending.popleft()
            for neighbor in adjacency[vertex]:
                if neighbor not in parent:
                    parent[neighbor] = vertex
                    pending.append(neighbor)
        counts = Counter()
        vertex = v - 1
        while vertex is not None:
            counts[labels[vertex]] += 1
            vertex = parent[vertex]
        answers.append(sum(low <= frequency <= high for frequency in counts.values()))
    return answers


def _subtree(adjacency, root, vertex):
    parent = {root: None}
    visited = [root]
    for u in visited:
        for v in adjacency[u]:
            if v not in parent:
                parent[v] = u
                visited.append(v)
    assert len(parent) == len(adjacency)
    subtree = [vertex]
    for u in subtree:
        subtree.extend(v for v in adjacency[u] if parent[v] == u)
    return set(subtree)


def _prune(adjacency, terminals):
    remaining = [set(neighbors) for neighbors in adjacency]
    alive = set(range(len(adjacency)))
    pending = deque(v for v in alive - terminals if len(remaining[v]) <= 1)
    while pending:
        leaf = pending.popleft()
        if leaf not in alive:
            continue
        alive.remove(leaf)
        for u in remaining[leaf]:
            remaining[u].remove(leaf)
            if len(remaining[u]) <= 1 and u not in terminals:
                pending.append(u)
        remaining[leaf].clear()
    return len(alive)


def _dynamic_steiner(rows):
    n, = rows[0]
    adjacency = _tree(n, rows[1:n])
    initial = rows[n]
    assert len(initial) == n and set(initial) <= {0, 1}
    marked = {v for v, color in enumerate(initial) if color == 1}
    q, = rows[n + 1]
    assert q == len(rows) - n - 2
    answers = []
    for operation in rows[n + 2:]:
        kind, *args = operation
        if kind == 1:
            v, = args
            marked.symmetric_difference_update({v - 1})
        else:
            assert kind == 2
            root, vertex = args
            terminals = marked & _subtree(adjacency, root - 1, vertex - 1)
            answers.append(_prune(adjacency, terminals))
    return answers


def _functional_graph_fold(rows):
    (n, k), values = rows
    assert len(values) == n and 1 <= k <= 10**12
    candies = 0
    for _ in range(k):
        candies += values[candies % n]
    return [candies]


def _dynamic_color_component_sum(rows):
    n, q = rows[0]
    weights, colors = rows[1], rows[2]
    assert 1 <= n <= 300000 and 1 <= q <= 200000
    assert len(weights) == len(colors) == n and len(rows) == n + 2 + q
    assert all(1 <= value <= 10**9 for value in weights) and set(colors) <= {0, 1}
    graph = [[] for _ in range(n)]
    for a, b in rows[3:n + 2]:
        assert 1 <= a < b <= n
        graph[a - 1].append(b - 1)
        graph[b - 1].append(a - 1)
    answers = []
    for kind, index, *args in rows[n + 2:]:
        assert 1 <= index <= n
        v = index - 1
        if kind == 1:
            assert not args
            colors[v] ^= 1
        elif kind == 2:
            delta, = args
            assert 1 <= delta <= 10**9
            weights[v] += delta
        else:
            assert kind == 3 and not args
            pending = [v]
            component = {v}
            for u in pending:
                for w in graph[u]:
                    if w not in component and colors[w] == colors[v]:
                        pending.append(w)
                        component.add(w)
            answers.append(sum(weights[u] for u in component))
    return answers


def _static_top_tree(rows):
    modulus = 998244353
    tokens = iter(value for row in rows for value in row)
    n, q = next(tokens), next(tokens)
    children = [[] for _ in range(n)]
    for child in range(1, n):
        children[next(tokens) - 1].append(child)
    values = [next(tokens) for _ in range(n)]
    answers = []
    for _ in range(q):
        vertex, value = next(tokens) - 1, next(tokens)
        values[vertex] = value
        hashes = values.copy()
        for v in reversed(range(n)):
            if children[v]:
                product = 1
                for child in children[v]:
                    product = product * hashes[child] % modulus
                hashes[v] = (hashes[v] + product) % modulus
        answers.append(hashes[0])
    return answers


def _tree_marked_diameter(rows):
    n, = rows[0]
    adjacency = _tree(n, rows[1:n])
    q, = rows[n]
    assert len(rows) == n + 1 + q
    distances = [_distances(adjacency, u) for u in range(n)]
    marked = set(range(n))
    output = []
    for v, in rows[n + 1:]:
        assert 1 <= v <= n
        marked.symmetric_difference_update([v - 1])
        assert len(marked) >= 2
        output.append(max(distances[u][w] for u in marked for w in marked))
    return output


def independent(plan, data):
    if plan["verify_id"] == "vertex_deletion_components":
        return str(_vertex_deletion_components(data))
    if plan["verify_id"] == "incident_label_spanning_tree":
        return " ".join(map(str, _incident_label_spanning_tree(data)))
    if plan["verify_id"] == "palindromic_shortest_paths":
        return " ".join(map(str, _palindromic_shortest_paths(data)))
    if plan["verify_id"] == "shortest_nonzero_paths":
        return " ".join(map(str, _shortest_nonzero_paths(data)))
    oracle = {
        "automaton_shortest_path_hopscotch": _automaton_shortest_path_hopscotch,
        "subtree_edge_add_distances": _subtree_edge_add_distances,
        "tree_path_difference_road": _tree_path_difference_road,
        "walk_weight_congruence_return": _walk_weight_congruence_return,
        "unit_step_forest_labels": _unit_step_forest_labels,
        "dsu_on_tree_mode": _dsu_on_tree_mode,
        "hypergraph_shortest_paths_merge": _hypergraph_shortest_paths_merge,
        "component_add_union_find": _component_add_union_find,
        "resource_constrained_paths": _resource_constrained_paths,
        "subcubic_tree_biconnectivity": _subcubic_tree_biconnectivity,
        "undirected_walk_lengths": _undirected_walk_lengths,
        "exact_length_walk_bottleneck": _exact_length_walk_bottleneck,
        "exact_length_walk_weighted_count": _exact_length_walk_weighted_count,
        "tree_metric": _tree_metric,
        "binary_tree_traversal": _binary_tree_traversal,
        "shortest_odd_cycle": _shortest_odd_cycle,
        "tree_permutation_distance": _tree_permutation_distance,
        "graph_metric_realization": _graph_metric_realization,
        "bitwise_path": _bitwise_path,
        "tree_distance_sums": _tree_distance_sums,
        "directed_return_distances": _directed_return_distances,
        "tree_heap_orderings": _tree_heap_orderings,
        "difference_constraints": _difference_constraints,
        "tree_walk_orientation": _tree_walk_orientation,
        "subsequence_shortest_paths": _subsequence_shortest_paths,
        "tournament_strong_counts": _tournament_strong_counts,
        "tournament_component_sums": _tournament_component_sums,
        "path_matching_candies": _path_matching_candies,
        "core_decomposition": _core_decomposition,
        "forest_independent_set": _forest_independent_set,
        "forest_edge_coloring": _forest_edge_coloring,
        "forest_subset_components": _forest_subset_components,
        "interval_cover_subsets": _interval_cover_subsets,
        "strong_connectivity_augmentation": _strong_connectivity_augmentation,
        "affine_walk_ranges": _affine_walk_ranges,
        "superincreasing_path_queries": _superincreasing_path_queries,
        "chinese_postman_route": _chinese_postman_route,
        "tree_backedge_absorption": _tree_backedge_absorption,
        "tree_edge_color_path_counts": _tree_edge_color_path_counts,
        "dynamic_edge_products": _dynamic_edge_products,
        "unrooted_tree_leaf_deletion": _unrooted_tree_leaf_deletion,
        "bipartite_coloring": _bipartite_coloring,
        "bipartite_matching_allowed_edges": _bipartite_matching_allowed_edges,
        "block_cut_forest": _block_cut_forest,
        "bottleneck_pair_histogram": _bottleneck_pair_histogram,
        "bridge_block_orientation": _bridge_block_orientation,
        "complete_binary_tree": _complete_binary_tree,
        "complement_shortest_paths": _complement_shortest_paths,
        "dag_critical_path_schedule": _dag_critical_path_schedule,
        "dag_grundy": _dag_grundy,
        "directed_game": _directed_game,
        "dynamic_color_component_sum": _dynamic_color_component_sum,
        "dynamic_tree_distances": _dynamic_tree_distances,
        "edge_removal_distances": _edge_removal_distances,
        "enumerate_topological_orders": _enumerate_topological_orders,
        "equal_tree_path_partition": _equal_tree_path_partition,
        "eulerian_trail_decomposition": _eulerian_trail_decomposition,
        "functional_graph_fold": _functional_graph_fold,
        "grid_slide_distances": _grid_slide_distances,
        "grid_stride_distances": _grid_stride_distances,
        "offline_dag_reachability": _dag_reachability,
        "incremental_weighted_forest": _incremental_weighted_forest,
        "incremental_apsp": _incremental_apsp,
        "incremental_bridges": _incremental_bridges,
        "johnson_shortest_paths": _johnson_shortest_paths,
        "k_shortest_simple_paths": _k_shortest_simple_paths,
        "kruskal_reconstruction_tree": _kruskal_reconstruction_tree,
        "lexicographic_shortest_path": _lexicographic_shortest_path,
        "lower_bound_flow": _lower_bound_flow,
        "mst_sensitivity_delete": _mst_sensitivity,
        "near_tree_distances": _near_tree_distances,
        "neighborhood_assignment": _neighborhood_assignment,
        "parity_orientation": _parity_orientation,
        "virtual_tree_distance_product": _distance_product,
        "range_edge_graph_express": _range_edge,
        "reachability_chain_cover": _reachability_chain_cover,
        "time_dependent_paths_latest": _time_dependent_paths_latest,
        "topological_order_counter": _topological_order_counter,
        "tree_nearest_marked": _nearest_marked,
        "tree_distance_layers": _tree_distance_layers,
        "static_top_tree": _static_top_tree,
        "tree_marked_diameter": _tree_marked_diameter,
        "tree_mo": _tree_mo,
        "tree_path_bitwise_counts_and": _tree_path_bitwise_counts_and,
        "two_nearest_source_groups": _two_nearest_source_groups,
        "undirected_pseudoforest": _undirected_pseudoforest,
        "undirected_xor_walks": _undirected_xor_walks,
        "walk_weight_ranges": _walk_weight_ranges,
        "xor_walk_reachability": _xor_walk_reachability,
        "shortest_path_edge_usage": _shortest_path_edge_usage,
        "shortest_walk_counts": _shortest_walk_counts,
        "shortest_weighted_cycle": _shortest_weighted_cycle,
        "dynamic_steiner_subtree": _dynamic_steiner,
    }[plan["verify_id"]]
    rows = data.splitlines()
    if plan["verify_id"] not in ("directed_game", "grid_slide_distances", "grid_stride_distances", "lower_bound_flow"):
        rows = [list(map(int, line.split())) for line in rows]
    return " ".join(map(str, oracle(rows)))


def _kruskal_reconstruction_tree(rows):
    h, w = rows[0]
    floor = rows[1:h + 1]
    q, = rows[h + 1]
    queries = rows[h + 2:]
    assert len(queries) == q and all(len(r) == w for r in floor)

    def shortest(start, goal):
        pending = [(0, start)]
        visited = set()
        while pending:
            steps, cell = heappop(pending)
            if cell == goal:
                return steps
            if cell in visited:
                continue
            visited.add(cell)
            r, c, height = cell
            for x, y in ((r - 1, c), (r + 1, c), (r, c - 1), (r, c + 1)):
                if 0 <= x < h and 0 <= y < w and height <= floor[x][y]:
                    heappush(pending, (steps, (x, y, height)))
            for level in (height - 1, height + 1):
                if 1 <= level <= floor[r][c]:
                    heappush(pending, (steps + 1, (r, c, level)))
        raise AssertionError("every building connects on its first floor")

    return [shortest((a - 1, b - 1, y), (c - 1, d - 1, z)) for a, b, y, c, d, z in queries]


def _grid_stride_distances(rows):
    h, w, k = map(int, rows[0].split())
    x1, y1, x2, y2 = map(int, rows[1].split())
    grid = rows[2:]
    assert len(grid) == h and all(len(row) == w for row in grid)
    start = x1 - 1, y1 - 1
    goal = x2 - 1, y2 - 1
    assert start != goal and grid[start[0]][start[1]] == grid[goal[0]][goal[1]] == "."
    pending = deque([start])
    distance = {start: 0}
    while pending:
        r, c = pending.popleft()
        if (r, c) == goal:
            return [distance[goal]]
        for dr, dc in ((1, 0), (0, 1), (-1, 0), (0, -1)):
            for step in range(1, k + 1):
                x, y = r + dr * step, c + dc * step
                if not (0 <= x < h and 0 <= y < w) or grid[x][y] == "@":
                    break
                if (x, y) not in distance:
                    distance[x, y] = distance[r, c] + 1
                    pending.append((x, y))
    return [-1]


def _grid_slide_distances(rows):
    n = int(rows[0])
    start = tuple(int(value) - 1 for value in rows[1].split())
    goal = tuple(int(value) - 1 for value in rows[2].split())
    grid = rows[3:]
    assert len(grid) == n and all(len(row) == n for row in grid)
    assert start != goal and grid[start[0]][start[1]] == grid[goal[0]][goal[1]] == "."
    pending = deque([start])
    distance = {start: 0}
    while pending:
        r, c = pending.popleft()
        if (r, c) == goal:
            return [distance[goal]]
        for dr, dc in ((1, 1), (1, -1), (-1, 1), (-1, -1)):
            x, y = r + dr, c + dc
            while 0 <= x < n and 0 <= y < n and grid[x][y] != "#":
                if (x, y) not in distance:
                    distance[x, y] = distance[r, c] + 1
                    pending.append((x, y))
                x += dr
                y += dc
    return [-1]


def _lower_bound_flow(rows):
    h, w = map(int, rows[0].split())
    assert len(rows) == h + 1
    assert all(len(row) == w and set(row) <= {"1", "2", "?"} for row in rows[1:])
    grid = "".join(rows[1:])

    @cache
    def cover(available):
        if available == 0:
            return True
        bit = available & -available
        i = bit.bit_length() - 1
        remaining = available ^ bit
        if grid[i] != "2" and cover(remaining):
            return True
        if grid[i] == "1":
            return False
        neighbors = []
        if i % w != w - 1:
            neighbors.append(i + 1)
        if i // w != h - 1:
            neighbors.append(i + w)
        return any(grid[j] != "1" and remaining & (1 << j) and cover(remaining ^ (1 << j)) for j in neighbors)

    return ["Yes" if cover((1 << (h * w)) - 1) else "No"]


def _edge_removal_distances(rows):
    n, m = rows[0]
    edges = rows[1:]
    assert len(edges) == m and len(set(map(tuple, edges))) == m
    adjacent = [[] for _ in range(n)]
    for i, (u, v) in enumerate(edges):
        assert 1 <= u <= n and 1 <= v <= n and u != v
        adjacent[u - 1].append((v - 1, i))

    def bfs(omitted):
        active = deque([(0, 0)])
        seen = {0}
        while active:
            u, distance = active.popleft()
            if u == n - 1:
                return distance
            for v, i in adjacent[u]:
                if i != omitted and v not in seen:
                    seen.add(v)
                    active.append((v, distance + 1))
        return -1

    return [bfs(i) for i in range(m)]


def _bipartite_coloring(rows):
    (n, m), first, second = rows
    assert len(first) == len(second) == m
    root = list(range(n))
    parity = [0] * n

    def representative(v):
        flipped = 0
        while root[v] != v:
            flipped ^= parity[v]
            v = root[v]
        return v, flipped

    for u, v in zip(first, second):
        assert 1 <= u <= n and 1 <= v <= n
        a, pa = representative(u - 1)
        b, pb = representative(v - 1)
        if a == b:
            if pa == pb:
                return ["No"]
        else:
            root[a] = b
            parity[a] = 1 ^ pa ^ pb
    return ["Yes"]


def _neighborhood_assignment(rows):
    n, m, q = rows[0]
    adjacent = [[] for _ in range(n)]
    edges = rows[1:m + 1]
    assert len(edges) == m and len(set(map(tuple, edges))) == m
    for u, v in edges:
        assert 1 <= u < v <= n
        adjacent[u - 1].append(v - 1)
        adjacent[v - 1].append(u - 1)
    operations = [vertex for row in rows[m + 1:] for vertex in row]
    assert len(operations) == q
    labels = list(range(1, n + 1))
    for u in operations:
        assert 1 <= u <= n
        value = labels[u - 1]
        for v in adjacent[u - 1]:
            labels[v] = value
    return labels


def _shortest_weighted_cycle(rows):
    directed, = rows[0]
    n, m = rows[1]
    edges = rows[2:]
    assert directed in (0, 1) and len(edges) == m
    assert len({tuple(sorted((u, v))) for u, v, _ in edges}) == m
    adjacent = [[] for _ in range(n)]
    for i, (u, v, weight) in enumerate(edges):
        assert 1 <= u <= n and 1 <= v <= n and u != v and 1 <= weight <= 10**9
        adjacent[u - 1].append((v - 1, weight, i))
        if not directed:
            adjacent[v - 1].append((u - 1, weight, i))

    def return_path(source, target, ignored):
        pending = [(0, source)]
        visited = set()
        while pending:
            distance, u = heappop(pending)
            if u == target:
                return distance
            if u not in visited:
                visited.add(u)
                for v, weight, i in adjacent[u]:
                    if i != ignored and v not in visited:
                        heappush(pending, (distance + weight, v))
        return None

    candidates = []
    for i, (u, v, weight) in enumerate(edges):
        path = return_path(v - 1, u - 1, i)
        if path is not None:
            candidates.append(weight + path)
    return [min(candidates, default=-1)]


def _near_tree_distances(rows):
    n, m = rows[0]
    adjacent = [[] for _ in range(n)]
    edges = rows[1:m + 1]
    assert len(edges) == m and len(set(map(tuple, edges))) == m
    for u, v in edges:
        assert 1 <= u < v <= n
        adjacent[u - 1].append(v - 1)
        adjacent[v - 1].append(u - 1)
    q, = rows[m + 1]
    queries = rows[m + 2:]
    assert len(queries) == q
    return [_distances(adjacent, u - 1)[v - 1] for u, v in queries]


def _undirected_pseudoforest(rows):
    n, = rows[0]
    adjacent = [[] for _ in range(n)]
    edges = rows[1:n + 1]
    assert len(edges) == n and len(set(map(tuple, edges))) == n
    for u, v in edges:
        assert 1 <= u < v <= n
        adjacent[u - 1].append(v - 1)
        adjacent[v - 1].append(u - 1)
    q, = rows[n + 1]
    queries = rows[n + 2:]
    assert len(queries) == q

    def paths(u, target, visited):
        if u == target:
            return 1
        visited = visited | {u}
        return sum(paths(v, target, visited) for v in adjacent[u] if v not in visited)

    return ["Yes" if paths(u - 1, v - 1, set()) == 1 else "No" for u, v in queries]


def _shortest_path_edge_usage(rows):
    n, m = rows[0]
    edges = rows[1:]
    assert len(edges) == m and len({(u, v) for u, v, _ in edges}) == m
    adjacent = [[] for _ in range(n)]
    for i, (u, v, w) in enumerate(edges):
        assert 1 <= u < v <= n and 1 <= w <= 10**9
        adjacent[u - 1].append((v - 1, w, i))
        adjacent[v - 1].append((u - 1, w, i))

    def shortest(removed):
        pending = [(0, 0)]
        visited = set()
        while pending:
            distance, u = heappop(pending)
            if u == n - 1:
                return distance
            if u not in visited:
                visited.add(u)
                for v, w, edge in adjacent[u]:
                    if edge != removed and v not in visited:
                        heappush(pending, (distance + w, v))
        return None

    baseline = shortest(-1)
    assert baseline is not None
    return ["Yes" if shortest(i) != baseline else "No" for i in range(m)]


def _xor_values(n, edges, source):
    attainable = [set() for _ in range(n)]
    attainable[source].add(0)
    updated = True
    while updated:
        updated = False
        for u, v, weight in edges:
            extension = {value ^ weight for value in attainable[u]} - attainable[v]
            if extension:
                attainable[v].update(extension)
                updated = True
    return attainable


def _undirected_xor_walks(rows):
    cases, = rows[0]
    assert cases == 1
    n, m, bound = rows[1]
    edges = rows[2:]
    assert len(edges) == m and len({(u, v) for u, v, _ in edges}) == m
    assert all(1 <= u < v <= n and 0 <= w < (1 << 30) for u, v, w in edges)
    arcs = [(u - 1, v - 1, w) for u, v, w in edges]
    arcs += [(v, u, w) for u, v, w in arcs]
    count = 0
    for source in range(n):
        attainable = _xor_values(n, arcs, source)
        assert all(attainable)
        count += sum(min(values) <= bound for values in attainable[source + 1:])
    return [count]


def _xor_walk_reachability(rows):
    n, m = rows[0]
    edges = rows[1:]
    assert len(edges) == m and all(1 <= u <= n and 1 <= v <= n and 0 <= w < 1024 for u, v, w in edges)
    arcs = [(u - 1, v - 1, w) for u, v, w in edges]
    attainable = _xor_values(n, arcs, 0)
    return [min(attainable[-1], default=-1)]


def _walk_weight_ranges(rows):
    n, m = rows[0]
    assert len(rows) == m + 1 and len(set(map(tuple, rows[1:]))) == m
    adjacent = [[] for _ in range(n)]
    for u, v, bit in rows[1:]:
        assert 1 <= u <= n and 1 <= v <= n and bit in (0, 1)
        adjacent[u - 1].append((v - 1, bit))

    # An alternating string is determined by its first digit and its length.
    # Exploring lengths above 2*N is enough: a finite endpoint has a simple
    # witness in the 2*N-state signed-product graph.
    threshold = 2 * n
    longest = [None] * n
    seen = {(0, ())}
    pending = [(0, ())]
    for u, stack in pending:
        if len(stack) > threshold:
            continue
        longest[u] = max(longest[u] or 0, len(stack))
        for v, bit in adjacent[u]:
            reduced = stack[:-1] if stack and stack[-1] == bit else (*stack, bit)
            if (v, reduced) not in seen:
                seen.add((v, reduced))
                pending.append((v, reduced))

    unbounded = {u for u, stack in seen if len(stack) > threshold}
    pending_vertices = list(unbounded)
    for u in pending_vertices:
        for v, _ in adjacent[u]:
            if v not in unbounded:
                unbounded.add(v)
                pending_vertices.append(v)
    assert all(v in unbounded or value is not None for v, value in enumerate(longest))
    return [-1 if v in unbounded else value for v, value in enumerate(longest)]


def _parity_orientation(rows):
    n, m = rows[0]
    edges = rows[1:]
    assert len(edges) == m and len({tuple(sorted(edge)) for edge in edges}) == m
    adjacent = [[] for _ in range(n + 1)]
    for u, v in edges:
        assert 1 <= u <= n and 1 <= v <= n and u != v
        adjacent[u].append(v)
        adjacent[v].append(u)
    color = {1: 0}
    pending = [1]
    for u in pending:
        for v in adjacent[u]:
            if v not in color:
                color[v] = 1 - color[u]
                pending.append(v)
            assert color[u] != color[v]
    assert len(color) == n
    if m % 2:
        return [-1]
    assert all(len(adjacent[u]) % 2 == 0 for u in pending if color[u] == 0)
    return [vertex for u, v in edges for vertex in ((u, v) if color[u] == 0 else (v, u))]


def _time_dependent_paths_latest(rows):
    n, m = rows[0]
    assert len(rows) == m + 1
    trains = []
    for first, interval, count, duration, u, v in rows[1:]:
        assert all(1 <= x <= 10**9 for x in (first, interval, count, duration))
        assert 1 <= u <= n and 1 <= v <= n and u != v
        for i in range(count):
            depart = first + i * interval
            trains.append((depart, depart + duration, u, v))
    latest = [None] * (n + 1)
    latest[n] = max(arrive for _, arrive, _, _ in trains)
    for depart, arrive, u, v in sorted(trains, reverse=True):
        if latest[v] is not None and arrive <= latest[v] and latest[u] is None:
            latest[u] = depart
    return ["Unreachable" if time is None else time for time in latest[1:n]]


def _complete_binary_tree(rows):
    tests, = rows[0]
    assert tests + 1 == len(rows)
    answer = []
    for n, x, k in rows[1:]:
        assert 1 <= x <= n and 0 <= k < n
        neighbors = [[] for _ in range(n + 1)]
        for u in range(2, n + 1):
            neighbors[u].append(u // 2)
            neighbors[u // 2].append(u)
        distance = {x: 0}
        pending = [x]
        for u in pending:
            for v in neighbors[u]:
                if v not in distance:
                    distance[v] = distance[u] + 1
                    pending.append(v)
        answer.append(list(distance.values()).count(k))
    return answer


def _shortest_walk_counts(rows):
    n, m = rows[0]
    assert len(rows) == m + 1 and len(set(map(tuple, rows[1:]))) == m
    neighbors = [set() for _ in range(n)]
    for u, v in rows[1:]:
        assert 1 <= u < v <= n
        neighbors[u - 1].add(v - 1)
        neighbors[v - 1].add(u - 1)

    pending, visited = {0}, {0}
    distance = 0
    while pending and n - 1 not in pending:
        pending = set().union(*(neighbors[u] for u in pending)) - visited
        visited.update(pending)
        distance += 1
    if not pending:
        return [0]
    # Count all walks of exactly the previously determined shortest length.
    counts = [1] + [0] * (n - 1)
    for _ in range(distance):
        counts = [sum(counts[u] for u in adjacent) % (10**9 + 7) for adjacent in neighbors]
    return [counts[-1]]


def _dynamic_tree_distances(rows):
    n, = rows[0]
    adjacent = [[] for _ in range(n + 1)]
    weights = []
    for edge, (u, v, weight) in enumerate(rows[1:n]):
        assert 1 <= u <= n and 1 <= v <= n and 1 <= weight <= 10**9
        adjacent[u].append((v, edge))
        adjacent[v].append((u, edge))
        weights.append(weight)
    q, = rows[n]
    assert len(rows) == n + q + 1
    output = []
    for kind, a, b in rows[n + 1:]:
        if kind == 1:
            assert 1 <= a < n and 1 <= b <= 10**9
            weights[a - 1] = b
        else:
            assert kind == 2 and 1 <= a <= n and 1 <= b <= n
            distance = {a: 0}
            pending = [a]
            for u in pending:
                for v, edge in adjacent[u]:
                    if v not in distance:
                        distance[v] = distance[u] + weights[edge]
                        pending.append(v)
            assert len(distance) == n
            output.append(distance[b])
    assert output
    return output


def _bridge_block_orientation(rows):
    n, m = rows[0]
    edges, reach = rows[1:-1], rows[-1]
    assert len(reach) == n and len(edges) == m == n * (n - 1) // 2
    assert len({tuple(edge) for edge in edges}) == m
    groups = {count: [u for u in range(1, n + 1) if reach[u - 1] == count] for count in reach}
    forward_path = {pair for group in groups.values() for pair in zip(group, group[1:])}
    directed = [[] for _ in range(n + 1)]
    result = []
    for u, v in edges:
        assert 1 <= u < v <= n
        forward = reach[u - 1] > reach[v - 1] or (u, v) in forward_path
        directed[u if forward else v].append(v if forward else u)
        result.append("->" if forward else "<-")
    for start, count in enumerate(reach, 1):
        seen = {start}
        frontier = {start}
        while frontier:
            frontier = {v for u in frontier for v in directed[u]} - seen
            seen.update(frontier)
        assert len(seen) == count
    return result


def _reachability_chain_cover(rows):
    n, m = rows[0]
    assert len(rows) == m + 1 and len(set(map(tuple, rows[1:]))) == m
    outgoing = [set() for _ in range(n + 1)]
    has_incoming = set()
    for u, v in rows[1:]:
        assert 1 <= u < v <= n
        outgoing[u].add(v)
        has_incoming.add(v)
    starts = [u for u in range(1, n + 1) if u not in has_incoming]
    stops = [u for u in range(1, n + 1) if not outgoing[u]]
    assert len(starts) == len(stops)
    color = [1] * (n + 1)
    for i, (start, stop) in enumerate(zip(starts, stops), 1):
        color[start] = color[stop] = i
    for i in range(1, len(starts) + 1):
        ordered = [u for u in range(1, n + 1) if color[u] == i]
        for start, stop in zip(ordered, ordered[1:]):
            found, pending = {start}, [start]
            for u in pending:
                for v in outgoing[u]:
                    if v <= stop and v not in found:
                        found.add(v)
                        pending.append(v)
            assert stop in found
    return color[1:]


def _directed_game(rows):
    assert int(rows[0]) == len(rows) - 1
    moves = {}
    for word in rows[1:]:
        assert 3 <= len(word) <= 8 and word.isascii() and word.isalpha()
        moves.setdefault(word[:3], set()).add(word[-3:])
        moves.setdefault(word[-3:], set())
    status = {u: None if children else "Takahashi" for u, children in moves.items()}
    for _ in range(len(status)):
        last = status.copy()
        for u, children in moves.items():
            if any(last[v] == "Takahashi" for v in children):
                status[u] = "Aoki"
            elif all(last[v] == "Aoki" for v in children):
                status[u] = "Takahashi"
        if last == status:
            break
    return [status[word[-3:]] or "Draw" for word in rows[1:]]


def _dag_grundy(rows):
    (n,), values = rows
    assert len(values) == n and all(2 <= value <= 100000 for value in values)
    state = 0
    for value in values:
        factor_count = 0
        divisor = 2
        while divisor * divisor <= value:
            while value % divisor == 0:
                value //= divisor
                factor_count += 1
            divisor += 1
        state ^= factor_count + int(value != 1)
    return ["Anna" if state else "Bruno"]


def _tree_distance_layers(rows):
    n, = rows[0]
    adjacent = [[] for _ in range(n + 1)]
    for u, v in rows[1:n]:
        assert 1 <= u < v <= n
        adjacent[u].append(v)
        adjacent[v].append(u)
    q, = rows[n]
    assert len(rows) == n + q + 1
    result = []
    for source, k in rows[n + 1:]:
        assert 1 <= source <= n and 1 <= k <= n
        layers = {source: 0}
        pending = [source]
        for u in pending:
            for v in adjacent[u]:
                if v not in layers:
                    layers[v] = layers[u] + 1
                    pending.append(v)
        assert len(layers) == n
        result.append(min((v for v, distance in layers.items() if distance == k), default=-1))
    return result


def _tree_heap_orderings(rows):
    n, = rows[0]
    adjacency = [0] * n
    for u, v in rows[1:]:
        adjacency[u - 1] |= 1 << (v - 1)
        adjacency[v - 1] |= 1 << (u - 1)
    full = (1 << n) - 1

    @cache
    def continuations(chosen):
        if chosen == full:
            return 1
        return sum(continuations(chosen | (1 << v)) for v, neighbors in enumerate(adjacency)
                   if not (chosen >> v & 1) and chosen & neighbors)

    return [continuations(1 << root) % 1000000007 for root in range(n)]


def _difference_constraints(rows):
    n, m, target = rows[0]
    points = rows[1:n + 1]
    links = rows[n + 1:]
    assert len(links) == m
    infinity = 10 ** 30
    best = 0
    for sign in (1, -1):
        dist = [[infinity] * (n + 1) for _ in range(n + 1)]
        for u in range(n + 1):
            dist[u][u] = 0
        for u, (x, y, r) in enumerate(points, 1):
            center = x + sign * y
            dist[0][u], dist[u][0] = center + r, r - center
        for u, v, radius in links:
            dist[u][v] = dist[v][u] = radius
        for k in range(n + 1):
            for i in range(n + 1):
                for j in range(n + 1):
                    if dist[i][k] != infinity and dist[k][j] != infinity:
                        dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j])
        if any(dist[k][k] < 0 for k in range(n + 1)):
            return [-1]
        best = max(best, dist[0][target], dist[target][0])
    return [best]


def _tree_walk_orientation(rows):
    n, m = rows[0]
    edges, walks = rows[1:n], rows[n:]
    assert len(walks) == m
    graph = [[] for _ in range(n + 1)]
    for u, v in edges:
        graph[u].append(v)
        graph[v].append(u)

    def route(source, target):
        stack = [(source, 0, [])]
        while stack:
            v, parent, path = stack.pop()
            if v == target:
                return path
            stack.extend((w, v, [*path, (v, w)]) for w in graph[v] if w != parent)
        raise AssertionError((source, target))

    paths = [route(u, v) for u, v in walks]
    best, answer = -1, []
    for reversed_bits in product((False, True), repeat=m):
        seen = set()
        for reversed_path, path in zip(reversed_bits, paths):
            seen.update((v, u) if reversed_path else (u, v) for u, v in path)
        if len(seen) > best:
            best = len(seen)
            answer = [best]
            for reversed_path, (u, v) in zip(reversed_bits, walks):
                answer.extend((v, u) if reversed_path else (u, v))
    return answer


def _vertex_deletion_components(data):
    first, *grid = data.splitlines()
    height, width = map(int, first.split())
    live = {(r, c) for r in range(height) for c in range(width) if grid[r][c] == "#"}
    total = 0
    for deleted in live:
        remaining = live - {deleted}
        while remaining:
            total += 1
            queue = [remaining.pop()]
            for r, c in queue:
                for cell in ((r - 1, c), (r + 1, c), (r, c - 1), (r, c + 1)):
                    if cell in remaining:
                        remaining.remove(cell)
                        queue.append(cell)
    return total * pow(len(live), -1, 998244353) % 998244353


def _subsequence_shortest_paths(rows):
    n, m, q = rows[0]
    edges = rows[1:m + 1]
    schedule = rows[m + 1]
    assert len(schedule) == q
    best = None
    for used in range(1 << q):
        total, at = 0, 1
        for i, edge in enumerate(schedule):
            if used >> i & 1:
                a, b, cost = edges[edge - 1]
                if at != a:
                    break
                total += cost
                at = b
        else:
            if at == n and (best is None or total < best):
                best = total
    return [-1 if best is None else best]


def _tournament_strong_counts(rows):
    n, = rows[0]
    edges = [(u, v) for v in range(n) for u in range(v)]
    orientations = 1 << len(edges)
    answer = 0
    for state in range(orientations):
        reach = [1 << v for v in range(n)]
        indegree = [0] * n
        for i, (u, v) in enumerate(edges):
            if state >> i & 1:
                u, v = v, u
            reach[u] |= 1 << v
            indegree[v] += 1
        for middle in range(n):
            for u in range(n):
                if reach[u] >> middle & 1:
                    reach[u] |= reach[middle]
        greatest = max(indegree)
        champion = 0
        for v in range(n):
            if indegree[v] == greatest:
                champion |= reach[v]
        answer += champion.bit_count()
    return [answer * pow(orientations, -1, 998244353) % 998244353]


def _tournament_component_sums(rows):
    n, m = rows[0]
    edges = [(u, v) for v in range(n) for u in range(v)]
    total = 0
    for forward in combinations(range(len(edges)), m):
        selected = set(forward)
        reach = [1 << v for v in range(n)]
        for i, (u, v) in enumerate(edges):
            if i in selected:
                reach[u] |= 1 << v
            else:
                reach[v] |= 1 << u
        for middle in range(n):
            for v in range(n):
                if reach[v] >> middle & 1:
                    reach[v] |= reach[middle]
        total += sum(all(not (reach[u] >> v & 1 and reach[v] >> u & 1) for u in range(v)) for v in range(n))
    return [total % 998244353]


def _path_matching_candies(rows):
    n, = rows[0]
    weights = [a for a, in rows[1:]]
    best = [0] * ((n + 1) // 2 + 1)
    for subset in range(1, 1 << n):
        if subset & (subset >> 1):
            continue
        count = subset.bit_count()
        best[count] = max(best[count], sum(a for i, a in enumerate(weights) if subset >> i & 1))
    return best[1:]


def _core_decomposition(rows):
    n, m = rows[0]
    weights = rows[1]
    edges = [(u - 1, v - 1) for u, v in rows[2:]]
    assert len(weights) == n and len(edges) == m
    best = 0
    for state in range(1, 1 << n):
        scores = [0] * n
        for u, v in edges:
            if state >> u & 1 and state >> v & 1:
                scores[u] += weights[v]
                scores[v] += weights[u]
        best = max(best, min(score for v, score in enumerate(scores) if state >> v & 1))
    return [best]


def _forest_independent_set(rows):
    n, = rows[0]
    masks = [(1 << (u - 1)) | (1 << (v - 1)) for u, v in rows[1:]]
    return [sum(all(subset & edge != edge for edge in masks) for subset in range(1 << n)) % 1000000007]


def _forest_edge_coloring(rows):
    n, = rows[0]
    edges = [set(edge) for edge in rows[1:]]
    assert len(edges) == n - 1
    conflicts = [[bool(a & b) for b in edges] for a in edges]
    minimum = max(sum(v in edge for edge in edges) for v in range(1, n + 1))
    colors = []
    for i in range(len(edges)):
        occupied = {color for j, color in enumerate(colors) if conflicts[i][j]}
        colors.append(next(c for c in range(1, minimum + 1) if c not in occupied))
    return [minimum, *colors]


def _forest_subset_components(rows):
    n, = rows[0]
    assert 1 <= n <= 5000
    graph = [[] for _ in range(n)]
    assert len(rows) == n
    for u, v in rows[1:]:
        assert 1 <= u < v <= n
        graph[u - 1].append(v - 1)
        graph[v - 1].append(u - 1)
    counts = [0] * (n + 1)
    for mask in range(1, 1 << n):
        pending = {i for i in range(n) if mask >> i & 1}
        components = 0
        while pending:
            components += 1
            layer = [pending.pop()]
            for u in layer:
                for v in graph[u]:
                    if v in pending:
                        pending.remove(v)
                        layer.append(v)
        counts[components] += 1
    return [count % 998244353 for count in counts[1:]]


def _incident_label_spanning_tree(data):
    tokens = data.split()
    n, m = map(int, tokens[:2])
    labels = tokens[-1]
    assert 2 <= n <= 200000 and n - 1 <= m <= 200000 and len(tokens) == 3 * m + 3
    assert len(labels) == n and set(labels) <= {"R", "B"}
    edges = [(int(tokens[i]) - 1, int(tokens[i + 1]) - 1, tokens[i + 2]) for i in range(2, 3 * m + 2, 3)]
    assert len(set(edges)) == m and all(0 <= u < v < n and c in {"R", "B"} for u, v, c in edges)
    for selected in combinations(range(m), n - 1):
        graph = [[] for _ in range(n)]
        checked = set()
        for edge in selected:
            u, v, label = edges[edge]
            graph[u].append(v)
            graph[v].append(u)
            if labels[u] == label:
                checked.add(u)
            if labels[v] == label:
                checked.add(v)
        if len(checked) != n:
            continue
        queue, seen = [0], {0}
        for u in queue:
            for v in graph[u]:
                if v not in seen:
                    seen.add(v)
                    queue.append(v)
        if len(seen) == n:
            return ["Yes", *(i + 1 for i in selected)]
    return ["No"]


def _interval_cover_subsets(rows):
    n, m = rows[0]
    assert 2 <= n <= 200000 and 1 <= m <= 200000 and len(rows) == m + 1
    assert all(1 <= u < v <= n for u, v in rows[1:])
    count = 0
    for mask in range(1 << m):
        edges = [[] for _ in range(n)]
        for v in range(1, n):
            edges[v].append(v - 1)
        for i, (u, v) in enumerate(rows[1:]):
            if mask >> i & 1:
                edges[u - 1].append(v - 1)
        reachable, pending = {0}, [0]
        for u in pending:
            for v in edges[u]:
                if v not in reachable:
                    reachable.add(v)
                    pending.append(v)
        count += len(reachable) == n
    return [count % 998244353]


def _strong_connectivity_augmentation(rows):
    n, m = rows[0]
    assert 1 <= n <= 10000 and 0 <= m <= 100000 and len(rows) == m + 1
    assert len(set(map(tuple, rows[1:]))) == m
    reach = [[u == v for v in range(n)] for u in range(n)]
    for u, v in rows[1:]:
        assert 0 <= u < n and 0 <= v < n and u != v
        reach[u][v] = True
    for k in range(n):
        for u in range(n):
            for v in range(n):
                reach[u][v] |= reach[u][k] and reach[k][v]
    groups = [min(u for u in range(n) if reach[u][v] and reach[v][u]) for v in range(n)]
    components = set(groups)
    if len(components) == 1:
        return [0]
    indegree = {groups[v] for u, v in rows[1:] if groups[u] != groups[v]}
    outdegree = {groups[u] for u, v in rows[1:] if groups[u] != groups[v]}
    return [max(len(components - indegree), len(components - outdegree))]


def _affine_walk_ranges(rows):
    n, m = rows[0]
    assert 1 <= n <= 200000 and 0 <= m <= 200000 and len(rows) == m + 1
    assert len(set(map(tuple, rows[1:]))) == m
    graph = [[] for _ in range(n + 1)]
    for u, v, bit in rows[1:]:
        assert 1 <= u <= n and 1 <= v <= n and bit in (0, 1)
        graph[u].append((v, bit))
    pending = deque([(1, ())])
    seen = {(1, ())}
    maximum = [-1] * (n + 1)
    grow = set()
    while pending:
        v, word = pending.popleft()
        maximum[v] = max(maximum[v], len(word))
        for u, bit in graph[v]:
            rewritten = word[:-1] if word and word[-1] == bit else word + (bit,)
            if len(rewritten) > 2 * n:
                grow.add(u)
            elif (u, rewritten) not in seen:
                pending.append((u, rewritten))
                seen.add((u, rewritten))
    assert all(value >= 0 or v in grow for v, value in enumerate(maximum[1:], 1))
    todo = list(grow)
    for v in todo:
        maximum[v] = -1
        for u, _ in graph[v]:
            if u not in grow:
                grow.add(u)
                todo.append(u)
    return maximum[1:]


def _superincreasing_path_queries(rows):
    n, m = rows[0]
    q, = rows[m + 1]
    assert 2 <= n <= 200000 and n - 1 <= m <= 200000 and 1 <= q <= 200000
    assert len(rows) == m + q + 2
    graph = [[] for _ in range(n + 1)]
    edges = rows[1:m + 1]
    assert len(set(map(tuple, edges))) == m
    for edge_id, (u, v) in enumerate(edges, 1):
        assert 1 <= u < v <= n
        graph[u].append((v, edge_id))
        graph[v].append((u, edge_id))
    answers = []
    for u, v, removed in rows[m + 2:]:
        assert 1 <= u < v <= n and 1 <= removed <= m
        cost = {u: 0}
        pending = [(0, u)]
        while pending:
            length, here = heappop(pending)
            if length != cost[here]:
                continue
            if here == v:
                break
            for head, edge_id in graph[here]:
                if edge_id != removed:
                    candidate = length + (1 << edge_id)
                    if head not in cost or candidate < cost[head]:
                        cost[head] = candidate
                        heappush(pending, (candidate, head))
        answers.append(cost[v] % 1000000007 if v in cost else -1)
    return answers


def _chinese_postman_route(rows):
    n, m = rows[0]
    assert 2 <= n <= 100000 and n - 1 <= m <= 100000 and len(rows) == m + 1
    graph = [[] for _ in range(n + 1)]
    for edge_id, (u, v, weight) in enumerate(rows[1:]):
        assert 1 <= u <= n and 1 <= v <= n and u != v and 1 <= weight <= 10**6
        bit = 1 << edge_id
        graph[u].append((v, weight, bit))
        graph[v].append((u, weight, bit))
    assert sum(len(neighbors) % 2 for neighbors in graph) <= 16
    full = (1 << m) - 1
    pending = [(0, 1, 0)]
    cost = {(1, 0): 0}
    while pending:
        value, node, seen = heappop(pending)
        if cost[node, seen] != value:
            continue
        if node == 1 and seen == full:
            return [value]
        for other, weight, bit in graph[node]:
            key = other, seen | bit
            candidate = value + weight
            if candidate < cost.get(key, float("inf")):
                cost[key] = candidate
                heappush(pending, (candidate, *key))
    raise AssertionError("route does not cover all edges")


def _tree_backedge_absorption(rows):
    n, m = rows[0]
    parents = rows[1]
    assert 2 <= n <= 30 and len(parents) == n - 1 and len(rows) == m + 2
    transitions = [[] for _ in range(n)]
    for v, parent in enumerate(parents, 1):
        assert 1 <= parent <= v
        transitions[parent - 1].append(v)
    for start, ancestor in rows[2:]:
        assert 1 <= ancestor < start <= n and transitions[start - 1]
        cursor = start
        while cursor > ancestor:
            cursor = parents[cursor - 2]
        assert cursor == ancestor
        transitions[start - 1].append(ancestor - 1)

    modulus = 998244353
    matrix = [[0] * (n + 1) for _ in range(n)]
    for node, neighbors in enumerate(transitions):
        matrix[node][node] = max(1, len(neighbors))
        matrix[node][-1] = len(neighbors)
        for destination in neighbors:
            matrix[node][destination] -= 1
    for current in range(n):
        pivot = next(i for i in range(current, n) if matrix[i][current] % modulus != 0)
        matrix[current], matrix[pivot] = matrix[pivot], matrix[current]
        scale = pow(matrix[current][current], -1, modulus)
        matrix[current] = [value * scale % modulus for value in matrix[current]]
        for i, row in enumerate(matrix):
            if i != current:
                mult = row[current]
                matrix[i] = [(value - mult * reference) % modulus for value, reference in zip(row, matrix[current])]
    return [matrix[0][-1], modulus]


def _tree_edge_color_path_counts(rows):
    n, k = rows[0]
    assert 1 <= n <= 30 and 1 <= k <= 200000 and len(rows) == n
    adjacent = [[] for _ in range(n + 1)]
    for u, v, color in rows[1:]:
        assert 1 <= u <= n and 1 <= v <= n and 1 <= color <= k
        adjacent[u].append((v, color))
        adjacent[v].append((u, color))
    count = 0
    for start in range(1, n + 1):
        pending = [(start, frozenset())]
        seen = {start}
        while pending:
            here, colors = pending.pop()
            count += start < here and len(colors) == 2
            for there, color in adjacent[here]:
                if there not in seen:
                    pending.append((there, colors | {color}))
                    seen.add(there)
        assert len(seen) == n
    return [count]


def _dynamic_edge_products(rows):
    n, q = rows[0]
    assert 2 <= n <= 100000 and 1 <= q <= 100000 and len(rows) == q + 1
    active = set()
    result = []
    for u, v in rows[1:]:
        assert 1 <= u < v <= n
        active ^= {(u, v)}
        degree = [0] * (n + 1)
        for a, b in active:
            degree[a] += 1
            degree[b] += 1
        result.append(sum(degree[a] * degree[b] for a, b in active))
    return result


def _directed_return_distances(rows):
    n, m = rows[0]
    assert 1 <= n <= 20 and 1 <= m <= 2000 and len(rows) == m + 1
    graph = [[] for _ in range(n)]
    for u, v, w in rows[1:]:
        assert 1 <= u <= n and 1 <= v <= n and 1 <= w <= 10**5
        graph[u - 1].append((v - 1, w))
    result = []
    for source in range(n):
        shortest = None

        def visit(u, used, cost):
            nonlocal shortest
            for v, w in graph[u]:
                if v == source:
                    candidate = cost + w
                    shortest = candidate if shortest is None else min(shortest, candidate)
                elif v not in used:
                    visit(v, used | {v}, cost + w)
        visit(source, {source}, 0)
        result.append(shortest if shortest is not None else -1)
    return result


def _tree_distance_sums(rows):
    n, q = rows[0]
    graph = _tree(n, rows[1:n])
    assert 1 <= n <= 200000 and 1 <= q <= 200000 and len(rows) == n + q
    weights = [0] * n
    distance = [_distances(graph, v) for v in range(n)]
    answers = []
    for kind, u, value in rows[n:]:
        assert 1 <= u <= n
        if kind == 1:
            assert 1 <= value <= 10**9
            weights[u - 1] += value
        else:
            assert kind == 2 and 1 <= value <= n
            u, v = u - 1, value - 1
            path = [x for x in range(n) if distance[u][x] + distance[x][v] == distance[u][v]]
            answers.append(sum(sum(w * d for w, d in zip(weights, distance[x])) for x in path) % 998244353)
    return answers


def _bitwise_path(rows):
    n, m = rows[0]
    assert 2 <= n <= 100 and n - 1 <= m <= 200000 and len(rows) == m + 1
    graph = [[] for _ in range(n)]
    for u, v, weight in rows[1:]:
        assert 1 <= u < v <= n and 0 <= weight < 64
        graph[u - 1].append((v - 1, weight))
        graph[v - 1].append((u - 1, weight))
    seen = [{0}] + [set() for _ in range(n - 1)]
    queue = [(0, 0)]
    for u, mask in queue:
        for v, added in graph[u]:
            value = mask | added
            if value not in seen[v]:
                seen[v].add(value)
                queue.append((v, value))
    assert all(seen)
    return [min(seen[-1])]


def _graph_metric_realization(rows):
    n, = rows[0]
    target = rows[1:]
    assert 1 <= n <= 8 and len(target) == n and all(len(row) == n for row in target)
    assert all(target[u][u] == 0 for u in range(n))
    assert all(1 <= target[u][v] == target[v][u] <= 10**9 for u, v in combinations(range(n), 2))
    edges = {(u, v): target[u][v] for u, v in combinations(range(n), 2)}

    def distances_now():
        distance = [[0 if u == v else 10**18 for v in range(n)] for u in range(n)]
        for (u, v), weight in edges.items():
            distance[u][v] = distance[v][u] = weight
        for k in range(n):
            for u in range(n):
                for v in range(n):
                    distance[u][v] = min(distance[u][v], distance[u][k] + distance[k][v])
        return distance

    if distances_now() != target:
        return [-1]
    for edge in sorted(edges, key=edges.__getitem__, reverse=True):
        weight = edges.pop(edge)
        if distances_now() != target:
            edges[edge] = weight
    return [sum(edges.values())]


def _tree_permutation_distance(rows):
    n, = rows[0]
    assert 2 <= n <= 8
    tree = _tree(n, rows[1:])
    dist = [_distances(tree, start) for start in range(n)]
    scores = Counter(sum(dist[source][target] for source, target in enumerate(order))
                     for order in permutations(range(n)))
    return [scores[max(scores)] % 1_000_000_007]


def _shortest_odd_cycle(rows):
    n, m, length, source, target = rows[0]
    assert 2 <= n <= 12 and 1 <= length <= 12 and 0 <= m <= min(n * (n - 1) // 2, 2000)
    assert 1 <= source <= n and 1 <= target <= n and source != target
    graph = [set() for _ in range(n + 1)]
    assert len(rows) == m + 1
    for u, v in rows[1:]:
        assert 1 <= u < v <= n and v not in graph[u]
        graph[u].add(v)
        graph[v].add(u)

    def find_target():
        at = {source}
        for _ in range(length):
            at = {v for u in at for v in graph[u]}
        return target in at

    if find_target():
        return ["Yes"]
    for u in range(1, n + 1):
        for v in range(u + 1, n + 1):
            if v in graph[u]:
                continue
            graph[u].add(v)
            graph[v].add(u)
            if find_target():
                return ["Yes"]
            graph[u].remove(v)
            graph[v].remove(u)
    return ["No"]


def _binary_tree_traversal(rows):
    n, = rows[0]
    preorder, inorder = rows[1:]
    assert 2 <= n <= 12 and sorted(preorder) == sorted(inorder) == list(range(1, n + 1))
    rank = {v: i for i, v in enumerate(preorder)}
    result = [(0, 0)] * (n + 1)
    generated = []

    def cartesian(order):
        if not order:
            return 0
        root = min(order, key=rank.__getitem__)
        index = order.index(root)
        generated.append(root)
        result[root] = cartesian(order[:index]), cartesian(order[index + 1:])
        return root

    if cartesian(inorder) != 1 or generated != preorder:
        return [-1]
    return [v for children in result[1:] for v in children]


def _tree_metric(rows):
    n, = rows[0]
    assert 2 <= n <= 12 and len(rows) == n
    matrix = [[0] * n for _ in range(n)]
    for u, row in enumerate(rows[1:]):
        assert len(row) == n - u - 1 and all(1 <= d <= 9999 for d in row)
        for v, d in enumerate(row, u + 1):
            matrix[u][v] = matrix[v][u] = d
    edges = [[] for _ in range(n)]
    for u in range(n):
        for v in range(u + 1, n):
            if any(k != u and k != v and matrix[u][v] == matrix[u][k] + matrix[k][v] for k in range(n)):
                continue
            edges[u].append(v)
            edges[v].append(u)
    if sum(map(len, edges)) != 2 * (n - 1):
        return ["No"]
    for source in range(n):
        distance = {source: 0}
        pending = [source]
        for u in pending:
            for v in edges[u]:
                if v not in distance:
                    distance[v] = distance[u] + matrix[u][v]
                    pending.append(v)
        if [distance.get(v) for v in range(n)] != matrix[source]:
            return ["No"]
    return ["Yes"]


def _exact_length_walk_bottleneck(rows):
    n, m, length = rows[0]
    edges = rows[1:]
    assert 2 <= n <= 100 and 1 <= m <= n**2 and 1 <= length <= 30
    assert len(edges) == m and len(set(map(tuple, edges))) == m
    assert all(1 <= u <= n and 1 <= v <= n for u, v in edges)
    result = [-1] * n
    graph = [set() for _ in range(n)]
    # Independent by activation times: simulate the Boolean reachability layers.
    for instant, (u, v) in enumerate(edges, 1):
        graph[u - 1].add(v - 1)
        seen = {0}
        for _ in range(length):
            seen = {w for p in seen for w in graph[p]}
        for w in seen:
            if result[w] == -1:
                result[w] = instant
    return result


def _exact_length_walk_weighted_count(rows):
    n, m, length = rows[0]
    edges = rows[1:]
    assert 2 <= n <= 100 and 0 <= m <= n * (n - 1) and 1 <= length <= 30
    assert len(edges) == m and len({(u, v) for u, v, _ in edges}) == m
    assert all(1 <= u <= n and 1 <= v <= n and u != v and 0 <= weight <= 10**9
               for u, v, weight in edges)
    count = [1] + [0] * (n - 1)
    for _ in range(length):
        after = [0] * n
        for u, v, weight in edges:
            after[v - 1] += count[u - 1] * weight
        count = [value % 998244353 for value in after]
    return [count[-1]]


def _undirected_walk_lengths(rows):
    n, m, length = rows[0]
    source, target = rows[1]
    assert 2 <= n <= 100000 and 1 <= m <= min(100000, n * (n - 1) // 2)
    assert 1 <= source <= n and 1 <= target <= n and source != target
    assert 1 <= length <= 30 and len(rows) == m + 2
    graph = [set() for _ in range(n + 1)]
    for u, v in rows[2:]:
        assert 1 <= u <= n and 1 <= v <= n and u != v and v not in graph[u]
        graph[u].add(v)
        graph[v].add(u)

    def layers(start):
        result = [{start}]
        for _ in range(length):
            result.append({v for u in result[-1] for v in graph[u]})
        return result

    forward = layers(source)
    backward = layers(target)
    vertices = sorted(set.union(*(left & right for left, right in zip(forward, reversed(backward)))))
    return [len(vertices), *vertices] if vertices else [-1]


def _subcubic_tree_biconnectivity(rows):
    assert rows[0] == [1]
    n, = rows[1]
    weights = rows[2]
    adjacency = _tree(n, rows[3:])
    assert n <= 8 and max(map(len, adjacency)) <= 3
    base = [set(neighbors) for neighbors in adjacency]
    _distances(base, 0)
    options = [(u, v) for u in range(n) for v in range(u + 1, n) if v not in base[u]]

    def stays_connected(chosen):
        expanded = [neighbors.copy() for neighbors in base]
        for i in chosen:
            u, v = options[i]
            expanded[u].add(v)
            expanded[v].add(u)
        for cut in range(n):
            reached = {cut}
            pending = [int(cut == 0)]
            reached.update(pending)
            for u in pending:
                for v in expanded[u] - reached:
                    reached.add(v)
                    pending.append(v)
            if len(reached) != n:
                return False
        return True

    pending = [(0, ())]
    while pending:
        cost, chosen = heappop(pending)
        if stays_connected(chosen):
            return [len(chosen), *(v + 1 for i in chosen for v in options[i])]
        for i in range(chosen[-1] + 1 if chosen else 0, len(options)):
            u, v = options[i]
            heappush(pending, (cost + weights[u] + weights[v], (*chosen, i)))
    raise AssertionError("no biconnectivity augmentation")


def _resource_constrained_paths(rows):
    n, m, initial = rows[0]
    assert 2 <= n <= 50 and n - 1 <= m <= 100 and 0 <= initial <= 10**9
    assert len(rows) == 1 + m + n
    between = [dict() for _ in range(n)]
    for u, v, fare, time in rows[1:1 + m]:
        assert 1 <= u < v <= n and v - 1 not in between[u - 1]
        assert 1 <= fare <= 50 and 1 <= time <= 10**9
        between[u - 1][v - 1] = fare, time
        between[v - 1][u - 1] = fare, time
    assert min(_distances(between, 0)) >= 0
    exchange = rows[1 + m:]
    assert all(1 <= gain <= 10**9 and 1 <= time <= 10**9 for gain, time in exchange)
    cap = 50 * n
    initial = min(initial, cap)
    inf = 10**20
    distance = [[inf] * (cap + 1) for _ in range(n)]
    distance[0][initial] = 0
    pending = [(0, 0, initial)]
    while pending:
        time, vertex, remaining = heappop(pending)
        if time != distance[vertex][remaining]:
            continue
        transitions = [(v, remaining - fare, travel) for v, (fare, travel) in between[vertex].items()
                       if fare <= remaining]
        gain, cost = exchange[vertex]
        transitions.append((vertex, min(cap, remaining + gain), cost))
        for v, after, cost in transitions:
            candidate = time + cost
            if candidate < distance[v][after]:
                distance[v][after] = candidate
                heappush(pending, (candidate, v, after))
    return [min(states) for states in distance[1:]]


def _unrooted_tree_leaf_deletion(rows):
    (n,), larger, smaller = rows
    assert 2 <= n <= 200000 and len(larger) == n and len(smaller) == n - 1
    full = _tree(n + 1, list(enumerate(larger, 1)))
    short = _tree(n, list(enumerate(smaller, 1)))

    def canonical(graph, cut):
        def rooted(v, parent):
            return "(" + "".join(sorted(rooted(w, v) for w in graph[v] if w != parent and w != cut)) + ")"

        return min(rooted(v, -1) for v in range(len(graph)) if v != cut)

    goal = canonical(short, -1)
    answer = [v + 1 for v, adjacent in enumerate(full) if len(adjacent) == 1 and canonical(full, v) == goal]
    assert answer
    return answer


def _component_add_union_find(rows):
    n, q = rows[0]
    assert 1 <= n <= 500000 and 1 <= q <= 500000 and len(rows) == q + 1
    graph = [set() for _ in range(n + 1)]
    values = [0] * (n + 1)
    outputs = []
    for kind, a, b in rows[1:]:
        assert 1 <= a <= n
        if kind == 1:
            assert 1 <= b <= n and a != b and b not in graph[a]
            graph[a].add(b)
            graph[b].add(a)
        elif kind == 2:
            assert -2000 <= b <= 2000
            seen = {a}
            queue = deque([a])
            while queue:
                v = queue.popleft()
                values[v] += b
                for u in graph[v] - seen:
                    queue.append(u)
                    seen.add(u)
        else:
            assert kind == 3 and b == 0
            outputs.append(values[a])
    return outputs


def _hypergraph_shortest_paths_merge(rows):
    groups, n = rows[0]
    assert 1 <= groups <= 200000 and 2 <= n <= 200000 and len(rows) == 2 * groups + 1
    total = 0
    graph = [set() for _ in range(n + 1)]
    for (size,), vertices in zip(rows[1::2], rows[2::2]):
        assert size == len(vertices) == len(set(vertices))
        assert all(1 <= v <= n for v in vertices)
        total += size
        for v in vertices:
            graph[v].update(vertices)
    assert 1 <= total <= 500000
    queue = deque([1])
    distance = {1: 0}
    while queue:
        v = queue.popleft()
        for u in graph[v]:
            if u not in distance:
                distance[u] = distance[v] + 1
                queue.append(u)
    return [distance.get(n, 0) - 1]


def _shortest_nonzero_paths(data):
    header, *rows = data.splitlines()
    n, m, bits = map(int, header.split())
    assert 2 <= n <= 100000 and 1 <= m <= 200000 and 1 <= bits <= 30 and len(rows) == m
    edges = [[] for _ in range(n)]
    for row in rows:
        first, second, weight, label = row.split()
        u, v, distance = int(first) - 1, int(second) - 1, int(weight)
        assert 0 <= u < n and 0 <= v < n and u != v and 1 <= distance <= 10**9
        assert len(label) == bits and set(label) <= {"0", "1"}
        xor = int(label, 2)
        edges[u].append((v, distance, xor))
        edges[v].append((u, distance, xor))

    best = {}
    distance = [None] * n
    reached = set()

    def visit(v, seen, label, total):
        reached.add(v)
        key = v, seen, label
        if key in best and best[key] <= total:
            return
        best[key] = total
        if label and (distance[v] is None or total < distance[v]):
            distance[v] = total
        for w, length, xor in edges[v]:
            if not seen & (1 << w):
                visit(w, seen | (1 << w), label ^ xor, total + length)

    visit(n - 1, 1 << (n - 1), 0, 0)
    assert len(reached) == n
    return [-1 if result is None else result for result in distance[:-1]]


def _dsu_on_tree_mode(rows):
    (n, state, given, palette), parent, color = rows
    assert 2 <= n <= 2500000 and 2 <= given <= min(n, 100000)
    assert 1 <= palette <= n and 1 <= state < 1 << 31
    assert len(parent) == given - 1 and all(1 <= p <= v for v, p in enumerate(parent, 1))
    assert len(color) == given and all(1 <= c <= n for c in color)
    children = [[] for _ in range(n)]
    for v in range(1, n):
        if v < given:
            p = parent[v - 1] - 1
        else:
            p = state % v
            state = (1103515245 * state + 12345) & ((1 << 31) - 1)
        children[p].append(v)
    colors = color.copy()
    for _ in range(given, n):
        colors.append(state % palette + 1)
        state = (1103515245 * state + 12345) & ((1 << 31) - 1)
    total = 0
    for v in range(n):
        counts = Counter()
        todo = [v]
        while todo:
            w = todo.pop()
            counts[colors[w]] += 1
            todo.extend(children[w])
        maximum = max(counts.values())
        ties = list(counts.values()).count(maximum)
        total += (maximum ^ (v + 1)) * (ties ^ (v + 1))
    return [total % 998244353]


def _unit_step_forest_labels(rows):
    n, = rows[0]
    assert 1 <= n <= 100000
    graph = _tree(n, rows[1:n])
    k, = rows[n]
    assert 1 <= k <= n and len(rows) == n + k + 1
    fixed = {v - 1: label for v, label in rows[n + 1:]}
    assert len(fixed) == k and all(0 <= v < n and 0 <= label <= 10**6 for v, label in fixed.items())

    labels = [0] * n
    root = next(iter(fixed))

    def search(v, parent, label):
        if v in fixed and fixed[v] != label:
            return False
        labels[v] = label
        return all(
            (search(child, v, label + 1) or search(child, v, label - 1))
            for child in graph[v] if child != parent
        )

    return ["Yes", *labels] if search(root, -1, fixed[root]) else ["No"]


def _walk_weight_congruence_return(rows):
    tests, = rows[0]
    assert tests == 1
    n, m = rows[1]
    assert 2 <= n <= 200000 and 1 <= m <= 200000 and len(rows) == m + 2
    edges = set(map(tuple, rows[2:]))
    assert len(edges) == m
    forward = [[] for _ in range(n + 1)]
    reverse = [[] for _ in range(n + 1)]
    for u, v in edges:
        assert 1 <= u <= n and 1 <= v <= n and u != v
        forward[u].append(v)
        reverse[v].append(u)

    def connected(graph):
        seen = {1}
        pending = [1]
        while pending:
            u = pending.pop()
            for v in graph[u]:
                if v not in seen:
                    seen.add(v)
                    pending.append(v)
        return seen

    component = connected(forward) & connected(reverse)
    period = 0

    def visit(v, used):
        nonlocal period
        for w in forward[v]:
            if w == start:
                period = gcd(period, len(used))
            elif w > start and w in component and w not in used:
                visit(w, used | {w})

    for start in component:
        visit(start, {start})
    return ["Yes" if period and pow(10, n, period) == 0 else "No"]


def _tree_path_difference_road(rows):
    n, = rows[0]
    assert 1 <= n <= 50000
    graph = [[] for _ in range(n)]
    weights = {}
    for v, (p, w) in enumerate(rows[1:n], 1):
        assert 1 <= p <= v and 0 <= w <= 10**9
        graph[p - 1].append(v)
        graph[v].append(p - 1)
        weights[p - 1, v] = w

    def path(source, target):
        assert 1 <= source <= n and 1 <= target <= n
        queue = deque([source - 1])
        predecessor = {source - 1: None}
        while target - 1 not in predecessor:
            v = queue.popleft()
            for w in graph[v]:
                if w not in predecessor:
                    predecessor[w] = v
                    queue.append(w)
        v = target - 1
        while predecessor[v] is not None:
            w = predecessor[v]
            yield min(v, w), max(v, w)
            v = w

    q, = rows[n]
    assert 1 <= q <= 50000
    for u, v in rows[n + 1:n + 1 + q]:
        for edge in path(u, v):
            weights[edge] += 1
    r, = rows[n + q + 1]
    assert 1 <= r <= 50000 and len(rows) == n + q + r + 2
    return [min((weights[edge] for edge in path(u, v)), default=0) for u, v in rows[n + q + 2:]]


def _subtree_edge_add_distances(rows):
    n, = rows[0]
    assert 2 <= n <= 100000
    children = [[] for _ in range(n)]
    parent = {}
    weight = {}
    for u, v, w in rows[1:n]:
        assert 0 <= u < n and 0 < v < n and u != v and v not in parent and 0 <= w <= 100000
        children[u].append(v)
        parent[v], weight[v] = u, w
    q, = rows[n]
    assert 1 <= q <= 100000 and len(rows) == n + q + 1
    answers = []
    for kind, v, *arguments in rows[n + 1:]:
        assert 0 <= v < n
        if kind == 1:
            delta, = arguments
            assert 0 <= delta <= 100000
            pending = children[v].copy()
            while pending:
                w = pending.pop()
                weight[w] += delta
                pending.extend(children[w])
        else:
            assert kind == 2 and not arguments
            total = 0
            while v:
                total += weight[v]
                v = parent[v]
            answers.append(total)
    return answers


def _automaton_shortest_path_hopscotch(rows):
    n, m = rows[0]
    assert len(rows) == m + 2
    edges = set(map(tuple, rows[1:-1]))
    assert len(edges) == m
    adjacency = [[] for _ in range(n)]
    for u, v in edges:
        assert 1 <= u <= n and 1 <= v <= n and u != v
        adjacency[u - 1].append(v - 1)
    source, target = rows[-1]
    assert 1 <= source <= n and 1 <= target <= n and source != target
    queue = deque([source - 1])
    distance = [-1] * n
    distance[source - 1] = 0
    while queue:
        u = queue.popleft()
        for first in adjacency[u]:
            for second in adjacency[first]:
                for v in adjacency[second]:
                    if distance[v] == -1:
                        distance[v] = distance[u] + 1
                        queue.append(v)
    return [distance[target - 1]]


def _dag_critical_path_schedule(rows):
    n, m = rows[0]
    assert 3 <= n <= 100000 and 2 <= m <= 500000 and len(rows) == m + 1
    graph = [[] for _ in range(n)]
    for u, v, duration in rows[1:]:
        assert 0 <= u < v < n and 0 <= duration < 160
        graph[u].append((v, duration))
    paths = []

    def visit(path, time):
        u = path[-1]
        if u == n - 1:
            paths.append((time, path))
            return
        for v, duration in graph[u]:
            visit((*path, v), time + duration)

    visit((0,), 0)
    reachable = {v for _, path in paths for v in path}
    assert len(reachable) == n
    makespan = max(time for time, _ in paths)
    critical = {v for time, path in paths if time == makespan for v in path}
    return [makespan, f"{n - len(critical)}/{n}"]


def _bottleneck_pair_histogram(rows):
    n, = rows[0]
    assert 2 <= n <= 100000 and len(rows) == n
    graph = [[] for _ in range(n)]
    for u, v, weight in rows[1:]:
        assert 1 <= u < v <= n and 1 <= weight <= 10 ** 7
        graph[u - 1].append((v - 1, weight))
        graph[v - 1].append((u - 1, weight))
    total = 0
    for root in range(n):
        seen = {root}
        pending = [(root, 0)]
        for u, maximum in pending:
            if u > root:
                total += maximum
            for v, weight in graph[u]:
                if v not in seen:
                    seen.add(v)
                    pending.append((v, max(weight, maximum)))
        assert len(seen) == n
    return [total]


def _lexicographic_shortest_path(rows):
    n, m, source, target = rows[0]
    assert 2 <= n <= 8 and len(rows) == m + 1 and 0 <= source < n and 0 <= target < n and source != target
    assert n - 1 <= m <= n * (n - 1) // 2
    graph = [[] for _ in range(n)]
    unique = set()
    for u, v, weight in rows[1:]:
        assert 0 <= u < v < n and 1 <= weight <= 10000 and (u, v) not in unique
        unique.add((u, v))
        graph[u].append((v, weight))
        graph[v].append((u, weight))

    def visit(path, weight):
        u = path[-1]
        if u == target:
            return weight, path
        candidates = (visit((*path, v), weight + w) for v, w in graph[u] if v not in path)
        return min((candidate for candidate in candidates if candidate is not None), default=None)

    answer = visit((source,), 0)
    assert answer is not None
    return answer[1]


def _tree_path_bitwise_counts_and(rows):
    n, = rows[0]
    edges = rows[1:]
    assert 2 <= n <= 100000 and len(edges) == n - 1
    graph = [[] for _ in range(n)]
    for u, v, weight in edges:
        assert 1 <= u < v <= n and 1 <= weight <= 10 ** 9
        graph[u - 1].append((v - 1, weight))
        graph[v - 1].append((u - 1, weight))
    total = 0
    for root in range(n):
        pending = [(root, (1 << 64) - 1)]
        seen = {root}
        for u, value in pending:
            if u > root:
                total += value
            for v, weight in graph[u]:
                if v not in seen:
                    seen.add(v)
                    pending.append((v, value & weight))
        assert len(seen) == n
    return [total % 1000000007]


def _johnson_shortest_paths(rows):
    n, m = rows[0]
    assert 2 <= n <= 100 and 1 <= m <= 9900 and len(rows) == m + 1
    missing = 10 ** 50
    distances = [[missing for _ in range(n)] for _ in range(n)]
    for u in range(n):
        distances[u][u] = 0
    for u, v, weight in rows[1:]:
        assert 1 <= u <= n and 1 <= v <= n and u != v and abs(weight) <= 10 ** 12
        distances[u - 1][v - 1] = min(distances[u - 1][v - 1], weight)
    for k in range(n):
        for u in range(n):
            for v in range(n):
                if distances[u][k] != missing and distances[k][v] != missing:
                    distances[u][v] = min(distances[u][v], distances[u][k] + distances[k][v])
    assert all(distances[u][u] == 0 for u in range(n))
    return [sum(d for d in row if d != missing) for row in distances]


def _incremental_bridges(rows):
    n, m, source, target = rows[0]
    assert 2 <= n <= 200000 and 1 <= m <= 200000 and len(rows) == m + 1
    assert 1 <= source <= n and 1 <= target <= n and source != target
    graph = [[] for _ in range(n)]
    unique = set()
    for i, (u, v) in enumerate(rows[1:]):
        assert 1 <= u < v <= n and (u, v) not in unique
        unique.add((u, v))
        graph[u - 1].append((v - 1, i))
        graph[v - 1].append((u - 1, i))

    in_cycle = []
    for i, (u, v) in enumerate(rows[1:]):
        u, v = u - 1, v - 1
        seen = {u}
        pending = [u]
        for a in pending:
            for b, edge in graph[a]:
                if edge != i and b not in seen:
                    seen.add(b)
                    pending.append(b)
        in_cycle.append(v in seen)

    # Lexicographic Dijkstra prefers few edges first and many cycle edges second.
    source, target = source - 1, target - 1
    best = [(n + 1, 0)] * n
    best[source] = (0, 0)
    pending = [(0, 0, source)]
    while pending:
        distance, penalty, u = heappop(pending)
        if (distance, penalty) != best[u]:
            continue
        if u == target:
            return [-penalty]
        for v, edge in graph[u]:
            score = (distance + 1, penalty - in_cycle[edge])
            if score < best[v]:
                best[v] = score
                heappush(pending, (*score, v))
    return [-1]


def _enumerate_topological_orders(rows):
    n, m, k = rows[0]
    assert 1 <= n <= 8 and 0 <= m <= 100000 and 1 <= k <= 10
    edges = [tuple(edge) for edge in rows[1:]]
    assert len(edges) == len(set(edges)) == m and all(1 <= u <= n and 1 <= v <= n and u != v for u, v in edges)
    output = []
    for order in permutations(range(1, n + 1)):
        position = {vertex: i for i, vertex in enumerate(order)}
        if all(position[u] < position[v] for u, v in edges):
            output.append(order)
            if len(output) == k:
                return [vertex for row in output for vertex in row]
    return [-1]


def _equal_tree_path_partition(rows):
    groups, length = rows[0]
    n = groups * length
    edges = rows[1:]
    tree = _tree(n, edges)
    assert all(u < v for u, v in edges) and all(d >= 0 for d in _distances(tree, 0))
    for retained in combinations(edges, n - groups):
        graph = [[] for _ in range(n)]
        for u, v in retained:
            graph[u - 1].append(v - 1)
            graph[v - 1].append(u - 1)
        if any(len(neighbors) > 2 for neighbors in graph):
            continue
        unseen = set(range(n))
        valid = True
        while unseen:
            u = next(iter(unseen))
            reached = {u}
            pending = [u]
            for v in pending:
                for w in graph[v]:
                    if w not in reached:
                        reached.add(w)
                        pending.append(w)
            if len(reached) != length:
                valid = False
                break
            unseen -= reached
        if valid:
            return ["Yes"]
    return ["No"]


def _k_shortest_simple_paths(rows):
    n, m, k = rows[0]
    source, target = rows[1]
    points = rows[2:n + 2]
    edges = rows[n + 2:]
    assert 2 <= n <= 13 and n - 1 <= m <= n * (n - 1) // 2 and len(edges) == m
    assert 1 <= k <= 10 and 1 <= source <= n and 1 <= target <= n and source != target
    assert all(-1000 <= x <= 1000 and -1000 <= y <= 1000 for x, y in points)
    graph = [[] for _ in range(n)]
    unique = set()
    for a, b in edges:
        assert 1 <= a < b <= n and (a, b) not in unique
        unique.add((a, b))
        x, y = points[a - 1]
        u, v = points[b - 1]
        w = hypot(x - u, y - v)
        graph[a - 1].append((b - 1, w))
        graph[b - 1].append((a - 1, w))

    distances = []

    def visit(u, seen, distance):
        if u == target - 1:
            distances.append(distance)
            return
        for v, w in graph[u]:
            if v not in seen:
                visit(v, seen | {v}, distance + w)

    visit(source - 1, {source - 1}, 0.0)
    assert distances
    answer = sorted(distances)[:k]
    answer += [-1] * (k - len(answer))
    return [format(value, "g") for value in answer]


def _topological_order_counter(rows):
    n, m = rows[0]
    assert 2 <= n <= 8 and 1 <= m <= n * (n - 1) // 2
    constraints = [tuple(edge) for edge in rows[1:]]
    assert len(constraints) == len(set(constraints)) == m
    assert all(1 <= u <= n and 1 <= v <= n and u != v for u, v in constraints)
    count = 0
    for order in permutations(range(1, n + 1)):
        position = {vertex: i for i, vertex in enumerate(order)}
        count += all(position[u] < position[v] for u, v in constraints)
    assert count > 0
    return [count]


def _palindromic_shortest_paths(data):
    n, *rows = data.split()
    n = int(n)
    assert 1 <= n <= 100 and len(rows) == n
    assert all(len(row) == n and set(row) <= set("-abcdefghijklmnopqrstuvwxyz") for row in rows)

    def distance(start, goal):
        best = n ** 2 * 2
        seen = {(start, goal)}
        pending = [(start, goal, 0)]
        for u, v, prefix in pending:
            if prefix >= best:
                break
            if u == v:
                best = prefix
                break
            if rows[u][v] != "-":
                best = prefix + 1
            for a in range(n):
                color = rows[u][a]
                if color == "-":
                    continue
                for b in range(n):
                    if rows[b][v] == color and (a, b) not in seen:
                        seen.add((a, b))
                        pending.append((a, b, prefix + 2))
        return best if best < n ** 2 * 2 else -1

    return [distance(u, v) for u in range(n) for v in range(n)]


def _two_nearest_source_groups(rows):
    n, m, group_count, source_count = rows[0]
    groups = rows[1]
    sources = rows[2]
    assert len(groups) == n and len(sources) == source_count
    assert all(1 <= group <= group_count for group in groups)
    assert list(sources) == sorted(set(sources)) and all(1 <= v <= n for v in sources)
    assert len(rows) == m + 3
    unique = set()
    inf = 10 ** 30
    distances = [[inf] * n for _ in range(n)]
    for u, v, weight in rows[3:]:
        assert 1 <= u < v <= n and 1 <= weight <= 10 ** 9 and (u, v) not in unique
        unique.add((u, v))
        distances[u - 1][v - 1] = weight
        distances[v - 1][u - 1] = weight
    for v in range(n):
        distances[v][v] = 0
    for k in range(n):
        for i in range(n):
            for j in range(n):
                distances[i][j] = min(distances[i][j], distances[i][k] + distances[k][j])
    result = []
    for v, group in enumerate(groups):
        best = min((distances[v][u - 1] for u in sources if groups[u - 1] != group), default=inf)
        result.append(best if best < inf else -1)
    return result


def _eulerian_trail_decomposition(rows):
    tests, = rows[0]
    assert len(rows) == 1 + 3 * tests
    answers = []
    for i in range(tests):
        n, = rows[1 + 3 * i]
        a = list(rows[2 + 3 * i])
        b = rows[3 + 3 * i]
        assert 2 <= n <= 8 and len(a) == len(b) == n
        assert all(1 <= x <= n for x in (*a, *b))

        def shuffle(position):
            if position == n - 1:
                return sum(x == y for x, y in zip(a, b))
            best = 0
            for j in range(position + 1, n):
                a[position], a[j] = a[j], a[position]
                best = max(best, shuffle(position + 1))
                a[position], a[j] = a[j], a[position]
            return best

        answers.append(shuffle(0))
    return answers


def _incremental_apsp(rows):
    n, m, q = rows[0]
    edges = rows[1:1 + m]
    assert len(rows) == 1 + m + q
    adjacent = [dict() for _ in range(n + 1)]
    for u, v, weight in edges:
        assert 1 <= u < v <= n and 1 <= weight <= 10**9 and v not in adjacent[u]
        adjacent[u][v] = adjacent[v][u] = weight
    removed = set()
    answer = []
    for operation, u, *tail in rows[1 + m:]:
        if operation == 1:
            assert not tail and 1 <= u <= m and u not in removed and len(removed) < 300
            removed.add(u)
            a, b, _ = edges[u - 1]
            del adjacent[a][b], adjacent[b][a]
            continue
        assert operation == 2 and len(tail) == 1 and 1 <= u < tail[0] <= n
        end = tail[0]
        pending = [(0, u)]
        processed = set()
        best = -1
        while pending:
            distance, v = heappop(pending)
            if v in processed:
                continue
            if v == end:
                best = distance
                break
            processed.add(v)
            for neighbor, weight in adjacent[v].items():
                if neighbor not in processed:
                    heappush(pending, (distance + weight, neighbor))
        answer.append(best)
    return answer


def _bipartite_matching_allowed_edges(rows):
    left, right, count = rows[0]
    edges = rows[1:]
    assert len(edges) == count and len(set(map(tuple, edges))) == count
    neighbors = [[] for _ in range(left + 1)]
    for u, v in edges:
        assert 1 <= u <= left and 1 <= v <= right
        neighbors[u].append(v)

    def maximum(ignore_left, ignore_right):
        matching = [0] * (right + 1)

        def augment(u, used):
            for v in neighbors[u]:
                if v == ignore_right or v in used:
                    continue
                used.add(v)
                if matching[v] == 0 or augment(matching[v], used):
                    matching[v] = u
                    return True
            return False

        return sum(augment(u, set()) for u in range(1, left + 1) if u != ignore_left)

    size = maximum(0, 0)
    return ["Yes" if 1 + maximum(u, v) == size else "No" for u, v in edges]
