#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""ci_verifyのJSONを、Actionsのstep summary用Markdownへ集計する。"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import sys


@dataclass(frozen=True)
class Summary:
    status: str
    compiler: str = ""
    fallback: bool | None = None
    targets: int | None = None
    checked: int = 0
    skip: int = 0
    unsuccessful_ids: tuple[str, ...] = ()


def inline_code(value: str) -> str:
    """表・HTML・改行を入力値から作らせず、短いコード表記にする。"""
    value = " ".join(value.split())
    value = "".join(char if char.isprintable() else "?" for char in value)
    if len(value) > 80:
        value = value[:79] + "…"
    content = "".join(char if char.isalnum() or char in " ./+-" else f"&#{ord(char)};"
                      for char in value)
    return "<code>" + content + "</code>"


def summarize(report: object) -> Summary:
    if not isinstance(report, dict) or type(report.get("format")) is not int or report["format"] != 1:
        return Summary("不正な結果")
    compiler, fallback = report.get("compiler"), report.get("standard_fallback")
    plan, rows = report.get("plan"), report.get("results")
    if (not isinstance(compiler, str) or not compiler.strip() or type(fallback) is not bool
            or not isinstance(plan, dict) or not isinstance(rows, list)):
        return Summary("不正な結果")
    identifiers = plan.get("selected_ids")
    if (not isinstance(identifiers, list)
            or any(not isinstance(identifier, str) or not identifier for identifier in identifiers)
            or len(set(identifiers)) != len(identifiers)):
        return Summary("不正な結果", compiler, fallback)
    selected = set(identifiers)
    successful: set[tuple[str, bool]] = set()
    seen: set[tuple[str, bool]] = set()
    malformed = False
    failed = False
    for row in rows:
        if (not isinstance(row, dict) or not isinstance(row.get("id"), str)
                or row["id"] not in selected or type(row.get("skip_runtime_checks")) is not bool):
            malformed = True
            continue
        key = (row["id"], row["skip_runtime_checks"])
        if key in seen:
            malformed = True
        seen.add(key)
        samples = row.get("samples")
        if (type(row.get("passed")) is not bool or type(row.get("compile_ok")) is not bool
                or not isinstance(samples, list)
                or any(not isinstance(sample, dict) or type(sample.get("passed")) is not bool
                       for sample in samples)
                or row.get("compiler") != compiler
                or type(row.get("standard_fallback")) is not bool
                or row["standard_fallback"] != fallback):
            malformed = True
            continue
        if row["passed"] and row["compile_ok"] and samples and all(s["passed"] for s in samples):
            successful.add(key)
        else:
            failed = True
    expected = {(identifier, skip) for identifier in selected for skip in (False, True)}
    reported_status = report.get("status")
    if reported_status == "FAIL":
        status = "FAIL"
    elif malformed or reported_status not in ("PASS", "running"):
        status = "不正な結果"
    elif reported_status == "running" or seen != expected:
        status = "未完了"
    elif failed:
        status = "FAIL"
    else:
        status = "PASS" if selected else "対象なし"
    unsuccessful = tuple(sorted(identifier for identifier in selected
                                if any((identifier, skip) not in successful for skip in (False, True))))
    return Summary(status, compiler, fallback, len(selected),
                   sum(not skip for _, skip in successful),
                   sum(skip for _, skip in successful), unsuccessful)


def read_summary(path: Path) -> Summary:
    try:
        return summarize(json.loads(path.read_text(encoding="utf-8")))
    except FileNotFoundError:
        return Summary("結果なし")
    except (OSError, UnicodeError, ValueError):
        return Summary("不正な結果")


def markdown(summaries: list[Summary]) -> str:
    lines = ["### ローカル Verify", "",
             "C++23のビルド・入力例照合です。オンライン提出の判定（AC等）は含みません。", "",
             "| 結果 | コンパイラ | 標準代替 | 対象ID数 | checked成功 | skip成功 |",
             "| --- | --- | --- | ---: | ---: | ---: |"]
    for summary in summaries:
        compiler = inline_code(summary.compiler) if summary.compiler else "—"
        fallback = "有効" if summary.fallback is True else "無効" if summary.fallback is False else "—"
        total = str(summary.targets) if summary.targets is not None else "—"
        checked = f"{summary.checked}/{total}" if summary.targets else "—"
        skip = f"{summary.skip}/{total}" if summary.targets else "—"
        lines.append(f"| {summary.status} | {compiler} | {fallback} | {total} | {checked} | {skip} |")
    for index, summary in enumerate(summaries, 1):
        if summary.unsuccessful_ids:
            names = ", ".join(inline_code(identifier) for identifier in summary.unsuccessful_ids[:8])
            remaining = len(summary.unsuccessful_ids) - 8
            suffix = f"、ほか{remaining}件" if remaining > 0 else ""
            lines += ["", f"結果{index}の未成功ID: {names}{suffix}"]
    return "\n".join(lines) + "\n"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     epilog='例: ci_verify_summary.py results.json >> "$GITHUB_STEP_SUMMARY"')
    parser.add_argument("reports", nargs="+", type=Path, help="ci_verify.pyの結果JSON（複数可）")
    args = parser.parse_args(argv)
    summaries = [read_summary(path) for path in args.reports]
    sys.stdout.write(markdown(summaries))
    return int(any(summary.status not in ("PASS", "対象なし") for summary in summaries))


if __name__ == "__main__":
    raise SystemExit(main())
