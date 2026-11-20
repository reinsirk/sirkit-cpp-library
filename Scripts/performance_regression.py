#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""同一ジョブで測った比較元と現在版から性能判定を作る。"""
from __future__ import annotations

from statistics import median

LOCAL_PROFILE = "local"
REGRESSION_PROFILES = {
    LOCAL_PROFILE: (5, {"kernel_elapsed_ns": 5_000, "child_cpu_ns": 100_000}),
    "github-hosted": (15, {"kernel_elapsed_ns": 50_000, "child_cpu_ns": 1_000_000}),
}
WORST_CASE_POLICY = "library_worst_case"


def tolerance_description(metric: str, profile: str) -> str:
    relative, minimum = REGRESSION_PROFILES[profile]
    return f"{relative}%（最低{minimum[metric] // 1_000} µs）"


def _compare_medians(metric: str, baseline: int | float, observed: int | float, profile: str) -> dict:
    relative, minimum = REGRESSION_PROFILES[profile]
    tolerance = max(baseline * relative / 100, minimum[metric])
    limit = baseline + tolerance
    return {"status": "PASS" if observed <= limit else "FAIL", "metric": metric,
            "baseline_median_ns": baseline, "limit_ns": limit, "observed_median_ns": observed}


def evaluate(metric: str, samples: dict[str, list[int]], *, profile: str = LOCAL_PROFILE) -> dict:
    if profile not in REGRESSION_PROFILES or metric not in REGRESSION_PROFILES[profile][1]:
        raise ValueError("性能退行の設定または時間種別が不正です")
    if (not isinstance(samples, dict) or set(samples) != {"base", "current"}
            or any(not isinstance(group, list) or not group
                   or any(type(sample) is not int or sample < 0 for sample in group)
                   for group in samples.values())
            or len(samples["base"]) != len(samples["current"])):
        raise ValueError("性能退行の判定には同数の基準・現在版の測定値が必要です")
    return _compare_medians(metric, median(samples["base"]), median(samples["current"]), profile)


def evaluate_worst(metric: str, cases: dict[str, dict[str, list[int]]], *, profile: str = LOCAL_PROFILE) -> dict:
    if not isinstance(cases, dict) or not cases or any(not isinstance(name, str) or not name for name in cases):
        raise ValueError("最悪ケースの判定にはケース名と測定値が必要です")
    results = {name: evaluate(metric, samples, profile=profile) for name, samples in cases.items()}
    base = max(results, key=lambda name: (results[name]["baseline_median_ns"], name))
    current = max(results, key=lambda name: (results[name]["observed_median_ns"], name))
    return {**_compare_medians(metric, results[base]["baseline_median_ns"], results[current]["observed_median_ns"], profile),
            "baseline_case": base, "observed_case": current, "compared_cases": len(results)}
