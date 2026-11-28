# SPDX-License-Identifier: CC0-1.0
"""CI専用の決定的Max・構造ケース。入力と独立な閉形式の正解を逐次生成する。"""
from __future__ import annotations

import hashlib
from math import prod
from pathlib import Path
import time

LIBRARY_CHECKER_REVISION = "633c05792d34bcfa9994f6eaba8bd802bebe81bc"
UNION_FIND_LIMITS = {"n": 200_000, "q": 200_000}
YUKI2295_LIMITS = {"n": 200_000, "q": 300_000, "weight": 1_000_000_000}
PREFIX_SUM_2D_LIMITS = {"n": 500, "q": 100_000, "m": 200_000}
PREFIX_SUM_3D_LIMITS = {"n": 100, "q": 200_000, "value": 999}
MAX_INPUT_BYTES = 16 * 1024 * 1024
MAX_OUTPUT_BYTES = 8 * 1024 * 1024
MOD = 998244353


class GenerationError(ValueError):
    pass


def case_plans(quick: bool = False) -> list[dict]:
    """quickは縮小した生成器検査であり、問題のMaxケースとは表示しない。"""
    uf_source = ("https://github.com/yosupo06/library-checker-problems/blob/" +
                 LIBRARY_CHECKER_REVISION + "/data_structure/unionfind/info.toml")
    yuki_source = "https://yukicoder.me/problems/no/2295"
    values = [
        ("union_find_max", "union_find", 64 if quick else 200_000,
         256 if quick else 200_000, None, 2026111601,
         "max_size_and_operations", "Connected prefix and distinct singleton closed form."),
        ("union_find_balanced", "union_find", 32 if quick else 131_072,
         256 if quick else 200_000, None, 2026111602,
         "balanced_depth_and_max_operations", "All vertices connected after known balanced merges."),
        ("persistent_union_find_balanced", "persistent_union_find_union_path_query",
         32 if quick else 131_072, 256 if quick else 300_000, None, 229516,
         "balanced_history_and_max_operations", "Highest differing bit identifies the first connecting merge."),
        ("persistent_union_find_two_stars", "persistent_union_find_union_path_query",
         64 if quick else 200_000, 256 if quick else 300_000, None, 229517,
         "max_size_operations_and_int64_product", "Two-star tree path maximum and Python integer pair sum."),
        ("persistent_union_find_query_heavy", "persistent_union_find_union_path_query",
         64 if quick else 200_000, 256 if quick else 300_000,
         16 if quick else 65_536, 229565536,
         "max_size_operations_and_history_queries", "Opposite balanced halves connect only at final weight 1e9."),
        ("persistent_union_find_singleton", "persistent_union_find_union_path_query",
         1, 256 if quick else 300_000, None, 229518,
         "singleton_and_max_operations", "Only self distances and singleton pair sums, both zero."),
    ]
    plans = []
    for name, identifier, n, q, active, seed, category, oracle in values:
        limits = UNION_FIND_LIMITS if identifier == "union_find" else YUKI2295_LIMITS
        dimensions = {"n": n, "q": q}
        if active is not None:
            dimensions["active_balanced_component"] = active
        if identifier == "union_find":
            build_operations = n // 2 if name == "union_find_max" else n - 1
        elif name.endswith("two_stars"):
            build_operations = n + 1
        elif name.endswith("singleton"):
            build_operations = 0
        else:
            build_operations = 2 * (active or n) - 3
        if build_operations >= q:
            raise GenerationError("planned build leaves no queries")
        plans.append({"name": name, "verify_id": identifier, "dimensions": dimensions,
                      "seed": seed, "category": category, "planned_build_operations": build_operations,
                      "size_profile": "quick_smoke" if quick else "full",
                      "problem_limits": dict(limits),
                      "maximum_dimensions": [] if quick else [k for k in ("n", "q") if dimensions[k] == limits[k]],
                      "constraint_source": uf_source if identifier == "union_find" else yuki_source,
                      "constraint_checked_date": "2026-11-16", "oracle": oracle})
    for identifier, limits, small, task, oracle in (
        ("prefix_sum_2d", PREFIX_SUM_2D_LIMITS, {"n": 8, "q": 48, "m": 24},
         "abc106", "Count four repeated endpoint pairs directly."),
        ("prefix_sum_3d", PREFIX_SUM_3D_LIMITS, {"n": 5, "q": 48},
         "abc366", "Sum an affine value over three independent integer intervals."),
    ):
        maximum = {key: value for key, value in limits.items() if key != "value"}
        plans.append({"name": identifier + "_max", "verify_id": identifier,
                      "dimensions": small if quick else maximum, "seed": None,
                      "category": "max_size_and_queries", "planned_build_operations": 0,
                      "size_profile": "quick_smoke" if quick else "full",
                      "problem_limits": dict(limits), "maximum_dimensions": [] if quick else list(maximum),
                      "constraint_source": f"https://atcoder.jp/contests/{task}/tasks/{task}_d",
                      "constraint_checked_date": "2026-11-26", "oracle": oracle})
    return plans


class Random64:
    """Python版によるrandrange実装差を避ける固定64bit生成器。"""
    def __init__(self, seed: int):
        self.state = seed & ((1 << 64) - 1)

    def below(self, limit: int) -> int:
        if limit <= 0:
            raise GenerationError("random limit must be positive")
        self.state = (self.state * 6364136223846793005 + 1442695040888963407) & ((1 << 64) - 1)
        return self.state % limit


class CaseWriter:
    def __init__(self, directory: Path, expected_operations: int, timeout: float):
        self.input_path = directory / "input.txt"
        self.expected_path = directory / "expected.txt"
        directory.mkdir(parents=True, exist_ok=True)
        self.input = self.input_path.open("wb")
        try:
            self.expected = self.expected_path.open("wb")
        except OSError:
            self.input.close()
            raise
        self.input_hash, self.output_hash = hashlib.sha256(), hashlib.sha256()
        self.input_bytes = self.output_bytes = self.operations = self.answers = 0
        self.expected_operations = expected_operations
        self.deadline = time.monotonic() + timeout

    def line(self, *values: int, operation: bool = False) -> None:
        data = (" ".join(map(str, values)) + "\n").encode("ascii")
        self.input_bytes += len(data)
        if self.input_bytes > MAX_INPUT_BYTES:
            raise GenerationError("input size cap exceeded")
        if operation:
            self.operations += 1
            if self.operations > self.expected_operations:
                raise GenerationError("operation budget exceeded")
            if self.operations % 4096 == 0 and time.monotonic() > self.deadline:
                raise GenerationError("generation timeout")
        self.input.write(data)
        self.input_hash.update(data)

    def answer(self, value: int) -> None:
        data = (str(value) + "\n").encode("ascii")
        self.output_bytes += len(data)
        if self.output_bytes > MAX_OUTPUT_BYTES:
            raise GenerationError("expected output size cap exceeded")
        self.expected.write(data)
        self.output_hash.update(data)
        self.answers += 1

    def finish(self) -> dict:
        if self.operations != self.expected_operations or self.answers == 0:
            raise GenerationError("missing operations or nonempty oracle")
        if time.monotonic() > self.deadline:
            raise GenerationError("generation timeout")
        return {"input_sha256": self.input_hash.hexdigest(),
                "expected_sha256": self.output_hash.hexdigest(),
                "input_bytes": self.input_bytes, "expected_bytes": self.output_bytes,
                "generated_operations": self.operations, "expected_answer_count": self.answers}

    def close(self) -> None:
        self.input.close()
        self.expected.close()


def _union_find(plan: dict, writer: CaseWriter) -> dict:
    n, q = plan["dimensions"]["n"], plan["dimensions"]["q"]
    random = Random64(plan["seed"])
    writer.line(n, q)
    if plan["name"] == "union_find_max":
        merged = n // 2
        for vertex in range(1, merged + 1):
            writer.line(0, 0, vertex, operation=True)
        active = merged + 1
        query = 0
        while writer.operations < q:
            mode = query % 4
            if mode == 0:
                u, v, answer = random.below(active), random.below(active), 1
            elif mode == 1:
                u, v, answer = random.below(active), active + random.below(n - active), 0
            elif mode == 2:
                u = v = random.below(n)
                answer = 1
            else:
                u = active + random.below(n - active)
                v = active + (u - active + 1) % (n - active)
                answer = int(u == v)
            writer.line(1, u, v, operation=True)
            writer.answer(answer)
            query += 1
        return {"successful_merges": merged}
    stride = 1
    while stride < n:
        for start in range(0, n, 2 * stride):
            writer.line(0, start, start + stride, operation=True)
        stride *= 2
    query = 0
    while writer.operations < q:
        # 奇数strideの巡回で異なる葉を訪れ、最初は最深の葉を照会する。
        vertex = (n - 1 - query * 104729) % n
        writer.line(1, vertex, 0, operation=True)
        writer.answer(1)
        query += 1
    return {"successful_merges": n - 1, "precompression_tree_depth": n.bit_length() - 1}


def _persistent_union_find(plan: dict, writer: CaseWriter) -> dict:
    name = plan["name"]
    n, q = plan["dimensions"]["n"], plan["dimensions"]["q"]
    active = plan["dimensions"].get("active_balanced_component", n)
    random = Random64(plan["seed"])
    x = count = pair_sum = largest_cross = 0
    weight_offset = 1_000_000_000 - (active - 1)
    writer.line(n, x, q)

    def move(target: int) -> None:
        nonlocal x
        delta = (target - x) % n
        if delta:
            writer.line(4, delta, operation=True)
            x = target

    def merge(left: int, right: int, left_size: int, right_size: int) -> None:
        nonlocal count, pair_sum, largest_cross
        move(left)
        count += 1
        weight = weight_offset + count
        writer.line(1, right, weight, operation=True)
        cross = weight * left_size * right_size
        largest_cross = max(largest_cross, cross)
        pair_sum = (pair_sum + cross) % MOD

    if name.endswith("balanced") or name.endswith("query_heavy"):
        stride = 1
        while stride < active:
            for start in range(0, active, 2 * stride):
                merge(start, start + stride, stride, stride)
            stride *= 2
    elif name.endswith("two_stars"):
        half = n // 2
        for vertex in range(1, half):
            merge(0, vertex, vertex, 1)
        for vertex in range(half + 1, n):
            merge(half, vertex, vertex - half, 1)
        merge(0, half, half, half)
    build_operations = writer.operations
    type2_queries = 0
    while writer.operations < q:
        if name.endswith("query_heavy"):
            u, v = random.below(active // 2), active // 2 + random.below(active // 2)
            writer.line(2, u, v, operation=True)
            writer.answer(1_000_000_000)
            x = (x + 1_000_000_000) % n
            type2_queries += 1
            continue
        turn = writer.operations % 7
        u, v = random.below(n), random.below(n)
        if turn in (0, 3):
            writer.line(3, u, operation=True)
            writer.answer(pair_sum)
        elif turn == 6:
            value = random.below(n)
            writer.line(4, value, operation=True)
            x = (x + value) % n
        else:
            if turn == 1:
                v = u
            if u == v:
                answer = 0
            elif name.endswith("balanced"):
                bit = (u ^ v).bit_length() - 1
                index = active - (active >> bit) + (u >> (bit + 1)) + 1
                answer = weight_offset + index
            else:
                half = n // 2
                answer = (1_000_000_000 if u // half != v // half else
                          weight_offset + (half - 1 if u >= half else 0) + max(u % half, v % half))
            writer.line(2, u, v, operation=True)
            writer.answer(answer)
            x = (x + answer) % n
            type2_queries += 1
    return {"successful_merges": count, "build_operations": build_operations,
            "type2_queries": type2_queries, "maximum_cross_product": largest_cross,
            "cross_product_exceeds_int64": largest_cross > (1 << 63) - 1}


def cuboid_rows(n: int, q: int, constant: bool):
    """Yield the input rows and closed-form answers for rectangular queries."""
    yield (n,), None
    for x in range(1, n + 1):
        for y in range(1, n + 1):
            values = ((PREFIX_SUM_3D_LIMITS["value"],) * n if constant else
                      tuple(90 + 2 * x + 3 * y + 4 * z for z in range(1, n + 1)))
            yield values, None
    yield (q,), None
    for i in range(q):
        if i % 5 == 0:
            left, right = (1, 1, 1), (n, n, n)
        else:
            left = tuple((i * scale + axis) % n + 1 for axis, scale in enumerate((3, 7, 13)))
            right = tuple(lo + (i + 2 * axis) % (n - lo + 1) for axis, lo in enumerate(left))
        volume = prod(hi - lo + 1 for lo, hi in zip(left, right))
        answer = (PREFIX_SUM_3D_LIMITS["value"] * volume if constant else
                  volume * (180 + sum(scale * (lo + hi)
                                      for scale, lo, hi in zip((2, 3, 4), left, right))) // 2)
        yield tuple(coord for pair in zip(left, right) for coord in pair), answer


def _train_rows(n: int, m: int, q: int):
    yield (n, m, q), None
    trains = ((1, 1), (1, n), (n // 2, n // 2 + 1), (n, n))
    for i in range(m):
        yield trains[i % len(trains)], None
    for i in range(q):
        p = i % n + 1
        left, right = ((1, n), (p, p), (1, p), (p, n))[i % 4]
        yield (left, right), sum((m + len(trains) - 1 - j) // len(trains)
                                 for j, (lo, hi) in enumerate(trains) if left <= lo and hi <= right)


def generate_case(plan: dict, directory: Path, *, generation_timeout: float = 60) -> dict:
    """計画に対応する有効入力を生成。Q超過・空の正解・サイズ上限超過は失敗。"""
    n, q = plan["dimensions"]["n"], plan["dimensions"]["q"]
    limits = plan["problem_limits"]
    if not 1 <= n <= limits["n"] or not 1 <= q <= limits["q"]:
        raise GenerationError("case dimensions exceed cited problem limits")
    known = {p["name"]: p["verify_id"] for p in case_plans()}
    if known.get(plan["name"]) != plan["verify_id"]:
        raise GenerationError("unknown case or Verify ID")
    if plan not in case_plans() + case_plans(True):
        raise GenerationError("plan differs from the reviewed case catalog")
    writer = CaseWriter(directory, q, generation_timeout)
    try:
        identifier = plan["verify_id"]
        if identifier in {"prefix_sum_2d", "prefix_sum_3d"}:
            rows = (_train_rows(n, plan["dimensions"]["m"], q) if identifier == "prefix_sum_2d" else
                    cuboid_rows(n, q, constant=False))
            for fields, answer in rows:
                writer.line(*fields, operation=answer is not None)
                if answer is not None:
                    writer.answer(answer)
            details = {"build_operations": 0}
        else:
            details = (_union_find(plan, writer) if identifier == "union_find" else
                       _persistent_union_find(plan, writer))
        if details.get("build_operations", details.get("successful_merges")) != plan["planned_build_operations"]:
            raise GenerationError("actual build differs from planned operation budget")
        return {**writer.finish(), **details}
    finally:
        writer.close()
