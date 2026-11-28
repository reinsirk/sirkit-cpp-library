# SPDX-License-Identifier: CC0-1.0
"""Small brute-force checks of structure performance fixtures."""

from fractions import Fraction
from functools import cache
from itertools import combinations, permutations, product
from math import fsum, gcd, hypot
import re


def interval_mex(lines):
    n = int(lines[0])
    assignments = [list(map(int, row.split())) for row in lines[1:n+1]]
    points = list(map(int, lines[n+2].split()))
    assert int(lines[n+1]) == len(points) and sorted(set(points)) == points
    results = []
    for point in points:
        values = {value for first, last, value in assignments if first <= point <= last}
        results.append(next(value for value in range(n + 1) if value not in values))
    return results


def top_k_sliding(lines):
    n, width, k = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(values) == n and 1 <= k <= width <= n <= 200000
    return [sum(sorted(values[left:left+width])[:k]) for left in range(n-width+1)]


def notebook(lines):
    assert int(lines[0]) == len(lines) - 1
    state = []
    pages, results = {}, []
    for row in lines[1:]:
        command, *args = row.split()
        if command == "ADD":
            state.append(int(args[0]))
        elif command == "SAVE":
            pages[int(args[0])] = state.copy()
        elif command == "LOAD":
            state = pages.get(int(args[0]), []).copy()
        elif state:
            assert command == "DELETE"
            state.pop()
        results.append(state[-1] if state else -1)
    return results


def stamps(lines):
    h, w, bh, bw, wh, ww = map(int, lines[0].split())
    grid = [list(map(int, row.split())) for row in lines[1:]]
    assert len(grid) == h and all(len(row) == w for row in grid)
    best = 0
    for y in range(h - bh + 1):
        for x in range(w - bw + 1):
            worst = float("inf")
            for wy in range(h - wh + 1):
                for wx in range(w - ww + 1):
                    kept = sum(grid[row][col] for row in range(y, y + bh) for col in range(x, x + bw)
                               if not (wy <= row < wy + wh and wx <= col < wx + ww))
                    worst = min(worst, kept)
            best = max(best, worst)
    return [best]


def rectangle_min(lines):
    h, w, q = map(int, lines[0].split())
    grid = [list(map(int, row.split())) for row in lines[1:h+1]]
    assert all(len(row) == w for row in grid) and len(lines) == h + q + 2 and lines[-1] == "0 0 0"
    answers = []
    for line in lines[h+1:-1]:
        y1, x1, y2, x2 = map(int, line.split())
        answers.append(min(grid[y][x] for y in range(y1, y2 + 1) for x in range(x1, x2 + 1)))
    return answers



def range_fenwick(lines):
    n, q = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(values) == n and len(lines) == q + 2
    answers = []
    for line in lines[2:]:
        operation, first, last, *args = map(int, line.split())
        assert 1 <= first <= last <= n
        if operation == 1:
            for i in range(first - 1, last):
                values[i] += args[0]
        else:
            assert operation == 2
            answers.append(sum(values[first-1:last]))
    return answers


def pair_xor_order(lines):
    n, k = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(values) == n and 1 <= k <= n * (n - 1) // 2
    actual = sorted(a ^ b for a, b in combinations(values, 2))
    return [actual[k - 1]]


def permutation_order(lines):
    n = int(lines[0])
    permutation = list(map(int, lines[1].split()))
    assert sorted(permutation) == list(range(n))
    code = [sum(other < value for other in permutation[index+1:]) for index, value in enumerate(permutation)]
    unused = list(range(n))
    decoded = [unused.pop(digit) for digit in code]
    return code + decoded

def component_rank(lines):
    n, q = map(int, lines[0].split())
    assert 2 <= n <= 200000 and 1 <= q <= 200000 and len(lines) == q + 1
    group = list(range(n + 1))
    answer = []
    for line in lines[1:]:
        kind, a, b = map(int, line.split())
        assert 1 <= a <= n
        if kind == 1:
            assert 1 <= b <= n and a != b
            first, second = group[a], group[b]
            group = [first if old == second else old for old in group]
        else:
            assert kind == 2 and 1 <= b <= 10
            sizes = [i for i in range(n, 0, -1) if group[i] == group[a]]
            answer.append(sizes[b - 1] if len(sizes) >= b else -1)
    return answer


def monotone_line_frog(lines):
    n, cost = map(int, lines[0].split())
    heights = list(map(int, lines[1].split()))
    assert 2 <= n <= 200000 and 1 <= cost <= 10**12 and len(heights) == n
    assert all(a < b for a, b in zip(heights, heights[1:]))
    assert 1 <= heights[0] and heights[-1] <= 10**6
    best = [0]
    for i in range(1, n):
        best.append(min(best[j] + (heights[i] - heights[j])**2 + cost for j in range(i)))
    return [best[-1]]


def offline_li_chao(lines):
    count = int(lines[0])
    assert 1 <= count <= 300000 and len(lines) == count + 1
    active, result = {}, []
    for line in lines[1:]:
        operation, x, *rest = map(int, line.split())
        assert 0 <= x <= 10**9
        if operation == 1:
            weight, = rest
            assert x not in active and 0 <= weight <= 10**9
            active[x] = weight
        elif operation == 2:
            assert x in active and not rest
            del active[x]
        else:
            assert operation == 3 and x not in active and not rest
            best = max((weight / abs(x - p) for p, weight in active.items()), default=0)
            result.append(f"{best:.12f}")
    return result


def meldable_heap_tree_game(lines):
    n = int(lines[0])
    values = [0, *map(int, lines[1].split())]
    assert 2 <= n <= 200000 and len(values) == n and len(lines) == n + 1
    assert all(1 <= v <= 10**9 for v in values[1:])
    graph = [[] for _ in range(n)]
    for row in lines[2:]:
        u, v = (int(a) - 1 for a in row.split())
        assert 0 <= u < n and 0 <= v < n and u != v
        graph[u].append(v)
        graph[v].append(u)
    parent = [-2] * n
    parent[0] = -1
    order = [0]
    for u in order:
        for v in graph[u]:
            if v != parent[u]:
                assert parent[v] == -2
                parent[v] = u
                order.append(v)
    assert len(order) == n
    for threshold in sorted(set(values[1:]), reverse=True):
        count = [0] * n
        for v in reversed(order):
            count[v] = max(0, count[v] - 1) + (v != 0 and values[v] >= threshold)
            if v:
                count[parent[v]] += count[v]
        if count[0]:
            return [threshold]
    return [0]


def persistent_weighted_branches(lines):
    n, modulus, _ = map(int, lines[0].split())
    history = [[]]
    answers = []
    for line in lines[1:]:
        kind, base, left, right, *weight = map(int, line.split())
        from_vertex = left - 1
        edges = history[base]
        graph = [[] for _ in range(n + 1)]
        for u, v, delta in edges:
            graph[u].append((v, delta))
            graph[v].append((u, -delta))

        seen = {from_vertex: 0}
        queue = [from_vertex]
        for u in queue:
            for v, delta in graph[u]:
                if v not in seen:
                    seen[v] = (seen[u] + delta) % modulus
                    queue.append(v)

        expected = seen.get(right)
        if kind == 0:
            if expected is None:
                edges = [*edges, (from_vertex, right, weight[0])]
            answers.append("YES" if expected is None or expected == weight[0] else "NO")
        else:
            answers.append(expected if expected is not None else "UNKNOWN")
        history.append(edges)
    return answers


def rollback_lazy_brute(lines):
    n, m = map(int, lines[0].split())
    base = list(map(int, lines[1].split()))
    parameters = [list(map(int, line.split())) for line in lines[2:7]]
    outputs = []
    for i, line in enumerate(lines[8:], 1):
        s, q = map(int, line.split())
        values = base.copy()
        y = i
        for j in range(1, q + 1):
            z = (s + j) % m
            left, right = sorted(min(n, max(1, parameters[row][z] ^ y)) for row in (0, 1))
            ql, qr = sorted(min(n, max(1, parameters[row][z] ^ y)) for row in (3, 4))
            x = parameters[2][z] ^ y
            for at in range(left - 1, right):
                values[at] = values[at] | x if z % 2 else values[at] & x
            y = sum(values[ql-1:qr]) % (1 << 30)
        outputs.append(y)
    return outputs


def successor_set_brute(lines):
    q = int(lines[0])
    assert len(lines) == q + 1
    capacity = 1 << 20
    slots = {}
    answers = []
    for line in lines[1:]:
        kind, value = map(int, line.split())
        start = value % capacity
        if kind == 1:
            while start in slots:
                start = (start + 1) % capacity
            slots[start] = value
        else:
            answers.append(slots.get(start, -1))
    return answers


def weighted_minimum_brute(lines):
    n, q = map(int, lines[0].split())
    assert len(lines) == q + 1
    remaining = [[(i + 1) * (j + 1) for j in range(n)] for i in range(n)]
    for line in lines[1:]:
        top, left, bottom, right = map(int, line.split())
        for row in range(top - 1, bottom):
            for col in range(left - 1, right):
                remaining[row][col] = 0
    return [sum(map(sum, remaining)) % 998244353]


def eytzinger_search_brute(lines):
    n, q = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(values) == n and len(lines) == q + 2
    return [sum(value >= int(key) for value in values) for key in lines[2:]]


def range_add_state_ids_brute(lines):
    n, q = map(int, lines[0].split())
    assert len(lines) == q + 1
    values = [0] * n
    first = {tuple(values): 0}
    answers = []
    for step, line in enumerate(lines[1:], 1):
        if line.startswith("!"):
            left, right, delta = map(int, line.split()[1:])
            for index in range(left, right):
                values[index] += delta
            first.setdefault(tuple(values), step)
        else:
            answers.append(first[tuple(values)])
    return answers


def range_add_gcd_brute(lines):
    n, q = int(lines[0]), int(lines[2])
    values = list(map(int, lines[1].split()))
    assert len(values) == n and len(lines) == q + 3
    answers = []
    for line in lines[3:]:
        delta, left, right = map(int, line.split())
        if delta:
            for i in range(left - 1, right):
                values[i] += delta
        else:
            answers.append(gcd(*values[left - 1:right]))
    return answers


def disjoint_subarray_brute(lines):
    n, q = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(values) == n and len(lines) == q + 2
    previous = [0] * (n + 1)
    totals = [0]
    for _ in range(n):
        best = [None] * (n + 1)
        for right in range(1, n + 1):
            candidates = [previous[left] + sum(values[left:right])
                          for left in range(right) if previous[left] is not None]
            if best[right - 1] is not None:
                candidates.append(best[right - 1])
            best[right] = max(candidates) if candidates else None
        totals.append(best[n])
        previous = best
    return [totals[int(line)] for line in lines[2:]]


def binary_range_brute(lines):
    _, q = map(int, lines[0].split())
    bits = list(lines[1])
    assert len(lines) == q + 2
    answers = []
    for line in lines[2:]:
        kind, left, right = map(int, line.split())
        left -= 1
        if kind == 1:
            bits[left:right] = ["1" if bit == "0" else "0" for bit in bits[left:right]]
        else:
            answers.append(max(map(len, "".join(bits[left:right]).split("0"))))
    return answers


def maximum_weight_window_brute(lines):
    n, side = map(int, lines[0].split())
    points = [(2 * x, 2 * y, weight) for x, y, weight in (map(int, line.split()) for line in lines[1:])]
    side *= 2
    assert len(points) == n
    xs = {candidate for x, _, _ in points for candidate in (x - side, x, x + 1)}
    ys = {candidate for _, y, _ in points for candidate in (y - side, y, y + 1)}
    return [max(sum(w for x, y, w in points if left <= x <= left + side and bottom <= y <= bottom + side)
                for left in xs for bottom in ys)]


def point_plane_distance_brute(lines):
    coordinates = [tuple(int(Fraction(x) * 1000000) for x in line.split()) for line in lines[1:]]
    first, points = coordinates[0], coordinates[1:]
    assert len(points) == int(lines[0])
    distances = []
    for a, b, c in combinations(points, 3):
        row = [tuple(x - y for x, y in zip(endpoint, a)) for endpoint in (b, c, first)]
        normal = [row[0][(i + 1) % 3] * row[1][(i + 2) % 3] - row[0][(i + 2) % 3] * row[1][(i + 1) % 3]
                  for i in range(3)]
        distances.append(abs(sum(u * v for u, v in zip(normal, row[2]))) / hypot(*normal))
    return [fsum(distances) / 1000000]


def weighted_l1_statistics_brute(lines):
    values, bias, answers = [], 0, []
    for query in lines[1:]:
        args = list(map(int, query.split()))
        if args[0] == 1:
            values.append(args[1])
            bias += args[2]
        else:
            median = sorted(values)[(len(values) - 1) // 2]
            answers.extend((median, sum(abs(v - median) for v in values) + bias))
    return answers


def l1_origin_line_fit_brute(lines):
    xs, ys = (list(map(int, line.split())) for line in lines[1:])
    assert len(xs) == len(ys) == int(lines[0])
    best = None
    for horizontal, vertical in ((xs, ys), (ys, xs)):
        candidates = {Fraction(-1), Fraction(0), Fraction(1)}
        candidates.update(Fraction(y, x) for x, y in zip(horizontal, vertical) if x and abs(y) <= abs(x))
        value = min(sum(abs(y - x * slope) for x, y in zip(horizontal, vertical)) for slope in candidates)
        best = min(best, value) if best is not None else value
    return [best.numerator, best.denominator]


def weighted_slope_trick_brute(lines):
    m, n = map(int, lines[0].split())
    buyers = sorted(map(int, lines[1].split()))
    supply = sorted(map(int, lines[2].split()))
    assert len(buyers) == m and len(supply) == n
    answers = []
    for capacity in range(1, m + 1):
        costs = [0] + [10**30] * m
        for color in supply:
            for _ in range(capacity):
                for j in range(m - 1, -1, -1):
                    costs[j + 1] = min(costs[j + 1], costs[j] + abs(buyers[j] - color))
        answers.append(costs[m])
    return answers


def narrow_grid_domino_brute(lines):
    width = int(lines[0])
    edges = [(1 << left) | (1 << right) for left, right in (
        [(v, v + 1) for v in range(3 * width) if v % width < width - 1] +
        [(v, v + width) for v in range(2 * width)])]
    answers = 0
    for mask in range(1 << len(edges)):
        occupied = 0
        for index, edge in enumerate(edges):
            if (mask >> index) & 1:
                if occupied & edge:
                    break
                occupied |= edge
        else:
            answers += all(occupied & edge for edge in edges)
    return [answers]


def grid_corner_colorings_brute(lines):
    h, w = map(int, lines[0].split())
    n = h * w
    goal = 1 << (n - 1)
    left = sum(1 << (row * w) for row in range(h))
    right = left << (w - 1)
    count = 0
    for middle in range(1 << (n - 2)):
        black = (middle << 1) | goal | 1
        reached = 1
        while True:
            adjacent = ((reached & ~left) >> 1) | ((reached & ~right) << 1)
            adjacent |= (reached >> w) | (reached << w)
            following = reached | (adjacent & black)
            if following == reached:
                break
            reached = following
        count += bool(reached & goal)
    return [count % 1000000007]


def tournament_transitive_order(lines):
    n = int(lines[0])
    rows = lines[1:]
    assert len(rows) == n and all(len(row) == n and set(row) <= {"0", "1"} for row in rows)
    scores = [row.count("1") for row in rows]
    assert sorted(scores) == list(range(n))
    order = sorted(range(n), key=scores.__getitem__, reverse=True)
    for i, u in enumerate(order):
        assert rows[u][u] == "0"
        assert all(rows[u][v] == "1" and rows[v][u] == "0" for v in order[i + 1:])
    return order


def dfa_minimization_reference_brute(lines):
    n, alphabet, start, queries = map(int, lines[0].split())
    table = [list(map(int, line.split())) for line in lines[1:n + 1]]
    assert len(lines) == n + 1 + queries and all(len(row) == alphabet + 1 for row in table)
    reachable = {start}
    for _ in range(n):
        reachable |= {to for v in reachable for to in table[v][1:]}
    classes = [row[0] for row in table]
    while True:
        lookup = {}
        following = []
        for accept, *next_states in table:
            signature = accept, tuple(classes[v] for v in next_states)
            following.append(lookup.setdefault(signature, len(lookup)))
        if following == classes:
            break
        classes = following
    output = [len({classes[v] for v in reachable})]
    for line in lines[n + 1:]:
        a, b = map(int, line.split())
        output.append(int(a in reachable and b in reachable and classes[a] == classes[b]))
    return output


def labeled_tree_matching_counts_brute(lines):
    limit, kind = map(int, lines[0].split())
    totals = []
    for n in range(2, limit + 1):
        count = 0
        for code in product(range(n), repeat=n - 2):
            degree = [1 + code.count(i) for i in range(n)]
            graph = [[] for _ in range(n)]

            def edge(a, b):
                graph[a].append(b)
                graph[b].append(a)
                degree[a] -= 1
                degree[b] -= 1

            for v in code:
                edge(degree.index(1), v)
            edge(*(i for i in range(n) if degree[i] == 1))

            def winning(vertex, parent):
                return any(not winning(child, vertex) for child in graph[vertex] if child != parent)

            count += not winning(0, -1) if kind == 1 else any(not winning(i, -1) for i in range(n))
        totals.append(count)
    return totals


def trapezoid_count_brute(lines):
    n = int(lines[0])
    points = [tuple(map(int, line.split())) for line in lines[1:]]
    assert len(points) == n and len(set(points)) == n

    def edge(first, second):
        return second[0] - first[0], second[1] - first[1]

    def cross(first, second):
        return first[0] * second[1] - first[1] * second[0]

    assert all(cross(edge(a, b), edge(a, c)) for a, b, c in combinations(points, 3))
    count = 0
    for a, *tail in combinations(points, 4):
        for b, c, d in permutations(tail):
            sides = [edge(a, b), edge(b, c), edge(c, d), edge(d, a)]
            if all(cross(side, sides[(i + 1) % 4]) > 0 for i, side in enumerate(sides)):
                count += not cross(sides[0], sides[2]) or not cross(sides[1], sides[3])
                break
    return [count]


def todd_coxeter_reference_brute(lines):
    alphabet, rules, limit, q = map(int, lines[0].split())
    words = []
    for line in lines[1:]:
        length, *word = map(int, line.split())
        assert length == len(word) and all(0 <= g < alphabet for g in word)
        words.append(word)
    assert len(words) == 2 * (rules + q) and words[1] == []
    relations, queries = words[:2 * rules], words[2 * rules:]
    if rules == 1:
        period_word, empty = relations
        assert alphabet == 1 and period_word and not empty and len(period_word) < limit
        period = len(period_word)
    else:
        assert relations[2::2] == [[g] for g in range(alphabet)] and all(not w for w in relations[1::2])
        period = 1
    return [period, *(int(len(left) % period == len(right) % period)
                      for left, right in zip(queries[::2], queries[1::2]))]


def lexicographic_concatenation_second_brute(lines):
    source = iter(lines)
    out = []
    for _ in range(int(next(source))):
        words = [next(source) for _ in range(int(next(source)))]
        out.append(sorted(map("".join, permutations(words)))[1])
    assert next(source, None) is None
    return out


def lexicographic_string_shuffle_brute(lines):
    count = int(lines[0])
    words = lines[1:]
    assert len(words) == count

    @cache
    def interleave(offsets):
        return min((word[offset] + interleave(offsets[:i] + (offset + 1,) + offsets[i + 1:])
                    for i, (word, offset) in enumerate(zip(words, offsets)) if offset < len(word)), default="")

    return [interleave((0,) * count)]


def bitmask_nfa_wildcard_brute(lines):
    text, x, y = lines

    def matches(word, pattern, offset):
        return offset + len(pattern) <= len(word) and all(
            char == "?" or char == word[offset + i] for i, char in enumerate(pattern))

    count = 0
    for word in product(*(char if char != "?" else "01" for char in text)):
        boundaries = [set() for _ in range(len(word) + 1)]
        boundaries[0].add("x")
        for i, last in enumerate(boundaries):
            if last and matches(word, x, i):
                boundaries[i + len(x)].add("x")
            if "x" in last and matches(word, y, i):
                boundaries[i + len(y)].add("y")
        count += bool(boundaries[-1])
    return [count % 998244353]


def regex_language_edit_distance_brute(lines):
    def atoms(pattern):
        parsed = re.findall(r"([a-z])([?*]?)", pattern)
        assert "".join(letter + quantifier for letter, quantifier in parsed) == pattern
        return [atom for i, atom in enumerate(parsed)
                if atom[1] != "*" or i == 0 or atom != parsed[i - 1]]

    def language(pattern, repeat_limit):
        words = {""}
        for letter, quantifier in pattern:
            counts = {"": (1,), "?": (0, 1), "*": range(repeat_limit + 1)}[quantifier]
            words = {prefix + letter * count for prefix in words for count in counts}
        return words

    def levenshtein(first, second):
        row = list(range(len(second) + 1))
        for i, left in enumerate(first, 1):
            following = [i]
            for j, right in enumerate(second, 1):
                following.append(min(row[j] + 1, following[-1] + 1, row[j - 1] + (left != right)))
            row = following
        return row[-1]

    first, second = map(atoms, lines)
    left_words = language(first, sum(quantifier != "*" for _, quantifier in second))
    right_words = language(second, sum(quantifier != "*" for _, quantifier in first))
    return [min(levenshtein(left, right) for left in left_words for right in right_words)]


def delaunay_triangulation_reference_brute(lines):
    points = sorted({tuple(map(int, line.split())) for line in lines[1:]})
    if len(points) < 2:
        return [0]

    def side(a, b, c):
        return (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])

    if all(side(points[0], points[-1], p) == 0 for p in points):
        return [len(points) - 1]
    boundary = set()
    for a, b in combinations(points, 2):
        turns = [side(a, b, p) for p in points]
        if min(turns) >= 0 or max(turns) <= 0:
            boundary.update((a, b))
    return [3 * len(points) - 3 - len(boundary)]


_MODELS = {
    "delaunay_triangulation_reference": delaunay_triangulation_reference_brute,
    "dfa_minimization_reference": dfa_minimization_reference_brute,
    "labeled_tree_matching_counts": labeled_tree_matching_counts_brute,
    "trapezoid_count": trapezoid_count_brute,
    "todd_coxeter_reference": todd_coxeter_reference_brute,
    "lexicographic_concatenation_second": lexicographic_concatenation_second_brute,
    "lexicographic_string_shuffle": lexicographic_string_shuffle_brute,
    "bitmask_nfa_wildcard": bitmask_nfa_wildcard_brute,
    "regex_language_edit_distance": regex_language_edit_distance_brute,
    "maximum_weight_window": maximum_weight_window_brute,
    "point_plane_distance_sum": point_plane_distance_brute,
    "weighted_l1_statistics_absolute_minima": weighted_l1_statistics_brute,
    "l1_origin_line_fit": l1_origin_line_fit_brute,
    "weighted_slope_trick_selling_cars": weighted_slope_trick_brute,
    "narrow_grid_domino_packings": narrow_grid_domino_brute,
    "grid_corner_connected_colorings": grid_corner_colorings_brute,
    "tournament_path_reference": tournament_transitive_order,
    "successor_set_linear_probing": successor_set_brute,
    "range_add_weighted_minimum": weighted_minimum_brute,
    "eytzinger_search": eytzinger_search_brute,
    "range_add_state_ids": range_add_state_ids_brute,
    "range_add_range_gcd": range_add_gcd_brute,
    "disjoint_subarray_max_sum": disjoint_subarray_brute,
    "permutation_order_roundtrip": permutation_order,
    "pair_xor_order": pair_xor_order,
    "binary_range_statistics": binary_range_brute,
    "interval_set_range_mex": interval_mex,
    "monotone_line_container_frog": monotone_line_frog,
    "meldable_heap_tree_game": meldable_heap_tree_game,
    "offline_li_chao_tree_coins": offline_li_chao,
    "top_k_sum_sliding": top_k_sliding,
    "persistent_stack_notebook": notebook,
    "persistent_weighted_union_find_branching": persistent_weighted_branches,
    "rollback_lazy_segment_tree": rollback_lazy_brute,
    "sparse_table_2d_stamps": stamps,
    "segment_tree_2d_rectangle_minimum": rectangle_min,
    "range_fenwick_tree": range_fenwick,
    "union_find_order_statistics_rank": component_rank,
}


def independent(plan, data):
    values = _MODELS[plan["verify_id"]](data.splitlines())
    return " ".join(map(str, values))
