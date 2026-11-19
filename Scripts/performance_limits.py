#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""保存済み実測から固定した性能予算を読む。測定結果で予算を書き換えない。"""
from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import statistics


class LimitError(ValueError):
    pass


def integer(value: object, name: str, minimum: int = 0) -> int:
    if type(value) is not int or value < minimum:
        raise LimitError("性能予算の整数が不正です: " + name)
    return value


def identifier(value: object, name: str) -> str:
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z0-9_][A-Za-z0-9_.-]*", value):
        raise LimitError("性能予算の識別子が不正です: " + name)
    return value


def library_path(value: object) -> str:
    if (not isinstance(value, str) or not value or "\\" in value or "\0" in value
            or PurePosixPath(value).is_absolute() or ".." in PurePosixPath(value).parts
            or PurePosixPath(value).suffix != ".cpp" or PurePosixPath(value).as_posix() != value):
        raise LimitError("性能予算のライブラリパスが不正です")
    return value


def key(entry: dict) -> tuple:
    scope = entry.get("scope")
    common = (scope, library_path(entry.get("library")))
    case = identifier(entry.get("case"), "case")
    if scope == "whole_program":
        identifier(entry.get("verify_id"), "verify_id")
        integer(entry.get("input_bytes"), "input_bytes")
        return (*common, entry["verify_id"], case)
    if scope == "kernel":
        identifier(entry.get("kernel_id"), "kernel_id")
        size = integer(entry.get("size"), "size", 1)
        pattern = integer(entry.get("pattern"), "pattern")
        if pattern > 3:
            raise LimitError("性能予算のpatternは0..3が必要です")
        rounds = integer(entry.get("rounds"), "rounds", 1)
        return (*common, entry["kernel_id"], case, size, pattern, rounds)
    raise LimitError("性能予算の測定範囲が不正です")


class Limits:
    def __init__(self, path: Path):
        self.path = path.resolve()
        data = self.path.read_bytes()
        value = json.loads(data)
        if not isinstance(value, dict) or type(value.get("format")) is not int or value["format"] != 1:
            raise LimitError("性能予算のformatは1が必要です")
        baseline = value.get("baseline")
        policy = value.get("policy")
        if not isinstance(baseline, dict) or not baseline:
            raise LimitError("性能予算の固定基準がありません")
        if (not isinstance(policy, dict) or type(policy.get("multiplier")) is not int
                or policy["multiplier"] != 2
                or type(policy.get("whole_program_minimum_ns")) is not int
                or policy["whole_program_minimum_ns"] != 10_000_000):
            raise LimitError("性能予算は基準の2倍・全体CPU時間の下限10msで固定します")
        entries = value.get("limits")
        if not isinstance(entries, list) or not entries:
            raise LimitError("性能予算が空です")
        self._entries = {}
        for entry in entries:
            if not isinstance(entry, dict):
                raise LimitError("性能予算の行が不正です")
            identity = key(entry)
            expected_metric = "child_cpu_ns" if entry["scope"] == "whole_program" else "kernel_elapsed_ns"
            if entry.get("metric") != expected_metric:
                raise LimitError("性能予算の時間種別が一致しません")
            baseline_ns = integer(entry.get("baseline_median_ns"), "baseline_median_ns", 1)
            limit_ns = integer(entry.get("limit_ns"), "limit_ns", 1)
            expected = baseline_ns * policy["multiplier"]
            if entry["scope"] == "whole_program":
                expected = max(expected, policy["whole_program_minimum_ns"])
            if limit_ns != expected:
                raise LimitError("性能予算が固定基準と一致しません")
            if identity in self._entries:
                raise LimitError("性能予算が重複しています: " + repr(identity))
            self._entries[identity] = copy.deepcopy(entry)
        self.sha256 = hashlib.sha256(data).hexdigest()
        repository = Path(__file__).resolve().parents[1]
        name = self.path.relative_to(repository).as_posix() if self.path.is_relative_to(repository) else str(self.path)
        self.metadata = {"path": name, "sha256": self.sha256,
                         "baseline": copy.deepcopy(baseline), "policy": copy.deepcopy(policy)}

    def whole(self, library: str, verify_id: str, case: str, input_bytes: int) -> dict:
        request = {"scope": "whole_program", "library": library, "verify_id": verify_id,
                   "case": case, "input_bytes": input_bytes}
        identity = key(request)
        if identity not in self._entries:
            raise LimitError("固定性能予算がありません: " + repr(identity))
        entry = self._entries[identity]
        if entry["input_bytes"] != input_bytes:
            raise LimitError("固定性能予算と入力サイズが一致しません: " + repr(identity))
        return copy.deepcopy(entry)

    def kernel(self, library: str, kernel_id: str, case: str, size: int, pattern: int, rounds: int) -> dict:
        request = {"scope": "kernel", "library": library, "kernel_id": kernel_id,
                   "case": case, "size": size, "pattern": pattern, "rounds": rounds}
        identity = key(request)
        if identity not in self._entries:
            raise LimitError("固定性能予算がありません: " + repr(identity))
        return copy.deepcopy(self._entries[identity])

    def evaluate(self, entry: dict, samples: list[int]) -> dict:
        # 呼び出し側が渡す予算も、読み込んだ固定値と再照合する。
        identity = key(entry)
        if identity not in self._entries or entry != self._entries[identity]:
            raise LimitError("読込後に性能予算が変更されています")
        if not isinstance(samples, list) or not samples:
            raise LimitError("性能予算の照合に必要な測定値がありません")
        for sample in samples:
            integer(sample, "sample_ns")
        observed = statistics.median(samples)
        return {"status": "PASS" if observed <= entry["limit_ns"] else "FAIL",
                "metric": entry["metric"], "baseline_median_ns": entry["baseline_median_ns"],
                "limit_ns": entry["limit_ns"], "observed_median_ns": observed,
                "config_sha256": self.sha256}

    def check_unchanged(self) -> None:
        if hashlib.sha256(self.path.read_bytes()).hexdigest() != self.sha256:
            raise LimitError("測定中に固定性能予算が変更されています")


def load(path: Path) -> Limits:
    return Limits(Path(path))
