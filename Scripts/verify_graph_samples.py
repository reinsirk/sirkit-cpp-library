# SPDX-License-Identifier: CC0-1.0
"""非一意な解の出力を独立に検査する。"""
from array import array
from collections import deque
from functools import lru_cache
import re


def _has_no_articulation(graph):
    n = len(graph)
    order = [-1] * n
    low = [0] * n
    order[0] = 0
    count = 1
    root_children = 0
    stack = [(0, -1, iter(graph[0]))]
    while stack:
        u, parent, neighbors = stack[-1]
        v = next(neighbors, -1)
        if v < 0:
            stack.pop()
            if parent >= 0:
                if parent != 0 and low[u] >= order[parent]:
                    return False
                low[parent] = min(low[parent], low[u])
        elif order[v] < 0:
            root_children += u == 0
            low[v] = order[v] = count
            count += 1
            stack.append((v, u, iter(graph[v])))
        elif v != parent:
            low[u] = min(low[u], order[v])
    return count == n and root_children == 1


def check_tree_walk_orientation(_entry, sample, output):
    """各パスの両端と、各木辺で使う方向を独立に調べる。"""
    try:
        n, m, *data = map(int, sample["input"].split())
        claimed, *oriented = map(int, output.split())
    except ValueError:
        return False
    if (not 2 <= n <= 2000 or not 1 <= m <= 2000 or len(data) != 2 * (n - 1 + m)
            or len(oriented) != 2 * m or any(not 1 <= v <= n for v in data)):
        return False
    graph = [[] for _ in range(n + 1)]
    split = 2 * (n - 1)
    for u, v in zip(data[:split:2], data[1:split:2]):
        graph[u].append(v)
        graph[v].append(u)
    parent = [0] * (n + 1)
    depth = [0] * (n + 1)
    parent[1] = 1
    order = [1]
    for u in order:
        for v in graph[u]:
            if parent[v] == 0:
                parent[v] = u
                depth[v] = depth[u] + 1
                order.append(v)
    if len(order) != n:
        return False

    counts = [0] * (n + 1)
    directions = [0] * (n + 1)
    for a, b, u, v in zip(data[split::2], data[split + 1::2], oriented[::2], oriented[1::2]):
        if (u, v) not in ((a, b), (b, a)) or a == b:
            return False
        while u != v:
            if depth[u] >= depth[v]:
                counts[u] += 1
                directions[u] |= 1
                u = parent[u]
            else:
                counts[v] += 1
                directions[v] |= 2
                v = parent[v]
    actual = sum(direction.bit_count() for direction in directions)
    upper = sum(min(2, count) for count in counts)
    return claimed == actual == upper


def check_forest_edge_coloring(_entry, sample, output):
    """各頂点で使う色が重複せず、全体の色数が最大次数か確認する。"""
    try:
        n, *endpoints = map(int, sample["input"].split())
        k, *colors = map(int, output.split())
    except ValueError:
        return False
    if not 1 <= n <= 100000 or len(endpoints) != 2 * (n - 1) or len(colors) != n - 1:
        return False
    if any(not 1 <= vertex <= n for vertex in endpoints):
        return False
    incident = [set() for _ in range(n)]
    for color, u, v in zip(colors, endpoints[::2], endpoints[1::2]):
        if (not 1 <= color <= k or u == v or color in incident[u - 1] or color in incident[v - 1]):
            return False
        incident[u - 1].add(color)
        incident[v - 1].add(color)
    return k == max(map(len, incident))


def check_incident_label_spanning_tree(_entry, sample, output):
    """辺番号、木、全頂点の指定色を確かめる。解は公式出力と異なってよい。"""
    try:
        data = sample["input"].split()
        n, m = map(int, data[:2])
        required = data[-1]
        if (not 2 <= n <= 200000 or not n - 1 <= m <= 200000 or len(data) != 3 * m + 3
                or len(required) != n or any(label not in "RB" for label in required)):
            return False
        edges = [(int(data[i]), int(data[i + 1]), data[i + 2]) for i in range(2, len(data) - 1, 3)]
        if len(set(edges)) != m or any(not 1 <= u < v <= n or label not in ("R", "B") for u, v, label in edges):
            return False
        expected = sample["output"].split()[0]
        actual = output.split()
        if expected == "No":
            return actual == ["No"]
        if (expected != "Yes" or len(actual) != n or actual[0] != "Yes"
                or any(re.fullmatch(r"[+-]?[0-9]+", token) is None for token in actual[1:])):
            return False

        parents = [-1] * (n + 1)
        covered = bytearray(n + 1)
        used = set()

        def root(vertex):
            while parents[vertex] >= 0:
                p = parents[vertex]
                if parents[p] < 0:
                    return p
                parents[vertex] = parents[p]
                vertex = parents[vertex]
            return vertex

        for index in map(int, actual[1:]):
            if not 1 <= index <= m or index in used:
                return False
            used.add(index)
            u, v, label = edges[index - 1]
            a, b = root(u), root(v)
            if a == b:
                return False
            if parents[a] > parents[b]:
                a, b = b, a
            parents[a] += parents[b]
            parents[b] = a
            if label == required[u - 1]:
                covered[u] = 1
            if label == required[v - 1]:
                covered[v] = 1
        return all(covered[1:])
    except (AttributeError, IndexError, KeyError, TypeError, ValueError):
        return False


def check_subcubic_tree_biconnectivity(_entry, sample, output):
    """追加後の関節点がなく、入力ごとの費用が期待値と等しいことを確かめる。"""
    try:
        inputs = iter(map(int, sample["input"].split()))
        reference = iter(map(int, sample["output"].split()))
        tokens = output.split()
        if any(re.fullmatch(r"[+-]?[0-9]+", token) is None for token in tokens):
            return False
        actual = iter(map(int, tokens))
        tests = next(inputs)
        if not 1 <= tests <= 200000:
            return False
        total = 0
        for _ in range(tests):
            n = next(inputs)
            total += n
            if n < 3 or total > 200000:
                return False
            weights = [next(inputs) for _ in range(n)]
            if any(not 1 <= weight <= 10**9 for weight in weights):
                return False
            graph = [set() for _ in range(n)]
            for _ in range(n - 1):
                u, v = next(inputs) - 1, next(inputs) - 1
                if not 0 <= u < n or not 0 <= v < n or u == v or v in graph[u]:
                    return False
                graph[u].add(v)
                graph[v].add(u)
            if any(len(edges) > 3 for edges in graph):
                return False

            costs = []
            for stream in (reference, actual):
                count = next(stream)
                if not 0 <= count <= n * (n - 1) // 2:
                    return False
                cost = 0
                for _ in range(count):
                    u, v = next(stream) - 1, next(stream) - 1
                    if not 0 <= u < n or not 0 <= v < n or u == v or v in graph[u]:
                        return False
                    cost += weights[u] + weights[v]
                    if stream is actual:
                        graph[u].add(v)
                        graph[v].add(u)
                costs.append(cost)
            if costs[0] != costs[1] or not _has_no_articulation(graph):
                return False
        return all(next(stream, None) is None for stream in (inputs, reference, actual))
    except (AttributeError, KeyError, StopIteration, TypeError, ValueError):
        return False


def check_even_outdegree_orientation(_entry, sample, output):
    """単純連結グラフの各辺が一度だけ現れ、すべての出次数が偶数か確かめる。"""
    try:
        source = list(map(int, sample["input"].split()))
        n, m = source[:2]
        actual = output.split()
        if not 2 <= n <= 100000 or not n - 1 <= m <= 100000 or len(source) != 2 + 2 * m:
            return False
        if actual == ["-1"]:
            return m % 2 == 1
        if m % 2 or len(actual) != 2 * m or any(re.fullmatch(r"[+-]?[0-9]+", token) is None for token in actual):
            return False
        remaining = {tuple(sorted((u, v))) for u, v in zip(source[2::2], source[3::2])}
        parity = bytearray(n + 1)
        values = iter(map(int, actual))
        for u, v in zip(values, values):
            if not 1 <= u <= n or not 1 <= v <= n:
                return False
            edge = (u, v) if u < v else (v, u)
            if edge not in remaining:
                return False
            remaining.remove(edge)
            parity[u] ^= 1
        return not remaining and not any(parity)
    except (AttributeError, KeyError, TypeError, ValueError):
        return False


def check_reachability_size_orientation(_entry, sample, output):
    """各辺の矢印を読み、頂点ごとの到達可能数を独立した探索で確認する。"""
    try:
        source = list(map(int, sample["input"].split()))
        n, m = source[:2]
        arrows = output.split()
        if not 1 <= n <= 100 or not 0 <= m <= n * (n - 1) // 2:
            return False
        if len(source) != 2 + 2 * m + n or len(arrows) != m:
            return False
        adjacent = [[] for _ in range(n + 1)]
        edges = set()
        for i, arrow in enumerate(arrows):
            u, v = source[2 + 2 * i:4 + 2 * i]
            if not 1 <= u <= n or not 1 <= v <= n or u == v or arrow not in ("->", "<-"):
                return False
            edge = min(u, v), max(u, v)
            if edge in edges:
                return False
            edges.add(edge)
            if arrow == "<-":
                u, v = v, u
            adjacent[u].append(v)
        for start, expected in enumerate(source[2 + 2 * m:], 1):
            seen = {start}
            pending = [start]
            for u in pending:
                for v in adjacent[u]:
                    if v not in seen:
                        seen.add(v)
                        pending.append(v)
            if len(seen) != expected:
                return False
        return True
    except (AttributeError, KeyError, TypeError, ValueError):
        return False


def check_reachability_chain_cover(_entry, sample, output):
    """最少色数と、同じ色の頂点が到達可能性で比較できることを確かめる。"""
    try:
        data = list(map(int, sample["input"].split()))
        n, m = data[:2]
        tokens = output.split()
        if not 1 <= n <= 7000 or not 0 <= m <= 7000 or len(data) != 2 + 2 * m or len(tokens) != n:
            return False
        if any(re.fullmatch(r"[+-]?[0-9]+", token) is None for token in tokens):
            return False
        colors = list(map(int, tokens))
        if any(not 1 <= color <= n for color in colors) or max(colors) != max(map(int, sample["output"].split())):
            return False
        graph = [[] for _ in range(n)]
        reverse = [[] for _ in range(n)]
        for u, v in zip(data[2::2], data[3::2]):
            if not 1 <= u <= n or not 1 <= v <= n or u == v:
                return False
            graph[u - 1].append(v - 1)
            reverse[v - 1].append(u - 1)

        seen = bytearray(n)
        finish = []
        for root in range(n):
            if seen[root]:
                continue
            seen[root] = 1
            active = [(root, iter(graph[root]))]
            while active:
                u, neighbors = active[-1]
                v = next(neighbors, -1)
                if v == -1:
                    finish.append(u)
                    active.pop()
                elif not seen[v]:
                    seen[v] = 1
                    active.append((v, iter(graph[v])))

        component = [-1] * n
        count = 0
        for root in reversed(finish):
            if component[root] != -1:
                continue
            component[root] = count
            pending = [root]
            for u in pending:
                for v in reverse[u]:
                    if component[v] == -1:
                        component[v] = count
                        pending.append(v)
            count += 1

        diagram = [set() for _ in range(count)]
        groups = {}
        for u, color in enumerate(colors):
            origin = component[u]
            groups.setdefault(color, set()).add(origin)
            diagram[origin].update(component[v] for v in graph[u] if component[v] != origin)
        reachable = [0] * count
        for u in range(count - 1, -1, -1):
            bits = 1 << u
            for v in diagram[u]:
                bits |= reachable[v]
            reachable[u] = bits
        for group in groups.values():
            ordered = sorted(group)
            for u, v in zip(ordered, ordered[1:]):
                if not (reachable[u] >> v) & 1:
                    return False
        return True
    except (AttributeError, KeyError, TypeError, ValueError):
        return False


def check_tree_vertex_at_distance(_entry, sample, output):
    """解があるクエリでは実際の距離を照合し、複数の候補を受け付ける。"""
    try:
        tokens = iter(map(int, sample["input"].split()))
        n = next(tokens)
        if not 2 <= n <= 200000:
            return False
        adjacent = [[] for _ in range(n)]
        for _ in range(n - 1):
            u, v = next(tokens) - 1, next(tokens) - 1
            if not 0 <= u < n or not 0 <= v < n or u == v:
                return False
            adjacent[u].append(v)
            adjacent[v].append(u)
        q = next(tokens)
        actual = output.split()
        reference = list(map(int, sample["output"].split()))
        if not 1 <= q <= 200000 or len(actual) != q or len(reference) != q:
            return False
        if any(re.fullmatch(r"[+-]?[0-9]+", token) is None for token in actual):
            return False

        parent, depth = array("i", [-1]) * n, array("i", [0]) * n
        parent[0] = 0
        pending = [0]
        for u in pending:
            for v in adjacent[u]:
                if parent[v] == -1:
                    parent[v] = u
                    depth[v] = depth[u] + 1
                    pending.append(v)
        if len(pending) != n:
            return False
        jump = [parent]
        for _ in range(1, max(depth).bit_length()):
            last = jump[-1]
            jump.append(array("i", (last[u] for u in last)))

        def distance(a, b):
            total = depth[a] + depth[b]
            if depth[a] < depth[b]:
                a, b = b, a
            steps = depth[a] - depth[b]
            while steps:
                lowest = steps & -steps
                a = jump[lowest.bit_length() - 1][a]
                steps -= lowest
            if a == b:
                return total - 2 * depth[a]
            for ancestors in reversed(jump):
                if ancestors[a] != ancestors[b]:
                    a, b = ancestors[a], ancestors[b]
            return total - 2 * depth[parent[a]]

        for expected, reported in zip(reference, map(int, actual)):
            u, k = next(tokens) - 1, next(tokens)
            if not 0 <= u < n or not 1 <= k <= n:
                return False
            if reported == -1:
                if expected != -1:
                    return False
            elif not 1 <= reported <= n or distance(u, reported - 1) != k:
                return False
        return next(tokens, None) is None
    except (AttributeError, KeyError, StopIteration, TypeError, ValueError):
        return False


def check_maximum_independent_set(entry, sample, output):
    """参照解の最大サイズと、返された頂点集合の独立性を別々に確認する。"""
    try:
        tokens = sample["input"].split()
        actual = output.split()
        expected = sample["output"].split()
        if not expected or any(re.fullmatch(r"[+-]?[0-9]+", x) is None
                               for x in [*tokens, *actual, expected[0]]):
            return False
        data = list(map(int, tokens))
        values = list(map(int, actual))
        if len(data) < 2 or not values:
            return False
        n, m = data[:2]
        size = values[0]
        selected = set(values[1:])
        if (not 1 <= n <= 40 or m < 0 or len(data) != 2 + 2 * m
                or not 0 <= size <= n or size != int(expected[0])
                or len(values) != size + 1 or len(selected) != size
                or any(not 0 <= v < n for v in selected)):
            return False
        return all(0 <= u < n and 0 <= v < n and u != v
                   and not (u in selected and v in selected)
                   for u, v in zip(data[2::2], data[3::2]))
    except (ValueError, TypeError, KeyError, AttributeError):
        return False


def check_unit_step_tree_labels(_entry, sample, output):
    """公式例の可否と、隣接頂点の差・固定値を確かめる。"""
    try:
        data = list(map(int, sample["input"].split()))
        n = data[0]
        if not 1 <= n <= 100000 or len(data) < 2 * n:
            return False
        k = data[2 * n - 1]
        if not 1 <= k <= n or len(data) != 2 * (n + k):
            return False
        fixed = dict(zip(data[2 * n::2], data[2 * n + 1::2]))
        if len(fixed) != k or any(not 1 <= v <= n or not 0 <= x <= 1000000 for v, x in fixed.items()):
            return False
        edges = list(zip(data[1:2 * n - 1:2], data[2:2 * n - 1:2]))
        if any(not 1 <= u <= n or not 1 <= v <= n or u == v for u, v in edges):
            return False
        expected = sample["output"].split()[0]
        actual = output.split()
        if expected == "No":
            return actual == ["No"]
        if expected != "Yes" or len(actual) != n + 1 or actual[0] != "Yes":
            return False
        if any(re.fullmatch(r"[+-]?[0-9]+", token) is None for token in actual[1:]):
            return False
        labels = [0] + list(map(int, actual[1:]))
        return (
            all(labels[v] == value for v, value in fixed.items())
            and all(abs(labels[u] - labels[v]) == 1 for u, v in edges)
        )
    except (IndexError, KeyError, ValueError):
        return False


def check_distinct_topological_orders(_entry, sample, output):
    """公式または独立オラクルの可否と、K 個の相異なる順序を確かめる。"""
    try:
        data = list(map(int, sample["input"].split()))
        n, m, k = data[:3]
        if not (1 <= n <= 100000 and 0 <= m <= 100000 and 1 <= k <= 10 and len(data) == 3 + 2 * m):
            return False
        edges = list(zip(data[3::2], data[4::2]))
        if len(set(edges)) != m or any(not 1 <= u <= n or not 1 <= v <= n or u == v for u, v in edges):
            return False
        impossible = sample["output"].strip() == "-1"
        if output.strip() == "-1":
            return impossible
        if impossible:
            return False
        lines = [line for line in output.splitlines() if line.strip()]
        if len(lines) != k:
            return False
        used = set()
        for line in lines:
            tokens = line.split()
            if len(tokens) != n or any(not token.isascii() or "_" in token for token in tokens):
                return False
            order = tuple(map(int, tokens))
            if order in used:
                return False
            position = [-1] * (n + 1)
            for i, vertex in enumerate(order):
                if not 1 <= vertex <= n or position[vertex] != -1:
                    return False
                position[vertex] = i
            if any(position[u] >= position[v] for u, v in edges):
                return False
            used.add(order)
        return True
    except (AttributeError, KeyError, TypeError, ValueError):
        return False


def check_topological_order(entry, sample, output):
    """頂点の完全な順列と全有向辺を確認し、非一意な正解を許す。"""
    try:
        tokens = sample["input"].split()
        actual = output.split()
        # int()が受理するunderscore・Unicode数字等を出力形式に含めない。
        if any(re.fullmatch(r"[+-]?[0-9]+", token) is None for token in [*tokens, *actual]):
            return False
        data = list(map(int, tokens))
        if len(data) < 2:
            return False
        n, m = data[:2]
        if n < 0 or m < 0 or len(data) != 2 + 2 * m or len(actual) != n:
            return False
        position = [-1] * n
        for index, token in enumerate(actual):
            vertex = int(token)
            if not 0 <= vertex < n or position[vertex] != -1:
                return False
            position[vertex] = index
        return all(0 <= u < n and 0 <= v < n and position[u] < position[v]
                   for u, v in zip(data[2::2], data[3::2]))
    except (ValueError, KeyError, TypeError, AttributeError):
        return False

def check_scc(entry, sample, output):
    try:
        data=list(map(int,sample['input'].split()));n,m=data[:2];arcs=list(zip(data[2::2],data[3::2]));values=list(map(int,output.split()));k=values[0];at=1;ids=[-1]*n;groups=[]
        if not 0<=k<=n:return False
        for c in range(k):
            length=values[at];at+=1
            if length<=0 or at+length>len(values):return False
            group=values[at:at+length];at+=length
            for v in group:
                if not 0<=v<n or ids[v]!=-1:return False
                ids[v]=c
            groups.append(group)
        if at!=len(values)or any(i<0 for i in ids):return False
        g=[[]for _ in range(n)];r=[[]for _ in range(n)]
        for u,v in arcs:
            if ids[u]>ids[v]:return False
            g[u].append(v);r[v].append(u)
        for graph in[g,r]:
            seen=[False]*n
            for c,group in enumerate(groups):
                seen[group[0]]=True;stack=[group[0]];count=0
                while stack:
                    v=stack.pop();count+=1
                    for u in graph[v]:
                        if ids[u]==c and not seen[u]:seen[u]=True;stack.append(u)
                if count!=len(group):return False
        return True
    except (ValueError,IndexError,TypeError):return False

def check_two_sat(entry,sample,output):
    try:
        a=output.split();x=sample['input'].split();n,m=map(int,x[2:4]);expected=sample['output'].split()
        if a==['s','UNSATISFIABLE']:return expected[:2]==a
        if len(a)!=n+4 or a[:3]!=['s','SATISFIABLE','v']or a[-1]!='0':return False
        assignment=list(map(int,a[3:-1]))
        if any(abs(v)!=i+1 for i,v in enumerate(assignment)):return False
        for pos in range(4,len(x),3):
            u,v=int(x[pos]),int(x[pos+1]);left=(assignment[abs(u)-1]>0)==(u>0);right=(assignment[abs(v)-1]>0)==(v>0)
            if not(left or right):return False
        return True
    except(ValueError,IndexError,TypeError):return False

def check_max_flow_grid(entry,sample,output):
    try:
        x=sample['input'].split();h,w=map(int,x[:2]);original=x[2:];a=output.split();count=int(a[0]);grid=a[1:]
        if count!=int(sample['output'].split()[0])or len(grid)!=h or any(len(row)!=w for row in grid):return False
        placed=0
        for i in range(h):
            for j in range(w):
                c=grid[i][j]
                if original[i][j]=='#':
                    if c!='#':return False
                    continue
                if c=='.':continue
                if c=='>':
                    if j+1>=w or grid[i][j+1]!='<':return False
                    placed+=1
                elif c=='v':
                    if i+1>=h or grid[i+1][j]!='^':return False
                    placed+=1
                elif c=='<':
                    if j==0 or grid[i][j-1]!='>':return False
                elif c=='^':
                    if i==0 or grid[i-1][j]!='v':return False
                else:return False
        return placed==count
    except(ValueError,IndexError,TypeError):return False


def check_pruefer_completion(entry, sample, output):
    """出力順や辺の選び方によらず、次数・森・連結を照合する。"""
    try:
        data = list(map(int, sample["input"].split()))
        n, m = data[:2]
        if n <= 0 or len(data) != 2 + n + 2 * m:
            return False
        degrees = data[2:2 + n]
        edges = list(zip(data[2 + n::2], data[3 + n::2]))
        parent = list(range(n))
        size = [1] * n
        used = [0] * n

        def leader(v):
            while parent[v] != v:
                parent[v] = parent[parent[v]]
                v = parent[v]
            return v

        def add(u, v):
            u -= 1; v -= 1
            if not (0 <= u < n and 0 <= v < n):
                return False
            used[u] += 1; used[v] += 1
            a, b = leader(u), leader(v)
            if a == b:
                return False
            if size[a] < size[b]: a, b = b, a
            parent[b] = a; size[a] += size[b]
            return True

        forest = all(add(u, v) for u, v in edges)
        residual = [a - b for a, b in zip(degrees, used)]
        roots = {leader(i) for i in range(n)}
        budgets = {root: 0 for root in roots}
        for i, value in enumerate(residual): budgets[leader(i)] += value
        feasible = (forest and all(value >= 0 for value in residual) and
                    sum(residual) == 2 * (len(roots) - 1) and
                    (len(roots) == 1 or all(value > 0 for value in budgets.values())))
        tokens = list(map(int, output.split()))
        if tokens == [-1]:
            return not feasible
        if not feasible or len(tokens) != 2 * (n - 1 - m):
            return False
        for u, v in zip(tokens[::2], tokens[1::2]):
            if not add(u, v): return False
        return used == degrees and size[leader(0)] == n
    except (ValueError, IndexError, TypeError, KeyError):
        return False


def check_component_sets(entry,sample,output):
    def parse(text,n):
        a=list(map(int,text.split()));k=a[0];pos=1;groups=[]
        if k<0:return None
        for _ in range(k):
            length=a[pos];pos+=1
            if length<=0 or pos+length>len(a):return None
            group=a[pos:pos+length];pos+=length
            if len(set(group))!=length or any(v<0 or v>=n for v in group):return None
            groups.append(tuple(sorted(group)))
        return sorted(groups)if pos==len(a)else None
    try:
        n=int(sample['input'].split()[0]);actual=parse(output,n);expected=parse(sample['output'],n);return actual is not None and actual==expected
    except(ValueError,IndexError,TypeError):return False

def check_eulerian(entry,sample,output,directed):
    try:
        inp=list(map(int,sample['input'].split()));pos=1;a=output.split();expected=sample['output'].split();at=0;et=0
        for _ in range(inp[0]):
            n,m=inp[pos:pos+2];pos+=2;edges=[tuple(inp[pos+2*i:pos+2*i+2])for i in range(m)];pos+=2*m
            expected_yes=expected[et]=='Yes';et+=1
            if expected_yes:et+=(m+1 if n else 0)+m
            state=a[at];at+=1
            if state=='No':
                if expected_yes:return False
                continue
            if state!='Yes' or not expected_yes:return False
            length=m+1 if n else 0;vertices=list(map(int,a[at:at+length]));at+=length;ids=list(map(int,a[at:at+m]));at+=m
            if len(vertices)!=length or len(ids)!=m or sorted(ids)!=list(range(m))or any(v<0 or v>=n for v in vertices):return False
            for i,e in enumerate(ids):
                u,v=edges[e]
                if not((u,v)==(vertices[i],vertices[i+1])or(not directed and(v,u)==(vertices[i],vertices[i+1]))):return False
        return at==len(a)
    except(ValueError,IndexError,TypeError):return False

def check_eulerian_trail_directed(entry,sample,output):return check_eulerian(entry,sample,output,True)
def check_eulerian_trail_undirected(entry,sample,output):return check_eulerian(entry,sample,output,False)
def check_min_cost_flow_grid(entry,sample,output):
    try:
        inp=list(map(int,sample['input'].split()));n,k=inp[:2];values=inp[2:];a=output.split();score=int(a[0]);grid=a[1:]
        if score!=int(sample['output'].split()[0])or len(grid)!=n or any(len(row)!=n for row in grid):return False
        columns=[0]*n;total=0
        for i,row in enumerate(grid):
            count=0
            for j,c in enumerate(row):
                if c=='X':count+=1;columns[j]+=1;total+=values[i*n+j]
                elif c!='.':return False
            if count>k:return False
        return all(c<=k for c in columns)and total==score
    except(ValueError,IndexError,TypeError):return False


def check_bipartite_matching(entry, sample, output):
    """小さいsampleを右頂点集合の部分集合DPでも独立に判定する。"""
    try:
        data=list(map(int,sample['input'].split()));left,right,m=data[:3]
        if len(data)!=3+2*m or not(0<=right<=20):return False
        edges=set(zip(data[3::2],data[4::2]));actual=list(map(int,output.split()))
        if not actual or actual[0]<0 or len(actual)!=1+2*actual[0]:return False
        used_left=set();used_right=set()
        for u,v in zip(actual[1::2],actual[2::2]):
            if not(0<=u<left and 0<=v<right)or(u,v)not in edges or u in used_left or v in used_right:return False
            used_left.add(u);used_right.add(v)
        reachable={0}
        for u in range(left):
            following=set(reachable)
            for state in reachable:
                for v in range(right):
                    if(u,v)in edges and not(state>>v&1):following.add(state|(1<<v))
            reachable=following
        return actual[0]==max(state.bit_count()for state in reachable)
    except(ValueError,IndexError,TypeError):return False


def check_linear_recurrence(entry, sample, output):
    """小標本の全漸化式制約と、独立な掃き出し法による最短次数を検査する。"""
    try:
        data = list(map(int, sample["input"].split()))
        n, sequence = data[0], data[1:]
        if not (0 <= n <= 64) or len(sequence) != n:
            return False
        answer = list(map(int, output.split()))
        if not answer or not 0 <= answer[0] <= n or len(answer) != answer[0] + 1:
            return False
        mod = 998244353
        degree, coefficients = answer[0], answer[1:]
        if any(value < 0 or value >= mod for value in coefficients):
            return False
        if any((sequence[i] - sum(coefficients[j] * sequence[i - j - 1]
                                 for j in range(degree))) % mod for i in range(degree, n)):
            return False

        def consistent(d):
            rows = [[sequence[i - j - 1] % mod for j in range(d)] + [sequence[i] % mod]
                    for i in range(d, n)]
            rank = 0
            for column in range(d):
                pivot = next((i for i in range(rank, len(rows)) if rows[i][column]), None)
                if pivot is None:
                    continue
                rows[rank], rows[pivot] = rows[pivot], rows[rank]
                inverse = pow(rows[rank][column], mod - 2, mod)
                rows[rank] = [x * inverse % mod for x in rows[rank]]
                for i in range(rank + 1, len(rows)):
                    factor = rows[i][column]
                    if factor:
                        rows[i] = [(x - factor * y) % mod for x, y in zip(rows[i], rows[rank])]
                rank += 1
            return all(any(row[:d]) or row[d] == 0 for row in rows)

        return degree == next(d for d in range(n + 1) if consistent(d))
    except (ValueError, IndexError, TypeError, StopIteration):
        return False


def check_longest_common_substring(entry, sample, output):
    """小標本の最長長を独立DPで求め、異なる位置を返す有効解も受理する。"""
    try:
        first, second = sample["input"].split()
        positions = list(map(int, output.split()))
        if len(positions) != 4 or len(first) * len(second) > 1_000_000:
            return False
        a, b, c, d = positions
        if not (0 <= a <= b <= len(first) and 0 <= c <= d <= len(second)):
            return False
        if b - a != d - c or first[a:b] != second[c:d]:
            return False
        longest = 0
        before = [0] * (len(second) + 1)
        for x in first:
            current = [0] * (len(second) + 1)
            for j, y in enumerate(second):
                if x == y:
                    current[j + 1] = before[j] + 1
                    longest = max(longest, current[j + 1])
            before = current
        return b - a == longest
    except (ValueError, IndexError, TypeError):
        return False


def check_matrix_linear_system(entry, sample, output):
    """小標本の解・核基底・基底の独立性・不能判定を独立な掃き出し法で検査する。"""
    try:
        values = list(map(int, sample['input'].split()))
        n, m = values[:2]
        if not (0 <= n <= 64 and 0 <= m <= 64) or len(values) != 2 + n*m + n:
            return False
        mod = 998244353
        matrix = [values[2+i*m:2+(i+1)*m] for i in range(n)]
        rhs = values[2+n*m:]
        def rank(rows, columns):
            rows = [[x % mod for x in row] for row in rows]
            result = 0
            for column in range(columns):
                pivot = next((i for i in range(result, len(rows)) if rows[i][column]), None)
                if pivot is None:
                    continue
                rows[result], rows[pivot] = rows[pivot], rows[result]
                inverse = pow(rows[result][column], mod-2, mod)
                rows[result] = [x*inverse % mod for x in rows[result]]
                for i in range(result+1, len(rows)):
                    factor = rows[i][column]
                    if factor:
                        rows[i] = [(x-factor*y) % mod for x,y in zip(rows[i],rows[result])]
                result += 1
                if result == len(rows):
                    break
            return result
        coefficient_rank = rank(matrix, m)
        augmented_rank = rank([row + [b] for row,b in zip(matrix,rhs)], m+1)
        actual = list(map(int, output.split()))
        if actual == [-1]:
            return coefficient_rank != augmented_rank
        if coefficient_rank != augmented_rank or not actual:
            return False
        dimension = actual[0]
        if dimension != m-coefficient_rank or len(actual) != 1+m*(dimension+1):
            return False
        if any(x < 0 or x >= mod for x in actual[1:]):
            return False
        particular = actual[1:1+m]
        basis = [actual[1+m+i*m:1+m+(i+1)*m] for i in range(dimension)]
        def apply(vector):
            return [sum(x*y for x,y in zip(row,vector)) % mod for row in matrix]
        return (apply(particular) == [x % mod for x in rhs]
                and all(all(x == 0 for x in apply(vector)) for vector in basis)
                and rank(basis,m) == dimension)
    except (ValueError, IndexError, TypeError):
        return False


def validate_binary_linear_system(
    input_text: str, output_text: str
) -> tuple[bool, str]:
    try:
        tokens = input_text.split()
        n = int(tokens[0])
        m = int(tokens[1])
        rows = tokens[2 : 2 + n]
        rhs = tokens[2 + n]
        if (
            n < 1
            or m < 1
            or n > 128
            or m > 128
            or len(tokens) != n + 3
            or len(rows) != n
            or len(rhs) != n
            or any(len(row) != m or set(row) - {"0", "1"} for row in rows)
            or set(rhs) - {"0", "1"}
        ):
            return False, "invalid checker input"

        a = [sum((c == "1") << i for i, c in enumerate(row)) for row in rows]
        b = [int(c) for c in rhs]

        def rank(vectors, width):
            values = vectors[:]
            r = 0
            for col in range(width):
                pivot = next(
                    (j for j in range(r, len(values)) if (values[j] >> col) & 1),
                    None,
                )
                if pivot is None:
                    continue
                values[pivot], values[r] = values[r], values[pivot]
                for j in range(r + 1, len(values)):
                    if (values[j] >> col) & 1:
                        values[j] ^= values[r]
                r += 1
            return r

        coefficient_rank = rank(a, m)
        augmented_rank = rank([x | (y << m) for x, y in zip(a, b)], m + 1)
        exists = coefficient_rank == augmented_rank
        out = output_text.split()
        if out == ["-1"]:
            return not exists, "no-solution claim" if exists else ""
        if not exists:
            return False, "inconsistent system"
        if not out:
            return False, "empty output"
        dimension = int(out[0])
        if dimension != m - coefficient_rank or len(out) != 2 + dimension:
            return False, "dimension/output length"
        strings = out[1:]
        if any(len(s) != m or set(s) - {"0", "1"} for s in strings):
            return False, "binary vector length"
        vectors = [sum((c == "1") << i for i, c in enumerate(s)) for s in strings]
        if any(
            ((row & vectors[0]).bit_count() & 1) != value
            for row, value in zip(a, b)
        ):
            return False, "particular"
        if any((row & v).bit_count() & 1 for row in a for v in vectors[1:]):
            return False, "kernel"
        if rank(vectors[1:], m) != dimension:
            return False, "dependent basis"
        return True, ""
    except (IndexError, TypeError, ValueError):
        return False, "malformed output"


def check_binary_linear_system(entry: dict, sample: dict, output: str) -> bool:
    return validate_binary_linear_system(sample["input"], output)[0]


def validate_general_matching(input_text, output_text):
    try:
        data = list(map(int, input_text.split()))
        if len(data) < 2:
            return False, "input header"
        n, m = data[:2]
        if not 0 <= n <= 20 or m < 0 or len(data) != 2 + 2 * m:
            return False, "input size"
        neighbors = [0] * n
        edges = set()
        for u, v in zip(data[2::2], data[3::2]):
            if not (0 <= u < n and 0 <= v < n):
                return False, "input vertex"
            if u != v:
                edges.add((min(u, v), max(u, v)))
                neighbors[u] |= 1 << v
                neighbors[v] |= 1 << u

        actual = list(map(int, output_text.split()))
        if not actual or not 0 <= actual[0] <= n // 2:
            return False, "matching size"
        if len(actual) != 1 + 2 * actual[0]:
            return False, "output size"
        used = set()
        for u, v in zip(actual[1::2], actual[2::2]):
            if not (0 <= u < n and 0 <= v < n) or u == v:
                return False, "output vertex"
            if u in used or v in used:
                return False, "reused vertex"
            if (min(u, v), max(u, v)) not in edges:
                return False, "nonedge"
            used.update((u, v))

        @lru_cache(maxsize=None)
        def best(mask):
            if not mask:
                return 0
            bit = mask & -mask
            vertex = bit.bit_length() - 1
            remaining = mask ^ bit
            answer = best(remaining)
            candidates = remaining & neighbors[vertex]
            while candidates:
                other = candidates & -candidates
                answer = max(answer, 1 + best(remaining ^ other))
                candidates ^= other
            return answer

        if actual[0] != best((1 << n) - 1):
            return False, "not maximum"
        return True, ""
    except (ValueError, TypeError, IndexError):
        return False, "parse"


def check_general_matching(entry, sample, output):
    return validate_general_matching(sample["input"], output)[0]


def check_modular_square_root(entry, sample, output):
    """素数法を前提とするLC形式。任意の標準剰余の根と不能を独立に確認する。"""
    try:
        data = list(map(int, sample['input'].split()))
        count = data[0]
        answers = list(map(int, output.split()))
        if not 0 <= count <= 100000 or len(data) != 1 + 2*count or len(answers) != count:
            return False
        for index, root in enumerate(answers):
            value, prime = data[1+2*index:3+2*index]
            if not 2 <= prime <= (1 << 64)-1 or (prime != 2 and prime % 2 == 0):
                return False
            value %= prime
            if root == -1:
                if prime == 2 or value == 0 or pow(value, (prime-1)//2, prime) != prime-1:
                    return False
            elif not 0 <= root < prime or root*root % prime != value:
                return False
        return True
    except (ValueError, TypeError, IndexError):
        return False


def check_static_range_mode(entry, sample, output):
    """LC Static Range Mode Query形式。各区間で返した値の出現回数と、それが最大であることを確認する。"""
    try:
        tokens = sample["input"].split()
        actual = output.split()
        expected = sample["output"].split()
        if any(re.fullmatch(r"[+-]?[0-9]+", x) is None for x in [*tokens, *actual, *expected]):
            return False
        data = list(map(int, tokens))
        if len(data) < 2:
            return False
        n, count = data[:2]
        values = data[2:2 + n]
        ranges = data[2 + n:]
        if (not 1 <= n <= 100000 or not 0 <= count <= 100000 or len(values) != n
                or len(ranges) != 2 * count or len(actual) != 2 * count or len(expected) != 2 * count
                or any(not 0 <= value <= 10**9 for value in values)):
            return False
        answers = list(map(int, actual))
        references = list(map(int, expected))
        for index in range(count):
            left, right = ranges[2 * index:2 * index + 2]
            if not 0 <= left < right <= n:
                return False
            frequency = {}
            for value in values[left:right]:
                frequency[value] = frequency.get(value, 0) + 1
            best = max(frequency.values())
            mode, times = answers[2 * index:2 * index + 2]
            if references[2 * index + 1] != best or times != best or frequency.get(mode, 0) != best:
                return False
        return True
    except (ValueError, TypeError, KeyError, AttributeError):
        return False


def sample_tree_data(sample, output):
    """整数出力を読み、公式入力例の最適値と照合する。"""
    tokens = output.split()
    if any(re.fullmatch(r"[+-]?[0-9]+", token) is None for token in tokens):
        raise ValueError("整数以外の出力")
    inputs = list(map(int, sample["input"].split()))
    actual_cost, *actual = map(int, tokens)
    expected_cost = int(sample["output"].split()[0])
    if actual_cost != expected_cost:
        raise ValueError("目的値が一致しない")
    return inputs, actual, expected_cost


def sample_tree_cost(n, edges, cost):
    """n−1本の無閉路な辺を確認し、重みの和を照合する。"""
    parent = list(range(n))

    def root(vertex):
        while parent[vertex] != vertex:
            parent[vertex] = parent[parent[vertex]]
            vertex = parent[vertex]
        return vertex

    weight = 0
    count = 0
    for u, v, w in edges:
        if not 0 <= u < n or not 0 <= v < n:
            return False
        a, b = root(u), root(v)
        if a == b:
            return False
        parent[a] = b
        count += 1
        weight += w
    return count == n - 1 and weight == cost


def check_manhattan_mst(_entry, sample, output):
    try:
        inputs, actual, expected_cost = sample_tree_data(sample, output)
        n = inputs[0]
        if n < 1 or len(inputs) != 1 + 2 * n or len(actual) != 2 * (n - 1):
            return False
        if any(not 0 <= v < n for v in actual):
            return False
        points = list(zip(inputs[1::2], inputs[2::2]))
        return sample_tree_cost(n, ((u, v, abs(points[u][0] - points[v][0]) + abs(points[u][1] - points[v][1]))
                                    for u, v in zip(actual[::2], actual[1::2])), expected_cost)
    except (IndexError, ValueError, TypeError, KeyError, AttributeError):
        return False


def check_minimum_spanning_tree(_entry, sample, output):
    try:
        inputs, actual, expected_cost = sample_tree_data(sample, output)
        n, m = inputs[:2]
        if n < 1 or m < 0 or len(inputs) != 2 + 3 * m or len(actual) != n - 1:
            return False
        if any(not 0 <= edge < m for edge in actual):
            return False
        edges = list(zip(inputs[2::3], inputs[3::3], inputs[4::3]))
        return sample_tree_cost(n, (edges[i] for i in actual), expected_cost)
    except (IndexError, ValueError, TypeError, KeyError, AttributeError):
        return False


def check_tree_diameter(_entry, sample, output):
    try:
        inputs, actual, expected_cost = sample_tree_data(sample, output)
        n = inputs[0]
        count, *path = actual
        if (n < 1 or len(inputs) != 1 + 3 * (n - 1) or not 1 <= count <= n
                or len(path) != count or len(set(path)) != count or any(not 0 <= v < n for v in path)):
            return False
        weights = {}
        for u, v, w in zip(inputs[1::3], inputs[2::3], inputs[3::3]):
            weights[u, v] = weights[v, u] = w
        return sum(weights[u, v] for u, v in zip(path, path[1:])) == expected_cost
    except (IndexError, ValueError, TypeError, KeyError, AttributeError):
        return False
