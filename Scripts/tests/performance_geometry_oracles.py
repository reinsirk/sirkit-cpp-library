# SPDX-License-Identifier: CC0-1.0
"""Small linear-scan oracles for geometry performance cases."""

from collections import deque


def locate_convex(data):
    lines = data.splitlines()
    n = int(lines[0])
    polygon = [tuple(map(int, row.split())) for row in lines[1:n+1]]
    queries = [tuple(map(int, row.split())) for row in lines[n+2:]]
    assert len(queries) == int(lines[n+1])
    assert all(abs(x) <= 10**9 and abs(y) <= 10**9 for x, y in polygon + queries)
    edges = list(zip(polygon, polygon[1:] + polygon[:1]))
    assert all((b[0] - a[0]) * (c[1] - b[1]) - (b[1] - a[1]) * (c[0] - b[0]) > 0
               for (a, b), (_, c) in zip(edges, edges[1:] + edges[:1]))
    answers = []
    for x, y in queries:
        smallest = min((b[0] - a[0]) * (y - a[1]) - (b[1] - a[1]) * (x - a[0]) for a, b in edges)
        answers.append("OUT" if smallest < 0 else "ON" if smallest == 0 else "IN")
    return " ".join(answers)

def cover_orthogonal(data):
    tokens = iter(map(int, data.split()))
    n = next(tokens)
    polygons = []
    for _ in range(n):
        count = next(tokens)
        polygon = [(next(tokens), next(tokens)) for _ in range(count)]
        polygons.append(polygon)
    q = next(tokens)
    answers = []
    for _ in range(q):
        x, y = next(tokens), next(tokens)
        total = 0
        for polygon in polygons:
            crossings = 0
            for (x1, y1), (x2, y2) in zip(polygon, polygon[1:] + polygon[:1]):
                if x1 == x2 and x1 > x and (y1 <= y) != (y2 <= y):
                    crossings += 1
            total += crossings % 2
        answers.append(total)
    assert next(tokens, None) is None
    return " ".join(map(str, answers))


def debris_bfs(data):
    tokens = iter(map(int, data.split()))
    n, height, radius = next(tokens), next(tokens), next(tokens)
    points = [(next(tokens), next(tokens)) for _ in range(n)]
    assert next(tokens, None) is None
    assert 1 <= n <= 100000 and 2 <= height <= 10**9 and 1 <= radius <= 10**9
    assert all(abs(x) <= 10**9 and 1 <= y < height for x, y in points)
    first = n
    points.append((0, 0))
    remaining = set(range(n))
    queue = deque([(first, 0)])
    limit = radius * radius
    while queue:
        source, distance = queue.popleft()
        x, y = points[source]
        if source != first and x * x + (height - y)**2 <= limit:
            return str(distance + 1)
        for target in list(remaining):
            px, py = points[target]
            if (px - x)**2 + (py - y)**2 <= limit:
                remaining.remove(target)
                queue.append((target, distance + 1))
    return "-1"


def manhattan_minima(data):
    n, *values = map(int, data.split())
    assert 2 <= n <= 200000 and sorted(values) == list(range(1, n + 1))
    return " ".join(str(min(abs(i - j) + abs(x - y) for j, y in enumerate(values) if i != j))
                    for i, x in enumerate(values))


def cuboid_containment(data):
    n, *values = map(int, data.split())
    assert 2 <= n <= 200000 and len(values) == 3 * n
    assert all(1 <= value <= 10**9 for value in values)
    boxes = [sorted(values[i:i + 3]) for i in range(0, len(values), 3)]
    nested = any(all(small < large for small, large in zip(a, b)) for a in boxes for b in boxes)
    return "Yes" if nested else "No"


def lattice_polygon_grid_count(data):
    n, *coordinates = map(int, data.split())
    assert 3 <= n <= 100000 and len(coordinates) == 2 * n
    vertices = list(zip(coordinates[::2], coordinates[1::2]))
    assert len(set(vertices)) == n and all(0 <= item <= 10**9 for item in coordinates)
    xs, ys = zip(*vertices)
    edges = list(zip(vertices, vertices[1:] + vertices[:1]))
    points = 0
    for x in range(min(xs), max(xs) + 1):
        for y in range(min(ys), max(ys) + 1):
            signs = [(u - a) * (y - b) - (v - b) * (x - a) for (a, b), (u, v) in edges]
            points += min(signs) >= 0 or max(signs) <= 0
    return str(points - n)


def polygon_chords(data):
    lines = data.splitlines()
    n, q = map(int, lines[0].split())
    points = [tuple(map(int, line.split())) for line in lines[1:n+1]]
    assert q == len(lines) - n - 1
    answers = []
    for line in lines[n+1:]:
        left, right = (int(token) - 1 for token in line.split())
        polygon = points[left:right+1] if left < right else points[left:] + points[:right+1]
        area, sx, sy = 0, 0, 0
        for (ax, ay), (bx, by) in zip(polygon, polygon[1:] + polygon[:1]):
            cross = ax * by - ay * bx
            area += cross
            sx += cross * (ax + bx)
            sy += cross * (ay + by)
        answers.extend((f"{sx/(3*area):.12f}", f"{sy/(3*area):.12f}"))
    return " ".join(answers)


def clipped_polygon_area(data):
    from fractions import Fraction

    lines = data.splitlines()
    n, m = map(int, lines[0].split())
    vertices = [tuple(map(Fraction, line.split())) for line in lines[1:]]
    assert len(vertices) == n + m
    result = vertices[:n]
    other = vertices[n:]
    for a, b in zip(other, other[1:] + other[:1]):
        def side(p):
            return (b[0]-a[0])*(p[1]-a[1]) - (b[1]-a[1])*(p[0]-a[0])

        next_polygon = []
        for p, q in zip(result, result[1:] + result[:1]):
            f, g = side(p), side(q)
            if f >= 0:
                next_polygon.append(p)
            if (f < 0 < g) or (g < 0 < f):
                part = f/(f-g)
                next_polygon.append((p[0]+part*(q[0]-p[0]), p[1]+part*(q[1]-p[1])))
        result = next_polygon
    edges = zip(result, result[1:] + result[:1])
    area = abs(sum(x*w-y*z for (x,y),(z,w) in edges))/2
    return f"{float(area):.12f}"


def circle_polygon_quadrants(data):
    from math import pi

    lines = data.splitlines()
    n, radius = map(int, lines[0].split())
    points = [tuple(map(int, line.split())) for line in lines[1:]]
    assert len(points) == n
    edges = list(zip(points, points[1:] + points[:1]))
    for (x, y), (u, v) in edges:
        assert (x == u and x in (-100, 0, 100)) or (y == v and y in (-100, 0, 100))
    inside = 0
    for x in (-0.5, 0.5):
        for y in (-0.5, 0.5):
            inside += sum(a == c and x < a and min(b, d) < y < max(b, d)
                          for (a, b), (c, d) in edges) % 2
    return f"{pi * (radius**2 * inside / 4):.12f}"


def circle_tangents_score(data):
    from math import acos, atan2, cos, hypot, sin

    lines = data.splitlines()
    cases = int(lines[0])
    assert len(lines) == 2 * cases + 1
    scores = []
    for i in range(cases):
        x, y, r = map(int, lines[2 * i + 1].split())
        u, v, s = map(int, lines[2 * i + 2].split())
        distance = hypot(u - x, v - y)
        assert all(-100 <= t <= 100 for t in (x, y, u, v)) and 0 < r <= s <= 100 and distance > r + s
        baseline = atan2(v - y, u - x)
        score = 0.0
        for projection in (r - s, r + s):
            angle = acos(projection / distance)
            for theta in (baseline + angle, baseline - angle):
                a, b = cos(theta), sin(theta)
                c = -(a * x + b * y + r)
                score += abs(a + b + c) / max(abs(a), abs(b), abs(c))
        scores.append(f"{score:.8f}")
    return "\n".join(scores)


def independent(plan, data):
    oracles = {
        "circle_tangents_yuki3344": circle_tangents_score,
        "convex_polygon_locator": locate_convex,
        "circle_polygon_intersection": circle_polygon_quadrants,
        "half_plane_intersection": clipped_polygon_area,
        "orthogonal_polygon_coverage": cover_orthogonal,
        "kd_tree_point_extractor": debris_bfs,
        "lattice_polygon_typical90": lattice_polygon_grid_count,
        "manhattan_nearest_neighbors": manhattan_minima,
        "polygon_chord_moments": polygon_chords,
        "static_cuboid_sum_box_in_box": cuboid_containment,
    }
    return oracles[plan["verify_id"]](data)
