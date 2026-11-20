#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""同一ジョブで測った比較元と現在版から性能判定を作る。"""
from __future__ import annotations

from statistics import median

RELATIVE_TOLERANCE_PERCENT = 5
MINIMUM_TOLERANCE_NS = {"kernel_elapsed_ns": 5_000, "child_cpu_ns": 100_000}
WORST_CASE_POLICY = "library_worst_case"


def _compare_medians(metric: str, baseline: int | float, observed: int | float) -> dict:
    tolerance = max(baseline * RELATIVE_TOLERANCE_PERCENT / 100, MINIMUM_TOLERANCE_NS[metric])
    limit = baseline + tolerance
    return {"status": "PASS" if observed <= limit else "FAIL", "metric": metric,
            "baseline_median_ns": baseline, "limit_ns": limit, "observed_median_ns": observed}


def evaluate(metric: str, samples: dict[str, list[int]]) -> dict:
    if metric not in MINIMUM_TOLERANCE_NS:
        raise ValueError("性能退行の時間種別が不正です")
    if (not isinstance(samples, dict) or set(samples) != {"base", "current"}
            or any(not isinstance(group, list) or not group
                   or any(type(sample) is not int or sample < 0 for sample in group)
                   for group in samples.values())
            or len(samples["base"]) != len(samples["current"])):
        raise ValueError("性能退行の判定には同数の基準・現在版の測定値が必要です")
    return _compare_medians(metric, median(samples["base"]), median(samples["current"]))


def evaluate_worst(metric: str, cases: dict[str, dict[str, list[int]]]) -> dict:
    if not isinstance(cases, dict) or not cases or any(not isinstance(name, str) or not name for name in cases):
        raise ValueError("最悪ケースの判定にはケース名と測定値が必要です")
    results = {name: evaluate(metric, samples) for name, samples in cases.items()}
    base = max(results, key=lambda name: (results[name]["baseline_median_ns"], name))
    current = max(results, key=lambda name: (results[name]["observed_median_ns"], name))
    return {**_compare_medians(metric, results[base]["baseline_median_ns"], results[current]["observed_median_ns"]),
            "baseline_case": base, "observed_case": current, "compared_cases": len(results)}
