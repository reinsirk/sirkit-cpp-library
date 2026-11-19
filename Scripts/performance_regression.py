#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""同一ジョブで測った比較元と現在版から性能判定を作る。"""
from __future__ import annotations

from statistics import median

RELATIVE_TOLERANCE_PERCENT = 5
MINIMUM_TOLERANCE_NS = {"kernel_elapsed_ns": 5_000, "child_cpu_ns": 100_000}


def evaluate(metric: str, samples: dict[str, list[int]]) -> dict:
    if metric not in MINIMUM_TOLERANCE_NS:
        raise ValueError("性能退行の時間種別が不正です")
    if (not isinstance(samples, dict) or set(samples) != {"base", "current"}
            or any(not isinstance(group, list) or not group
                   or any(type(sample) is not int or sample < 0 for sample in group)
                   for group in samples.values())
            or len(samples["base"]) != len(samples["current"])):
        raise ValueError("性能退行の判定には同数の基準・現在版の測定値が必要です")
    baseline = median(samples["base"])
    observed = median(samples["current"])
    tolerance = max(baseline * RELATIVE_TOLERANCE_PERCENT / 100, MINIMUM_TOLERANCE_NS[metric])
    limit = baseline + tolerance
    return {"status": "PASS" if observed <= limit else "FAIL", "metric": metric,
            "baseline_median_ns": baseline, "limit_ns": limit, "observed_median_ns": observed}
