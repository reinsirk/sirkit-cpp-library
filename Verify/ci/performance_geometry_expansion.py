# SPDX-License-Identifier: CC0-1.0
"""Deterministic workloads for new geometry libraries without LC cases."""

from math import gcd, hypot, pi, sqrt

LIBRARY_VERIFY_IDS = {
    "Geometry/Circle_Tangents.cpp": "circle_tangents_yuki3344",
    "Geometry/Orthogonal_Polygon_Coverage.cpp": "orthogonal_polygon_coverage",
    "Geometry/Circle_Polygon_Intersection.cpp": "circle_polygon_intersection",
    "Geometry/Convex_Polygon_Locator.cpp": "convex_polygon_locator",
    "Geometry/Half_Plane_Intersection.cpp": "half_plane_intersection",
    "Geometry/Kd_Tree_Point_Extractor.cpp": "kd_tree_point_extractor",
    "Geometry/Lattice_Polygon.cpp": "lattice_polygon_typical90",
    "Geometry/Manhattan_Nearest_Neighbors.cpp": "manhattan_nearest_neighbors",
    "Geometry/Polygon_Chord_Moments.cpp": "polygon_chord_moments",
    "Geometry/Static_Cuboid_Sum.cpp": "static_cuboid_sum_box_in_box",
}
PROFILES = {
    "circle_tangents_yuki3344": (
        ("near_contact_parallel_centers", {"n": 200000}, {"n": 17},
         "Centers are one unit farther apart than the combined radii, with moving common y coordinates."),
        ("diagonal_unequal_radii", {"n": 200000}, {"n": 25},
         "Each normal has a known projection on the fixed diagonal between centers.")),
    "orthogonal_polygon_coverage": (
        ("separate_diagonal_cells", {"n": 100000}, {"n": 18},
         "Disjoint unit squares cover exactly the corresponding diagonal cells."),
        ("overlapping_nested_rectangles", {"n": 100000}, {"n": 20},
         "Rectangle i contains (x,y) precisely if i<=y and x<N-i.")),
    "convex_polygon_locator": (
        ("large_lattice_polygon", {"quarter": 50000, "q": 200000, "stretch": 1},
         {"quarter": 8, "q": 30, "stretch": 1},
         "Primitive lattice directions give a strictly convex, centrally symmetric polygon; test the center and vertices."),
        ("thin_lattice_polygon", {"quarter": 50000, "q": 200000, "stretch": 63},
         {"quarter": 9, "q": 33, "stretch": 3},
         "A positive horizontal scaling preserves convexity; edge midpoints are integral and points past the largest x lie outside.")),
    "kd_tree_point_extractor": (
        ("kd_point_extraction_single_jump_chain", {"n": 100000}, {"n": 27},
         "Consecutive points lie exactly one jump apart, and skipping a point takes twice the permitted distance."),
        ("kd_point_extraction_dense_component", {"n": 100000}, {"n": 24},
         "The densely repeated points can all be reached from the start, but the destination is too far from every point.")),
    "manhattan_nearest_neighbors": (
        ("manhattan_neighbors_monotone_permutation", {"n": 200000}, {"n": 20},
         "Adjacent indices have distance 2 on the identity permutation. All other pairs are farther apart."),
        ("manhattan_neighbors_parity_partition", {"n": 200000}, {"n": 22},
         "Neighbors in the same parity block have distance 3. Across the block boundary the distance is at least 3.")),
    "static_cuboid_sum_box_in_box": (
        ("strictly_nested_boxes", {"n": 200000}, {"n": 37},
         "All three sorted dimensions increase for every successive box."),
        ("incomparable_boxes", {"n": 200000}, {"n": 39},
         "The smallest dimension increases while the middle dimension decreases.")),
    "lattice_polygon_typical90": (
        ("long_collinear_diagonal", {"n": 100000, "step": 10000}, {"n": 18, "step": 3},
         "The convex hull is a diagonal segment with one lattice point per integer coordinate."),
        ("all_points_on_strict_hull", {"quarter": 25000}, {"quarter": 4},
         "Each primitive-direction edge is on the convex hull. Compute the Pick count from the Python shoelace sum and edge gcds.")),
    "polygon_chord_moments": (
        ("narrow_triangular_slices", {"quarter": 7500, "q": 200000}, {"quarter": 6, "q": 27},
         "Each cut leaves a triangle, whose centroid is the arithmetic mean of its three vertices."),
        ("long_wrapping_slices", {"quarter": 7500, "q": 200000}, {"quarter": 7, "q": 37},
         "Exact triangle fan sums independently give the centroid of each long, possibly wrapping subpolygon.")),
    "half_plane_intersection": (
        ("collinear_rectangle_boundaries", {"quarter": 250}, {"quarter": 6},
         "Both polygons are rectangles with extra boundary vertices. Intersect their integer coordinate ranges."),
        ("strictly_convex_nested_polygons", {"quarter": 250}, {"quarter": 7},
         "A scaled copy lies inside the original convex polygon; its shoelace sum gives the intersection area.")),
    "circle_polygon_intersection": (
        ("semicircle_boundary", {"n": 100, "radius": 100}, {"n": 20, "radius": 47},
         "The rectangular boundary keeps precisely the right half of the disk."),
        ("concave_missing_quadrant", {"n": 100, "radius": 100}, {"n": 24, "radius": 79},
         "A concave orthogonal polygon keeps three full quadrants inside the disk.")),
}
VALIDITY = {
    "circle_tangents_yuki3344": "1<=T<=200000, integer center coordinates in [-100,100], 0<r1<=r2<=100, circles are strictly separated.",
    "orthogonal_polygon_coverage": "N,Q<=100000, total vertices <=400000; valid rectangles within[0,1e5]^2.",
    "convex_polygon_locator": "N,Q<=200000; distinct edges turn strictly left; coordinates lie in[-1e9,1e9].",
    "kd_tree_point_extractor": "N<=100000, 2<=W<=1e9, 1<=D<=1e9, -1e9<=x<=1e9 and 1<=y<W.",
    "manhattan_nearest_neighbors": "2<=N<=200000. P is a permutation of 1..N.",
    "static_cuboid_sum_box_in_box": "2<=N<=200000; all three side lengths are in [1,10^9].",
    "lattice_polygon_typical90": "3<=N<=100000; all pairs are distinct, with coordinates in [0,10^9].",
    "polygon_chord_moments": "4<=N<=30000, Q<=200000; strict CCW integer convex polygon in [-500000,500000]^2 and nonadjacent query endpoints.",
    "half_plane_intersection": "3<=N,M<=1000; both CCW vertex sequences are convex and all coordinates are in [0,30000].",
    "circle_polygon_intersection": "3<=N<=100, 1<=R<=100; simple CCW polygon with integer coordinates in [-100,100].",
}


def lattice_polygon(quarter, stretch):
    step = 1 if quarter < 50 else 16
    bound = step
    while True:
        directions = [(x, y) for x in range(1, bound + 1) for y in range(1, bound + 1) if gcd(x, y) == 1]
        if len(directions) >= quarter:
            break
        bound += step
    directions.sort(key=lambda pair: pair[1] / pair[0])
    quadrant = directions[:quarter]
    edges = (
        [(x, y) for x, y in quadrant]
        + [(-y, x) for x, y in quadrant]
        + [(-x, -y) for x, y in quadrant]
        + [(y, -x) for x, y in quadrant]
    )
    center_x, center_y = map(sum, zip(*edges[:2*quarter]))
    x, y = -center_x, -center_y
    polygon = []
    for dx, dy in edges:
        polygon.append((x * stretch, y))
        x += 2 * dx
        y += 2 * dy
    assert (x, y) == (-center_x, -center_y)
    return polygon


def convex_polygon_cases(plan, inp, out):
    d = plan["dimensions"]
    polygon = lattice_polygon(d["quarter"], d["stretch"])
    count = len(polygon)
    largest_x = max(x for x, _ in polygon)
    inp.line(count)
    for point in polygon:
        inp.line(*point)
    inp.line(d["q"])
    for i in range(d["q"]):
        if i % 3 == 0:
            inp.line(0, 0)
            out.line("IN")
        elif i % 3 == 1:
            j = i * 65537 % count
            x, y = polygon[j]
            if plan["scenario"] == "thin_lattice_polygon":
                next_x, next_y = polygon[(j + 1) % count]
                x, y = (x + next_x) // 2, (y + next_y) // 2
            inp.line(x, y)
            out.line("ON")
        else:
            inp.line(largest_x + 1, i % 32749)
            out.line("OUT")

def orthogonal_cases(plan, inp, out):
    n = plan["dimensions"]["n"]
    diagonal = plan["scenario"] == "separate_diagonal_cells"
    inp.line(n)
    for i in range(n):
        left, bottom, right, top = (i, i, i+1, i+1) if diagonal else (0, i, n-i, n)
        inp.line(4)
        if i % 2:
            inp.line(right, bottom, right, top, left, top, left, bottom)
        else:
            inp.line(left, bottom, left, top, right, top, right, bottom)
    inp.line(n)
    for i in range(n):
        y = i if i % 2 == 0 else (i + 1) % n
        inp.line(i, y)
        out.line(int(i == y) if diagonal else min(y + 1, n - i))


def point_extraction_cases(plan, inp, out):
    n = plan["dimensions"]["n"]
    if plan["scenario"] == "kd_point_extraction_single_jump_chain":
        inp.line(n, 7 * (n + 1), 7)
        for i in range(1, n + 1):
            inp.line(0, 7 * i)
        out.line(n + 1)
    else:
        inp.line(n, 10**9, 1000)
        for i in range(n):
            inp.line(i % 997 - 498, i % 503 + 1)
        out.line(-1)


def manhattan_neighbor_cases(plan, inp, out):
    n = plan["dimensions"]["n"]
    inp.line(n)
    if plan["scenario"] == "manhattan_neighbors_monotone_permutation":
        inp.line(*range(1, n + 1))
        out.line(*([2] * n))
    else:
        inp.line(*range(1, n + 1, 2), *range(2, n + 1, 2))
        out.line(*([3] * n))


def cuboid_cases(plan, inp, out):
    n = plan["dimensions"]["n"]
    nested = plan["scenario"] == "strictly_nested_boxes"
    inp.line(n)
    for i in range(n):
        middle = 200000000 + i if nested else 600000000 - i
        inp.line(middle, 900000000 + i, i + 1)
    out.line("Yes" if nested else "No")


def lattice_polygon_cases(plan, inp, out):
    if plan["scenario"] == "long_collinear_diagonal":
        n, step = (plan["dimensions"][key] for key in ("n", "step"))
        inp.line(n)
        for i in range(n - 1, -1, -1):
            inp.line(i * step, i * step)
        out.line((n - 1) * step + 1 - n)
        return
    points = lattice_polygon(plan["dimensions"]["quarter"], 1)
    minimum_x = min(x for x, _ in points)
    minimum_y = min(y for _, y in points)
    inp.line(len(points))
    for x, y in points:
        inp.line(x - minimum_x, y - minimum_y)
    edges = list(zip(points, points[1:] + points[:1]))
    area_twice = abs(sum(x * v - y * u for (x, y), (u, v) in edges))
    boundary = sum(gcd(abs(x - u), abs(y - v)) for (x, y), (u, v) in edges)
    out.line((area_twice + boundary) // 2 + 1 - len(points))


def chord_moment_cases(plan, inp, out):
    dims = plan["dimensions"]
    polygon = [(x // 2 + 57000, y // 2 - 61000) for x, y in lattice_polygon(dims["quarter"], 1)]
    n = len(polygon)
    q = dims["q"]
    assert max(abs(c) for vertex in polygon for c in vertex) <= 500000
    inp.line(n, q)
    for point in polygon:
        inp.line(*point)

    if plan["scenario"] == "narrow_triangular_slices":
        for i in range(q):
            start = (i * 65537 + n - 1) % n
            end = (start + 2) % n
            inp.line(start + 1, end + 1)
            a, b, c = (polygon[(start + offset) % n] for offset in range(3))
            out.line(f"{(a[0]+b[0]+c[0])/3:.12f}", f"{(a[1]+b[1]+c[1])/3:.12f}")
        return

    pairs = []
    for i in range(min(q, 64)):
        start = (i * 65537 + n - 1) % n
        length = n // 2 + 1 + i % max(1, n // 5)
        end = (start + length) % n
        a, b = polygon[start]
        area, x_moment, y_moment = 0, 0, 0
        for offset in range(1, length):
            c, d = polygon[(start + offset) % n]
            e, f = polygon[(start + offset + 1) % n]
            weight = (c - a) * (f - b) - (d - b) * (e - a)
            area += weight
            x_moment += weight * (a + c + e)
            y_moment += weight * (b + d + f)
        pairs.append((start + 1, end + 1, f"{x_moment/(3*area):.12f}", f"{y_moment/(3*area):.12f}"))
    for i in range(q):
        start, end, x, y = pairs[i % len(pairs)]
        inp.line(start, end)
        out.line(x, y)


def half_plane_cases(plan, inp, out):
    quarter = plan["dimensions"]["quarter"]

    def rectangle(left, bottom, right, top):
        return ([(left+(right-left)*i//quarter, bottom) for i in range(quarter)]
                + [(right, bottom+(top-bottom)*i//quarter) for i in range(quarter)]
                + [(right-(right-left)*i//quarter, top) for i in range(quarter)]
                + [(left, top-(top-bottom)*i//quarter) for i in range(quarter)])

    if plan["scenario"] == "collinear_rectangle_boundaries":
        a = rectangle(0, 0, 20000, 20000)
        b = rectangle(5000, 11000, 25000, 28000)
        area = 15000 * 9000
    else:
        vertices = lattice_polygon(quarter, 1)
        a = [(x + 14000, y + 15000) for x, y in vertices]
        b = [(x // 2 + 14000, y // 2 + 15000) for x, y in vertices]
        edges = zip(b, b[1:] + b[:1])
        area = sum(x*v-y*u for (x, y), (u, v) in edges)/2
    assert all(0 <= coord <= 30000 for x in (a,b) for p in x for coord in p)
    inp.line(len(a), len(b))
    for p in a + b:
        inp.line(*p)
    out.line(f"{area:.12f}")


def circle_tangents_cases(plan, inp, out):
    n = plan["dimensions"]["n"]
    near = plan["scenario"] == "near_contact_parallel_centers"
    inp.line(n)
    for i in range(n):
        if near:
            x, y, r, u, v, s = -100, i % 201 - 100, 99, 99, i % 201 - 100, 99
        else:
            x, y, r, u, v, s = -90, -80, 1 + i % 50, 75, 100, 50 + i % 51
        inp.line(x, y, r)
        inp.line(u, v, s)
        length = hypot(u - x, v - y)
        dx, dy = (u - x) / length, (v - y) / length
        score = 0.0
        for signed in (s, -s):
            along = (r - signed) / length
            across = sqrt(1 - along * along)
            for side in (across, -across):
                a, b = along * dx - side * dy, along * dy + side * dx
                c = -a * x - b * y - r
                score += abs(a + b + c) / max(abs(a), abs(b), abs(c))
        out.line(f"{score:.8f}")


def circle_polygon_cases(plan, inp, out):
    n, radius = (plan["dimensions"][key] for key in ("n", "radius"))
    if plan["scenario"] == "semicircle_boundary":
        corners = [(0, -100), (100, -100), (100, 100), (0, 100)]
        fraction = 0.5
    else:
        corners = [(-100, -100), (100, -100), (100, 0), (0, 0), (0, 100), (-100, 100)]
        fraction = 0.75
    inp.line(n, radius)
    for index, ((x, y), (u, v)) in enumerate(zip(corners, corners[1:] + corners[:1])):
        count = n // len(corners) + (index < n % len(corners))
        for step in range(count):
            inp.line(x + (u - x) * step // count, y + (v - y) * step // count)
    out.line(f"{pi * (radius * radius * fraction):.12f}")


def generate(plan, inp, out):
    generators = {
        "circle_tangents_yuki3344": circle_tangents_cases,
        "convex_polygon_locator": convex_polygon_cases,
        "circle_polygon_intersection": circle_polygon_cases,
        "half_plane_intersection": half_plane_cases,
        "orthogonal_polygon_coverage": orthogonal_cases,
        "kd_tree_point_extractor": point_extraction_cases,
        "lattice_polygon_typical90": lattice_polygon_cases,
        "manhattan_nearest_neighbors": manhattan_neighbor_cases,
        "polygon_chord_moments": chord_moment_cases,
        "static_cuboid_sum_box_in_box": cuboid_cases,
    }
    generators[plan["verify_id"]](plan, inp, out)
