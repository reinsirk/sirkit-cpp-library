# SPDX-License-Identifier: CC0-1.0
"""Slow direct oracles for the small math performance fixtures."""

from collections import Counter, deque
from decimal import Decimal
from fractions import Fraction
from functools import cache
from functools import lru_cache
from itertools import combinations, permutations, product
from math import comb, factorial, gcd, lcm


def _floor_moments(plan, data):
    rows = data.splitlines()
    queries = int(rows[0])
    dimensions = plan["dimensions"]
    assert 1 <= queries <= 100000
    assert queries == dimensions["q"] == len(rows) - 1
    answers = []
    for row in rows[1:]:
        n, modulus, slope, first, second = map(int, row.split())
        assert (n, modulus, slope) == tuple(dimensions[name] for name in ("n", "m", "a"))
        assert 1 <= n <= 1000000 and 1 <= modulus <= 1000000
        assert all(0 <= value < modulus for value in (slope, first, second))
        if plan["scenario"] == "fibonacci_full_period":
            assert n == modulus and gcd(slope, modulus) == 1
        else:
            assert n < modulus and slope == modulus - 1
        answers.append(str(sum(((slope * i + first) % modulus) * ((slope * i + second) % modulus)
                               for i in range(n))))
    return " ".join(answers)


def _floor_path(plan, data):
    rows = data.splitlines()
    queries = int(rows[0])
    dimensions = plan["dimensions"]
    assert 1 <= queries <= 100
    assert queries == dimensions["q"] == len(rows) - 1
    answers = []
    for row in rows[1:]:
        n, modulus, slope, intercept, base, ring = map(int, row.split())
        assert (n, modulus, slope, ring) == tuple(dimensions[name] for name in ("n", "m", "a", "r"))
        assert 1 <= n <= 10**9 and 1 <= modulus <= 10**9 and 1 <= ring <= 10**9
        assert 0 <= slope < modulus and 0 <= intercept < modulus and 1 <= base < ring
        if plan["scenario"] == "weighted_fibonacci_periods":
            assert n % modulus == 0 and gcd(slope, modulus) == 1 and gcd(base, ring) == 1
        else:
            assert n < modulus and slope == modulus - 1 and 1 < gcd(base, ring) < ring
        answers.append(str(sum(pow(base, (slope * i + intercept) % modulus, ring) for i in range(n)) % ring))
    return " ".join(answers)


def _relaxed(plan, data):
    prime = 998244353
    n, *weights = map(int, data.split())
    assert 1 <= n <= 200000 and n == plan["dimensions"]["n"] == len(weights)
    assert all(0 <= weight < prime for weight in weights)
    values = [1]
    accumulated = 1
    for index, weight in enumerate(weights, start=1):
        values.append(weight * accumulated % prime)
        ordinary = sum(values[j] * values[index - j] for j in range(index + 1))
        accumulated = (accumulated + ordinary) % prime
    return " ".join(map(str, values[1:]))


def _mixed_radix(plan, data):
    n, *permutation = map(int, data.split())
    assert 1 <= n <= 100000 and n == plan["dimensions"]["n"] == len(permutation)
    assert sorted(permutation) == list(range(1, n + 1))
    answer = 1 + sum(sum(value < current for value in permutation[i + 1:]) * factorial(n - i - 1)
                     for i, current in enumerate(permutation))
    return str(answer)


def _adjacent_merge(plan, data):
    n, *weights = map(int, data.split())
    assert n == plan["dimensions"]["n"] == len(weights)
    assert 3 <= n <= 200000 and all(0 <= weight <= 10**9 for weight in weights)
    prefix = [0]
    for weight in weights:
        prefix.append(prefix[-1] + weight)
    dp = [[0] * (n + 1) for _ in range(n)]
    for left in range(n - 2, -1, -1):
        for right in range(left + 2, n + 1):
            dp[left][right] = prefix[right] - prefix[left] + min(
                dp[left][mid] + dp[mid][right] for mid in range(left + 1, right))
    return str(dp[0][n])


def _cartesian_sums(plan, data):
    lines = data.splitlines()
    x, y, z, k = map(int, lines[0].split())
    groups = [list(map(int, row.split())) for row in lines[1:]]
    assert [len(group) for group in groups] == [x, y, z]
    assert x == y == z == plan["dimensions"]["n"] and k == plan["dimensions"]["k"]
    assert 1 <= x <= 1000 and 1 <= k <= min(3000, x * y * z)
    assert all(1 <= value <= 10**10 for group in groups for value in group)
    return " ".join(map(str, sorted(map(sum, product(*groups)), reverse=True)[:k]))


def _knapsack_by_value(plan, data):
    lines = data.splitlines()
    n, bound = map(int, lines[0].split())
    items = [tuple(map(int, row.split())) for row in lines[1:]]
    assert len(items) == n == plan["dimensions"]["n"] and 1 <= n <= 100 and 1 <= bound <= 10**9
    assert all(1 <= weight <= bound and 1 <= value <= 1000 for weight, value in items)
    subsets = [(0, 0)]
    for weight, value in items:
        subsets += [(old_weight + weight, old_value + value) for old_weight, old_value in subsets
                    if old_weight + weight <= bound]
    return str(max(value for _, value in subsets))


def _histogram_rectangle(plan, data):
    n, *heights = map(int, data.split())
    assert n == plan["dimensions"]["n"] == len(heights) and 1 <= n <= 10000
    assert all(1 <= height <= 100000 for height in heights)
    best = 0
    for left, height in enumerate(heights):
        low = height
        for right in range(left, n):
            low = min(low, heights[right])
            best = max(best, low * (right - left + 1))
    return str(best)


def _subarray_extrema(plan, data):
    n, *values = map(int, data.split())
    assert n == plan["dimensions"]["n"] == len(values) and 1 <= n <= 200000
    assert sorted(values) == list(range(1, n + 1))
    answer = 0
    for left, value in enumerate(values):
        current = value
        for right in range(left, n):
            current = min(current, values[right])
            answer += current
    return str(answer)


def _bounded_compositions(plan, data):
    n, k, *bounds = map(int, data.split())
    assert n == plan["dimensions"]["n"] == len(bounds) and k == plan["dimensions"]["k"]
    assert 1 <= n <= 100 and 0 <= k <= 100000 and all(0 <= bound <= k for bound in bounds)
    counts = [1] + [0] * k
    for bound in bounds:
        counts = [sum(counts[total - value] for value in range(min(total, bound) + 1)) % 1000000007
                  for total in range(k + 1)]
    return str(counts[-1])


def _sparse_knapsack(plan, data):
    lines = data.splitlines()
    n, capacity = map(int, lines[0].split())
    items = [tuple(map(int, row.split())) for row in lines[1:]]
    assert len(items) == n == plan["dimensions"]["n"] and 2 <= n <= 5000 and 1 <= capacity <= 2**31
    assert all(1 <= weight <= 2**31 and 1 <= value <= 2**31 for value, weight in items)
    subsets = [(0, 0)]
    for value, weight in items:
        subsets += [(old_weight + weight, old_value + value) for old_weight, old_value in subsets
                    if old_weight + weight <= capacity]
    return str(max(value for _, value in subsets))


def _digit_dp(plan, data):
    bound, raw_divisor = data.split()
    divisor = int(raw_divisor)
    assert divisor == plan["dimensions"]["divisor"] and 1 <= divisor <= 100
    assert bound[0] != "0" and len(bound) == plan["dimensions"]["digits"] <= 10000
    count = sum(sum(map(int, str(number))) % divisor == 0 for number in range(1, int(bound) + 1))
    return str(count % 1000000007)


def _maximum_submatrix(plan, data):
    lines = data.splitlines()
    rows, columns = map(int, lines[0].split())
    assert rows == plan["dimensions"]["rows"] and columns == plan["dimensions"]["columns"]
    assert 1 <= rows <= 500 and 1 <= columns <= 500
    grid = [tuple(map(int, line.split())) for line in lines[1:]]
    assert len(grid) == rows and all(len(row) == columns for row in grid)
    assert all(-10000 <= value <= 10000 for row in grid for value in row)
    return str(max(sum(grid[i][j] for i in range(top, bottom) for j in range(left, right))
                   for top in range(rows) for bottom in range(top + 1, rows + 1)
                   for left in range(columns) for right in range(left + 1, columns + 1)))


def _exact_cover(plan, data):
    lines = data.splitlines()
    tile_count, rows, columns = map(int, lines[0].split())
    assert tile_count == plan["dimensions"]["tiles"] and 1 <= tile_count <= 7
    assert rows == columns == plan["dimensions"]["side"] and 1 <= rows <= 10
    pieces = [tuple(map(int, line.split())) for line in lines[1:]]
    assert len(pieces) == tile_count and all(1 <= a <= 10 and 1 <= b <= 10 for a, b in pieces)

    full = (1 << (rows * columns)) - 1

    @cache
    def place(occupied, used):
        unfilled = full ^ occupied
        if not unfilled:
            return True
        top, left = divmod((unfilled & -unfilled).bit_length() - 1, columns)
        for i, (a, b) in enumerate(pieces):
            if (used >> i) & 1:
                continue
            for height, width in {(a, b), (b, a)}:
                if top + height > rows or left + width > columns:
                    continue
                shape = sum(((1 << width) - 1) << (row * columns + left)
                            for row in range(top, top + height))
                if not occupied & shape and place(occupied | shape, used | (1 << i)):
                    return True
        return False

    return "Yes" if place(0, 0) else "No"


def _matrix_permanent(plan, data):
    lines = data.splitlines()
    n = int(lines[0])
    assert n == plan["dimensions"]["n"] and 1 <= n <= 21
    matrix = [tuple(map(int, line.split())) for line in lines[1:]]
    assert len(matrix) == n and all(len(row) == n and set(row) <= {0, 1} for row in matrix)
    count = sum(all(matrix[i][column] for i, column in enumerate(permutation))
                for permutation in permutations(range(n)))
    return str(count % 1000000007)


def _nonadjacent_selection(plan, data):
    n, red, *weights = map(int, data.split())
    assert n == plan["dimensions"]["n"] and 2 <= n <= 200000 and 1 <= red < n
    assert len(weights) == n - 1 and all(1 <= value <= 10**9 for value in weights)
    answer = 0
    for colors in combinations(range(n), red):
        red_positions = set(colors)
        reward = sum(value for i, value in enumerate(weights)
                     if (i in red_positions) != (i + 1 in red_positions))
        answer = max(answer, reward)
    return str(answer)


def _hamiltonian_route(plan, data):
    lines = data.splitlines()
    n = int(lines[0])
    points = [tuple(map(int, line.split())) for line in lines[1:]]
    assert n == plan["dimensions"]["n"] == len(points) == len(set(points)) and 2 <= n <= 17
    assert all(len(point) == 3 and all(-10**6 <= value <= 10**6 for value in point) for point in points)

    def edge(u, v):
        x, y, z = points[u]
        a, b, c = points[v]
        return abs(x - a) + abs(y - b) + max(0, c - z)

    answer = min(edge(0, order[0]) + sum(edge(u, v) for u, v in zip(order, order[1:])) + edge(order[-1], 0)
                 for order in permutations(range(1, n)))
    return str(answer)


def _disk_lattice_points(plan, data):
    x_text, y_text, radius_text = data.split()
    unit = 10000
    numbers = [Decimal(token) * unit for token in (x_text, y_text, radius_text)]
    x, y, radius = map(int, numbers)
    assert [x, y, radius] == numbers and 0 < radius <= 10**9 and max(abs(x), abs(y)) <= 10**9
    reach = radius // unit + 2
    answer = sum((px * unit - x)**2 + (py * unit - y)**2 <= radius**2
                 for px in range(x // unit - reach, x // unit + reach + 1)
                 for py in range(y // unit - reach, y // unit + reach + 1))
    return str(answer)


def _linear_matroid_parity(plan, data):
    lines = data.splitlines()
    assert lines[0] == "1"
    n, m = map(int, lines[1].split())
    capacities = list(map(int, lines[2].split()))
    edges = [tuple(map(int, line.split())) for line in lines[3:]]
    assert n == plan["dimensions"]["n"] == len(capacities) and 2 <= n <= 500
    assert len(set(edges)) == len(edges) == m and all(1 <= u < v <= n for u, v in edges)
    assert sum(capacities) <= 500 and all(1 <= limit < n for limit in capacities)

    def search(i, degrees):
        if i == m:
            return 0
        answer = search(i + 1, degrees)
        u, v = edges[i]
        if degrees[u - 1] < capacities[u - 1] and degrees[v - 1] < capacities[v - 1]:
            degrees[u - 1] += 1
            degrees[v - 1] += 1
            answer = max(answer, 1 + search(i + 1, degrees))
            degrees[u - 1] -= 1
            degrees[v - 1] -= 1
        return answer

    return str(search(0, [0] * n))


def _xor_two_independent_sets(plan, data):
    lines = data.splitlines()
    tests = int(lines[0])
    assert tests == plan["dimensions"]["t"] and 1 <= tests <= 10 and len(lines) == 1 + 2 * tests
    answers = []
    for case in range(tests):
        n, target = map(int, lines[1 + 2 * case].split())
        vectors = list(map(int, lines[2 + 2 * case].split()))
        assert n == plan["dimensions"]["n"] == len(vectors) and 1 <= n <= 200
        assert 1 <= target < 2**60 and all(0 <= v < 2**60 for v in vectors)

        @cache
        def play(remaining, alice):
            if not remaining:
                possible = {0}
                for i, value in enumerate(vectors):
                    if (alice >> i) & 1:
                        possible |= {old ^ value for old in possible}
                return target in possible
            alice_turn = (n - remaining.bit_count()) % 2 == 0
            children = (play(remaining ^ (1 << i), alice | ((1 << i) if alice_turn else 0))
                        for i in range(n) if (remaining >> i) & 1)
            return any(children) if alice_turn else all(children)

        answers.append("Alice" if play((1 << n) - 1, 0) else "Bob")
    return "\n".join(answers)


def _subtraction_game(plan, data):
    count, stones, *moves = map(int, data.split())
    assert count == plan["dimensions"]["moves"] == len(moves) and 1 <= count <= 100
    assert stones == plan["dimensions"]["stones"] and 1 <= stones <= 100000
    assert moves == sorted(set(moves)) and 1 <= moves[0] <= moves[-1] <= stones
    win = [False] * (stones + 1)
    for n in range(1, stones + 1):
        win[n] = any(not win[n - taken] for taken in moves if taken <= n)
    return "First" if win[stones] else "Second"


def _alien_trick(plan, data):
    tests, sequence, extra_text = data.split()
    assert tests == "1" and sequence.isascii() and sequence.isupper()
    extra = int(extra_text)
    assert extra == plan["dimensions"]["extra"] and 1 <= extra <= 10
    n = len(sequence)
    assert n == plan["dimensions"]["n"] and 3 <= n <= 300000
    need = extra + sequence.count("ABC")
    if need > n // 3:
        return "-1"
    impossible = n + 1
    best = [[0] + [impossible] * need for _ in range(n + 1)]
    for end in range(3, n + 1):
        change = sum(a != b for a, b in zip(sequence[end - 3:end], "ABC"))
        for count in range(1, need + 1):
            best[end][count] = min(best[end - 1][count], best[end - 3][count - 1] + change)
    return str(best[n][need])


def _permutation_comparisons(plan, data):
    length, pattern = data.split()
    n = int(length)
    assert n == plan["dimensions"]["n"] and 2 <= n <= 3000
    assert len(pattern) == n - 1 and set(pattern) <= {"<", ">"}
    counts = [1]
    modulus = 1_000_000_007
    for sign in pattern:
        total = sum(counts) % modulus
        lower = 0
        next_counts = []
        for count in counts:
            next_counts.append(lower if sign == "<" else (total - lower) % modulus)
            lower = (lower + count) % modulus
        next_counts.append(total if sign == "<" else 0)
        counts = next_counts
    return str(sum(counts) % modulus)


def _lcm_spanning_tree(plan, data):
    n, *labels = map(int, data.split())
    assert n == len(labels) == plan["dimensions"]["n"] and 2 <= n <= 200000
    assert all(1 <= label <= 200000 for label in labels)
    remaining = set(range(1, n))
    best = [lcm(label, labels[0]) for label in labels]
    cost = 0
    while remaining:
        u = min(remaining, key=best.__getitem__)
        cost += best[u]
        remaining.remove(u)
        for v in remaining:
            best[v] = min(best[v], lcm(labels[u], labels[v]))
    return str(cost)


def _modular_shift_minimum(plan, data):
    n, modulus, step, count, *offsets = map(int, data.split())
    expected = tuple(plan["dimensions"][key] for key in ("n", "m", "c", "k"))
    assert (n, modulus, step, count) == expected
    assert n == len(offsets) and 1 <= n <= 100000 and 1 <= modulus <= 10**9 and 1 <= count <= 10**9
    assert 0 <= step < modulus and all(0 <= offset < modulus for offset in offsets)
    return str(sum(min((step * k + offset) % modulus for offset in offsets) for k in range(count)))


def _modular_shift_mex(plan, data):
    tokens = list(map(int, data.split()))
    n, modulus, step, count = tokens[:4]
    q = tokens[4 + n]
    expected = tuple(plan["dimensions"][key] for key in ("n", "m", "c", "k", "q"))
    assert (n, modulus, step, count, q) == expected
    assert 1 <= n <= 200000 and 1 <= q <= 200000 and 1 <= modulus <= 10**9 and 1 <= count <= 10**9
    assert 0 <= step < modulus and 5 + n + 2 * q == len(tokens)
    offsets = tokens[4:4 + n]
    assert all(0 <= offset < modulus for offset in offsets)
    lines = []
    for i in range(q):
        index, replacement = tokens[5 + n + 2 * i:7 + n + 2 * i]
        assert 1 <= index <= n and 0 <= replacement < modulus
        offsets[index - 1] = replacement
        total = 0
        for k in range(count):
            shifted = {(step * k + offset) % modulus for offset in offsets}
            mex = 0
            while mex in shifted:
                mex += 1
            total += mex
        lines.append(str(total))
    return "\n".join(lines)


def _modular_shift_sums(plan, data):
    tokens = list(map(int, data.split()))
    n = tokens[0]
    q_index = 1 + 2 * n
    count = tokens[q_index]
    assert n == plan["dimensions"]["n"] and 1 <= n <= 500000
    assert count == plan["dimensions"]["queries"] == len(tokens) - q_index - 1 and 1 <= count <= 200000
    terms = list(zip(tokens[1:q_index:2], tokens[2:q_index:2]))
    queries = tokens[q_index + 1:]
    assert all(0 <= offset < modulus <= 500000 for offset, modulus in terms)
    assert max(queries) == plan["dimensions"]["upper"] and all(0 <= x <= 500000 for x in queries)
    return "\n".join(str(sum((x + offset) % modulus for offset, modulus in terms)) for x in queries)


def _tridiagonal_matrix_power(plan, data):
    n, m = map(int, data.split())
    assert (n, m) == (plan["dimensions"]["n"], plan["dimensions"]["m"])
    assert 2 <= n <= 10**9 and 1 <= m <= 125000
    feasible = [order for order in permutations(range(1, n + 1))
                if all(value <= i + m for i, value in enumerate(order, 1))]
    return str(sum(all(a != b for a, b in zip(first, second)) for first in feasible for second in feasible) % 998244353)


def _matrix_pencil_determinant(plan, data):
    n, m, *values = map(int, data.split())
    assert n == plan["dimensions"]["n"] and 2 <= n <= 100 and len(values) == 2 * m
    given = {(u - 1, v - 1) for u, v in zip(values[::2], values[1::2])}
    assert len(given) == m and all(0 <= u < v < n for u, v in given)
    edges = list(combinations(range(n), 2))
    best, count = (n * n, 0), 0
    for edge_count in range(n):
        for chosen in combinations(edges, edge_count):
            if sum(edge not in given for edge in chosen) > 1:
                continue
            labels = list(range(n))
            for u, v in chosen:
                old, new = labels[v], labels[u]
                labels = [new if label == old else label for label in labels]
            score = (sum(a != b for a in labels for b in labels), edge_count)
            if score < best:
                best, count = score, 0
            count += score == best
    return f"{best[0]}\n{count % 998244353}"


def _adjacent_swap_parity(plan, data):
    test, n, group, *values = map(int, data.split())
    assert test == 1 and n == plan["dimensions"]["n"] and group == plan["dimensions"]["k"]
    assert 2 <= n <= 300000 and 1 <= group <= 10**9 and len(values) == 2 * n
    assert all(1 <= value <= n for value in values)
    initial, result = tuple(values[:n]), tuple(values[n:])
    assert sorted(initial) == sorted(result)
    start = (initial, 0)
    reached = {start: 0}
    queue = deque([start])
    while queue:
        sequence, parity = queue.popleft()
        steps = reached[sequence, parity] + 1
        for i in range(n - 1):
            following = sequence[:i] + (sequence[i + 1], sequence[i]) + sequence[i + 2:]
            state = (following, 1 - parity)
            if state not in reached:
                reached[state] = steps
                queue.append(state)
    answer = n * n + 1
    for blocks in range(n * n + 1):
        moves = blocks * group
        distance = reached.get((result, moves % 2))
        if distance is not None and distance <= moves:
            answer = blocks
            break
    return str(answer if answer <= n * n else -1)


def _polynomial_matrix_product(plan, data):
    n = int(data)
    assert n == plan["dimensions"]["n"] and 1 <= n <= 1000000
    first = second = Fraction(0)
    for new in range(1, n + 1):
        p = Fraction(new, n)
        q = 1 - p
        following_first = (p * second + q + q * p * first) / (1 - q * q)
        second = p * first + q * following_first
        first = following_first
    modulus = 998244353
    return " ".join(str(value.numerator * pow(value.denominator, -1, modulus) % modulus)
                    for value in (first, second))


def _polynomial_recurrence(plan, data):
    n = int(data)
    assert n == plan["dimensions"]["n"] and 1 <= n <= 10000000
    modulus = 998244353
    root_one = [0] * (n + 1)
    root_any = [0] * (n + 1)
    for order in range(1, n + 1):
        root_one[order] = ((order == 1) + 2 * root_any[order - 1]
                           + sum(root_any[i] * root_any[order - 1 - i] for i in range(1, order - 1))) % modulus
        root_zero = (2 * root_one[order] + sum(root_one[i] * root_one[order - i] for i in range(1, order))) % modulus
        root_any[order] = (root_one[order] + root_zero) % modulus
    return str(root_one[n])


def _aggregate_runs(plan, data):
    n, *values = map(int, data.split())
    assert n == len(values) == plan["dimensions"]["n"] and 1 <= n <= 500000
    assert all(1 <= value <= 10**18 for value in values)
    answer = 0
    for left in range(n):
        common = 0
        for right in range(left, n):
            common = gcd(common, values[right])
            answer += common == 1
    return str(answer)


def _multiset_permutation(plan, data):
    a, b, rank = map(int, data.split())
    assert (a, b) == (plan["dimensions"]["a"], plan["dimensions"]["b"])
    assert 1 <= a <= 30 and 1 <= b <= 30
    ways = [[1] * (b + 1) for _ in range(a + 1)]
    for i in range(1, a + 1):
        for j in range(1, b + 1):
            ways[i][j] = ways[i - 1][j] + ways[i][j - 1]
    assert 1 <= rank <= ways[a][b]
    word = []
    while a and b:
        smaller = ways[a - 1][b]
        if rank <= smaller:
            word.append("a")
            a -= 1
        else:
            word.append("b")
            b -= 1
            rank -= smaller
    return "".join(word) + "a" * a + "b" * b


def _interval_matrix_rank(plan, data):
    first, second, *rest = data.splitlines()
    n, m, colors = map(int, first.split())
    assert (n, m, colors) == tuple(plan["dimensions"][name] for name in ("n", "m", "c"))
    caps = list(map(int, second.split()))
    assert 2 <= n <= 150 and n - 1 <= m <= min(colors * n * (n - 1) // 2, 500000) and 1 <= colors <= 300
    assert len(rest) == m and len(caps) == colors and all(1 <= cap < n for cap in caps) and sum(caps) <= 300
    by_color = [set() for _ in range(colors)]
    for line in rest:
        u, v, color = map(int, line.split())
        assert 1 <= u < v <= n and 1 <= color <= colors
        current = by_color[color - 1]
        assert (u, v) not in current
        current.add((u, v))
    # Edmondsの交差定理による独立な判定。
    # 色の残りが容量以上なら辺を全部残す。未満なら全部を森側へ移しても目的値は増えない。
    graphic, partition = [0] * (1 << colors), [0] * (1 << colors)
    for mask in range(1 << colors):
        components = [{vertex} for vertex in range(1, n + 1)]
        for color, edges in enumerate(by_color):
            if not (mask >> color & 1):
                continue
            partition[mask] += min(caps[color], len(edges))
            for u, v in edges:
                first = next(part for part in components if u in part)
                second = next(part for part in components if v in part)
                if first != second:
                    first.update(second)
                    components.remove(second)
        graphic[mask] = n - len(components)
    assert graphic[-1] == n - 1
    result = 0
    for left in range(colors):
        for right in range(left + 1, colors + 1):
            active = (1 << right) - (1 << left)
            included = active
            while graphic[included] + partition[active ^ included] >= n - 1:
                if included == 0:
                    result += 1
                    break
                included = (included - 1) & active
    return str(result)


def _domino_tableau(plan, data):
    n, *lengths = map(int, data.split())
    assert n == len(lengths) == plan["dimensions"]["height"]
    assert 1 <= n <= 1000 and lengths == sorted(lengths) and 1 <= min(lengths) <= max(lengths) <= 1000

    @lru_cache(None)
    def count(shape):
        if not any(shape):
            return 1
        result = 0
        for i, length in enumerate(shape):
            below = shape[i + 1] if i + 1 < len(shape) else 0
            if length - below >= 2:
                result += count(shape[:i] + (length - 2,) + shape[i + 1:])
            if i + 1 < len(shape) and length == below:
                after = shape[i + 2] if i + 2 < len(shape) else 0
                if below > after:
                    result += count(shape[:i] + (length - 1, length - 1) + shape[i + 2:])
        return result

    return str(count(tuple(reversed(lengths))) % 998244353)


def _prefix_dominant_compositions(plan, data):
    n, p, q = map(int, data.split())
    assert (n, p, q) == tuple(plan["dimensions"][name] for name in ("n", "p", "q"))
    assert 1 <= n <= 500000 and 1 <= p <= 500000 and 1 <= q <= 500000 and gcd(p, q) == 1

    modulus = 998244353
    current = [[0] * (q + 1) for _ in range(p + 1)]
    current[0][0] = 1
    answers = []
    for _ in range(n):
        sums = [[0] * (q + 2) for _ in range(p + 2)]
        following = [[0] * (q + 1) for _ in range(p + 1)]
        for a in range(p + 1):
            for b in range(q + 1):
                sums[a + 1][b + 1] = (current[a][b] + sums[a][b + 1] + sums[a + 1][b] - sums[a][b]) % modulus
                if q * a >= p * b:
                    following[a][b] = sums[a + 1][b + 1]
        current = following
        answers.append(current[p][q])
    return "\n".join(map(str, answers))


def _polynomial_moments(plan, data):
    n, moves, *cells = map(int, data.split())
    assert (n, moves) == (plan["dimensions"]["n"], plan["dimensions"]["moves"])
    assert 1 <= n <= 100000 and 1 <= moves <= 100000 and len(cells) == 3 * n
    modulus = 998244353
    assert all(1 <= cell < modulus for cell in cells)

    # Schedules visiting j distinct cards: (C)_j * S(k,j).
    stirling = [[0] * (moves + 1) for _ in range(moves + 1)]
    stirling[0][0] = 1
    for k in range(1, moves + 1):
        for j in range(1, k + 1):
            stirling[k][j] = (stirling[k - 1][j - 1] + j * stirling[k - 1][j]) % modulus

    answers = [1] + [0] * moves
    for index in range(0, len(cells), 3):
        front, back, total = cells[index:index + 3]
        choose_cards = 1
        scores = [pow(front, total, modulus)]
        for j in range(1, min(total, moves) + 1):
            choose_cards = choose_cards * (total - j + 1) % modulus
            scores.append(choose_cards * pow(front, total - j, modulus) * pow(back, j, modulus) % modulus)
        group = [sum(stirling[k][j] * scores[j] for j in range(min(k + 1, len(scores)))) % modulus
                 for k in range(moves + 1)]
        answers = [sum(comb(k, j) * answers[k - j] * group[j] for j in range(k + 1)) % modulus
                   for k in range(moves + 1)]
    return "\n".join(map(str, answers[1:]))


def _subset_xor_square_range(plan, data):
    dimensions, values, *rows = map(lambda line: list(map(int, line.split())), data.splitlines())
    n, q = dimensions
    assert n == plan["dimensions"]["n"] and q == plan["dimensions"]["q"]
    assert len(values) == n and len(rows) == q and all(0 <= x < 2**30 for x in values)
    answers = []
    for kind, first, second in rows:
        if kind == 1:
            assert 1 <= first <= n and 0 <= second < 2**30
            values[first - 1] = second
        else:
            assert kind == 2 and 1 <= first <= second <= n
            subset_xor = [0]
            for value in values[first - 1:second]:
                subset_xor += [x ^ value for x in subset_xor]
            answers.append(str(sum(x * x for x in subset_xor) % 998244353))
    return "\n".join(answers)


def _josephus_survivor(plan, data):
    rows = [tuple(map(int, line.split())) for line in data.splitlines()]
    assert rows.pop() == (0, 0) and len(rows) == plan["dimensions"]["t"] <= 50
    output = []
    for count, step in rows:
        assert 1 <= count <= plan["dimensions"]["n"] <= 999 and 1 <= step <= 999
        alive = list(range(1, count + 1))
        position = 0
        while len(alive) > 1:
            position = (position + step - 1) % len(alive)
            alive.pop(position)
        output.append(str(alive[0]))
    return "\n".join(output)


def _dyadic_interval_cover(plan, data):
    left, right = map(int, data.split())
    limit = 1 << plan["dimensions"]["h"]
    assert 0 <= left < right <= limit <= 2**60
    blocks = []

    def visit(lo, hi):
        if right <= lo or hi <= left:
            return
        if left <= lo and hi <= right:
            blocks.append(f"{lo} {hi}")
            return
        middle = (lo + hi) // 2
        visit(lo, middle)
        visit(middle, hi)

    visit(0, limit)
    return "\n".join([str(len(blocks)), *blocks])


def _divisibility_chain_ends(plan, data):
    n, m = map(int, data.split())
    assert 1 <= n == plan["dimensions"]["n"] <= 200000 and 1 <= m == plan["dimensions"]["m"] <= 200000
    modulus = 998244353
    state = [0] + [1] * m
    for _ in range(1, n):
        state = [0] + [sum(state[previous] for previous in range(1, end + 1) if end % previous == 0) % modulus
                       for end in range(1, m + 1)]
    return str(sum(state) % modulus)


def _pairwise_coprime_progression(plan, data):
    first, *rows = data.splitlines()
    assert int(first) == plan["dimensions"]["t"] == len(rows) <= 100000
    answers = []
    for row in rows:
        n, step = map(int, row.split())
        assert 1 <= n <= 10**18 and 1 <= step <= 10**18
        possible = True
        for modulus in range(2, min(n, 106)):
            if any(modulus % d == 0 for d in range(2, modulus)):
                continue
            if all(sum((start + i * step) % modulus == 0 for i in range(min(n, 2 * modulus))) >= 2
                   for start in range(modulus)):
                possible = False
                break
        answers.append("Yes" if possible else "No")
    return "\n".join(answers)


def _tree_subset_medians(plan, data):
    rows = data.splitlines()
    n, k = map(int, rows[0].split())
    assert (n, k) == (plan["dimensions"]["n"], plan["dimensions"]["k"])
    assert 1 <= k <= n <= 200000 and len(rows) == n
    graph = [[] for _ in range(n)]
    for row in rows[1:]:
        u, v = map(int, row.split())
        assert 1 <= u < v <= n
        graph[u - 1].append(v - 1)
        graph[v - 1].append(u - 1)
    distances = []
    for source in range(n):
        current = [-1] * n
        current[source] = 0
        queue = [source]
        for u in queue:
            for v in graph[u]:
                if current[v] < 0:
                    current[v] = current[u] + 1
                    queue.append(v)
        assert len(queue) == n
        distances.append(current)
    total = 0
    for selected in combinations(range(n), k):
        values = [sum(row[v] for v in selected) for row in distances]
        total += values.count(min(values))
    modulus = 998244353
    return str(total * pow(comb(n, k), -1, modulus) % modulus)


def _weighted_taylor_shifts(plan, data):
    dimensions, coefficients, weights = (list(map(int, row.split())) for row in data.splitlines())
    n, m = dimensions
    assert (n, m) == (plan["dimensions"]["n"], plan["dimensions"]["m"])
    assert 1 <= n <= 100000 and 1 <= m <= 100000 and len(coefficients) == n + 1 and len(weights) == m + 1
    mod = 998244353
    assert all(0 <= value < mod for value in (*coefficients, *weights))
    result = [0] * (n + 1)
    for shift, weight in enumerate(weights):
        for degree, a in enumerate(coefficients):
            for power in range(degree + 1):
                result[power] = (result[power] + weight * a * comb(degree, power) * pow(-shift, degree - power, mod)) % mod
    return " ".join(map(str, result))


def _dice_orientation(plan, data):
    dimensions, start, goal, *cells = data.splitlines()
    h, w = map(int, dimensions.split())
    sy, sx = (int(value) - 1 for value in start.split())
    gy, gx = (int(value) - 1 for value in goal.split())
    assert (h, w) == (plan["dimensions"]["h"], plan["dimensions"]["w"])
    assert 2 <= h <= 1000 and 2 <= w <= 1000 and len(cells) == h
    assert all(len(row) == w and set(row) <= {".", "#"} for row in cells)
    assert (sy, sx) != (gy, gx) and cells[sy][sx] == cells[gy][gx] == "."
    # Only the movement of the initial top face's normal is needed for this oracle.
    queue = deque([(sy, sx, (0, 0, 1), 0)])
    seen = {(sy, sx, (0, 0, 1))}
    while queue:
        y, x, (nx, ny, nz), steps = queue.popleft()
        if (y, x, nz) == (gy, gx, 1):
            return str(steps)
        for dy, dx, face in ((-1, 0, (nx, nz, -ny)), (1, 0, (nx, -nz, ny)),
                             (0, 1, (nz, ny, -nx)), (0, -1, (-nz, ny, nx))):
            vy, vx = y + dy, x + dx
            state = vy, vx, face
            if 0 <= vy < h and 0 <= vx < w and cells[vy][vx] == "." and state not in seen:
                seen.add(state)
                queue.append((vy, vx, face, steps + 1))
    return "-1"


def _line_chip_firing(plan, data):
    n, *chips = map(int, data.split())
    assert 1 <= n == plan["dimensions"]["n"] <= 200000
    assert len(chips) == n and all(0 <= chip <= 10**9 for chip in chips)
    count = Counter(chips)
    pending = deque(position for position, amount in count.items() if amount > 1)
    operations = 0
    while pending:
        position = pending.popleft()
        if count[position] >= 2:
            count[position] -= 2
            operations += 1
            for neighbor in (position - 1, position + 1):
                count[neighbor] += 1
                if count[neighbor] == 2:
                    pending.append(neighbor)
            if count[position] >= 2:
                pending.append(position)
    return str(operations)


def _hub_sparse_distances(plan, data):
    values = iter(map(int, data.split()))
    n, m, q = (next(values) for _ in range(3))
    assert (n, m, q) == tuple(plan["dimensions"][key] for key in ("n", "m", "q"))
    assert 2 <= n <= 200000 and 1 <= m <= 200000 and 1 <= q <= 200000
    tolls = [next(values) for _ in range(n)]
    assert all(0 <= value <= 10**18 for value in tolls)
    distances = [[0 if u == v else tolls[u] + tolls[v] for v in range(n)] for u in range(n)]
    for index in range(1, m + 1):
        u, v, weight = next(values), next(values), next(values)
        assert 1 <= u < v <= n and 0 <= weight <= 10**18
        u -= 1
        v -= 1
        distances[u][v] = distances[v][u] = min(distances[u][v], weight * 2**index)
    for pivot in range(n):
        for u in range(n):
            for v in range(n):
                distances[u][v] = min(distances[u][v], distances[u][pivot] + distances[pivot][v])
    answers = []
    for _ in range(q):
        u, v = next(values), next(values)
        assert 1 <= u < v <= n
        answers.append(str(distances[u - 1][v - 1]))
    assert next(values, None) is None
    return "\n".join(answers)


def _histogram_rook_cover(plan, data):
    n, *heights = map(int, data.split())
    assert 1 <= n == plan["dimensions"]["n"] <= 400
    assert len(heights) == n and all(1 <= height <= n for height in heights)
    cells = [(column, row) for column, height in enumerate(heights) for row in range(height)]
    cover = []
    for column, row in cells:
        mask = 0
        for i, (other_column, other_row) in enumerate(cells):
            lo, hi = sorted((column, other_column))
            if column == other_column or (row == other_row and min(heights[lo:hi + 1]) > row):
                mask |= 1 << i
        cover.append(mask)
    all_cells = (1 << len(cells)) - 1
    unions = [0] * (all_cells + 1)
    total = 0
    for selected in range(1, all_cells + 1):
        unions[selected] = unions[selected & (selected - 1)] | cover[(selected & -selected).bit_length() - 1]
        total += unions[selected] == all_cells
    return str(total % 998244353)


def _ferrers_rook(plan, data):
    n, *ends = map(int, data.split())
    assert 1 <= n == plan["dimensions"]["n"] <= 200000
    assert len(ends) == n and all(0 <= end < n for end in ends)
    circle = (1 << n) - 1
    masks = []
    for start in range(n):
        shifted = [((1 << (end + 1)) - 1) << start for end in ends]
        masks.append([(value & circle) | (value >> n) for value in shifted])
    total = 0
    for order in permutations(range(n)):
        face_up = circle
        for start, length_index in enumerate(order):
            face_up ^= masks[start][length_index]
        total += face_up.bit_count()
    modulus = 998244353
    return str(total * pow(factorial(n), -1, modulus) % modulus)


def _krawtchouk_transform(plan, data):
    n, k, *weights = map(int, data.split())
    assert 1 <= n == plan["dimensions"]["n"] <= 100000 and 1 <= k == plan["dimensions"]["k"] <= 10**9
    assert len(weights) == n + 1 and min(weights) >= 0 and sum(weights) == 10**9
    mod = 998244353
    inverse = pow(n, -1, mod)
    matrix = [[0] * (n + 1) for _ in range(n + 1)]
    for i in range(n):
        matrix[i + 1][i] = (n - i) * inverse % mod
        matrix[i][i + 1] = (i + 1) * inverse % mod
    inverse_total = pow(10**9, -1, mod)
    values = [weight * inverse_total % mod for weight in weights]
    while k:
        if k % 2:
            values = [sum(x * y for x, y in zip(row, values, strict=True)) % mod for row in matrix]
        columns = list(zip(*matrix))
        matrix = [[sum(x * y for x, y in zip(row, col, strict=True)) % mod for col in columns] for row in matrix]
        k //= 2
    return " ".join(map(str, values))


def _polynomial_differential_operator(plan, data):
    n, m, *a = map(int, data.split())
    mod = 998244353
    assert n == plan["dimensions"]["n"] and m == len(a) == plan["dimensions"]["m"]
    assert 0 <= n <= 100000 and 1 <= m <= 100000 and all(0 <= x < mod for x in a)
    assert a[:2] == [plan["dimensions"]["constant"], mod - plan["dimensions"]["degree"]] and not any(a[2:])
    return " ".join(map(str, _direct_differential_operator(n, a)))


def _direct_differential_operator(n, a):
    mod = 998244353
    terms = [(i, val) for i, val in enumerate(a) if val != 0]
    width = 2 * n + 2
    values = [0, 1] + [0] * (width - 2)
    for t in range(n):
        next_values = [0] * (width - 1 - t)
        for degree, value in enumerate(values):
            if degree != 0 and degree - 1 < len(next_values):
                next_values[degree - 1] -= degree * value
            if degree + 1 < len(next_values):
                next_values[degree + 1] += degree * value
            for i, coefficient in terms:
                if i + degree >= len(next_values):
                    break
                next_values[i + degree] += coefficient * value
        values = [value % mod for value in next_values]
    return values


def _reeds_sloane(plan, data):
    n, first, step, modulus = map(int, data.split())
    assert (n, first, step, modulus) == tuple(plan["dimensions"][key] for key in ("l", "a", "b", "m"))
    assert all(1 <= v < 10**18 for v in (n, first, step))
    assert first + step * (n - 1) < 10**18 and 2 <= modulus <= 10**9
    return str(int("".join(str(first + i * step) for i in range(n))) % modulus)


def _short_simple_path(plan, data):
    n, m, *edges = map(int, data.split())
    assert 2 <= n == plan["dimensions"]["n"] <= 20000
    assert 0 <= m == plan["dimensions"]["m"] <= min(n * (n - 1) // 2, 50000)
    assert len(edges) == 2 * m
    graph = [set() for _ in range(n + 1)]
    for u, v in zip(edges[::2], edges[1::2], strict=True):
        assert 1 <= u <= n and 1 <= v <= n and u != v and v not in graph[u]
        graph[u].add(v)
        graph[v].add(u)

    def closes(path):
        if len(path) == 5:
            return 1 in graph[path[-1]]
        return any(closes(path + [u]) for u in graph[path[-1]] if u not in path)

    return "YES" if closes([1]) else "NO"


def round_robin_latin_oracle(requested):
    n = len(requested)
    fixed = [u for u in range(n) if u == requested[u]]
    if any(requested[requested[u]] != u for u in range(n)) or (n % 2 and len(fixed) != 1):
        return "No"
    if n == 1:
        return "Yes\n1"

    rounds = n if n % 2 else n - 1
    cyclic = deque(range(rounds))
    baseline = [[0] * n for _ in range(n)]
    for round_index in range(rounds):
        first = cyclic[0]
        last = first if n % 2 else rounds
        baseline[first][last] = baseline[last][first] = round_index
        for offset in range(1, rounds // 2 + 1):
            u, v = cyclic[offset], cyclic[-offset]
            baseline[u][v] = baseline[v][u] = round_index
        cyclic.rotate(-1)
    if n % 2 == 0:
        for i in range(n):
            baseline[i][i] = rounds
    first_pairs = sorted((i, j) for i in range(n) for j in range(i + 1, n) if baseline[i][j] == 0)
    requested_pairs = [(i, j) for i, j in enumerate(requested) if i < j]
    new_labels = [0] * n
    for (i, j), (u, v) in zip(requested_pairs, first_pairs):
        new_labels[i], new_labels[j] = u, v
    if n % 2 == 0:
        for (i, j), (u, v) in zip(zip(fixed[::2], fixed[1::2]), first_pairs[len(requested_pairs):]):
            new_labels[i], new_labels[j] = u, v
            baseline[u][v] = baseline[v][u] = rounds
            baseline[u][u] = baseline[v][v] = 0
    matrix = [[baseline[u][v] for v in new_labels] for u in new_labels]
    for i, row in enumerate(matrix):
        assert sorted(row) == list(range(n)) and row[requested[i]] == 0
        assert all(row[j] == matrix[j][i] for j in range(n))
    return "\n".join(["Yes", *(" ".join(str(value + 1) for value in row) for row in matrix)])


def _round_robin_schedule(plan, data):
    tests, n, *requested = map(int, data.split())
    assert tests == 1 and n == len(requested) == plan["dimensions"]["n"] and 1 <= n <= 500
    assert all(1 <= value <= n for value in requested)
    assert sum(i != value for i, value in enumerate(requested, 1)) == 2 * plan["dimensions"]["pairs"]
    return round_robin_latin_oracle([value - 1 for value in requested])


def _xor_convolution_without_division(plan, data):
    lines = data.splitlines()
    dimension = int(lines[0])
    assert dimension == plan["dimensions"]["n"] and 0 <= dimension <= 17
    length = 1 << dimension
    assert len(lines) == 1 + 2 * length
    polynomials = []
    for line in lines[1:]:
        bits = list(map(int, line.split()))
        assert len(bits) == 32 and set(bits) <= {0, 1}
        polynomials.append(sum(value << i for i, value in enumerate(bits)))
    result = [0] * length
    for i, a in enumerate(polynomials[:length]):
        for j, b in enumerate(polynomials[length:]):
            for exponent in range(32):
                if b >> exponent & 1:
                    result[i ^ j] ^= a << exponent
    return "\n".join(" ".join(str(value >> bit & 1) for bit in range(63)) for value in result)


def _exponential_coefficient_transform(plan, data):
    n, m, *offsets = map(int, data.split())
    assert n == plan["dimensions"]["n"] and m == len(offsets) == plan["dimensions"]["m"]
    assert all(offset == plan["dimensions"]["offset"] for offset in offsets)
    return "\n".join(map(str, _occupancy_probability(n, offsets)))


def _occupancy_probability(n, offsets):
    modulus = 998244353
    inverse_length = pow(100000000, -1, modulus)
    left_fraction = [offset * inverse_length % modulus for offset in offsets]
    ordinary = [pow(factorial(i), -1, modulus) for i in range(n + 1)]
    inverse_m = pow(len(offsets), -1, modulus)
    full = [ordinary[i] * (1 - sum(pow(fraction, i, modulus) for fraction in left_fraction) * inverse_m) % modulus
            for i in range(n + 1)]
    previous = [1]
    answers = []
    for bins in range(1, n + 1):
        # A prefix may never have more blue balls than red. If a new bin makes both
        # counts equal, at least one blue ball in that bin must lie to the right of its red ball.
        current = [0] * (bins + 1)
        for taken, ways in enumerate(previous):
            for added in range(bins - taken + 1):
                weight = full[added] if taken + added == bins else ordinary[added]
                current[taken + added] = (current[taken + added] + ways * weight) % modulus
        answers.append(current[bins] * factorial(bins) * pow(bins, -bins, modulus) % modulus)
        previous = current
    return answers


def _degree_sequence(plan, data):
    n, *degree = map(int, data.split())
    assert n == len(degree) == plan["dimensions"]["n"] and 1 <= n <= 100000
    assert degree == sorted(degree) and 0 <= min(degree) <= max(degree) < n

    def graphical(values):
        remaining = sorted(values, reverse=True)
        while remaining and remaining[0] > 0:
            neighbors, remaining = remaining[0], remaining[1:]
            if neighbors > len(remaining) or remaining[neighbors - 1] <= 0:
                return False
            for v in range(neighbors):
                remaining[v] -= 1
            remaining.sort(reverse=True)
        return True

    if graphical(degree):
        return "YES"
    for i, value in enumerate(degree):
        if value + 1 < n and graphical(degree[:i] + [value + 1] + degree[i + 1:]):
            return "NO"
    return "ABSOLUTELY NO"


def independent(plan, data):
    oracles = {
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
    return oracles[plan["verify_id"]](plan, data)
