#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""同じPR実行で取得した全分割の計測を集約し、原本の100%判定を行う。"""

import argparse
import json
import os
from pathlib import Path
import shutil
import sys

if __package__:
    from . import ci_instrumented as ci
else:
    import ci_instrumented as ci


def read_json(path: Path) -> dict:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"JSONのオブジェクトが必要です: {path}")
    return value


def merge_maps(target: dict, current: dict) -> None:
    for key, value in current.items():
        if target.setdefault(key, value) != value:
            raise ValueError(f"分割で記録した値が一致しません: {key}")


def merge(entries: list[dict], root: Path, parts: Path, output: Path, count: int, provenance: dict) -> dict:
    if not 2 <= count <= len(entries):
        raise ValueError("分割数が不正です")
    root = root.resolve()
    ids = [entry["id"] for entry in entries]
    if len(set(ids)) != len(ids):
        raise ValueError("計測対象IDが重複しています")
    allowed = ci.library_paths(entries, root)
    library_hashes = {path.relative_to(root).as_posix(): ci.digest(path) for path in allowed}
    vendor_hashes = {path.relative_to(root).as_posix(): ci.digest(path)
                     for path in (root / "Verify/current/vendor").rglob("*") if path.is_file()}
    input_reports, exports, commands = [], [], []
    results, hashes, tools = {}, {}, {}
    for index in range(count):
        directory = parts / f"coverage-part-{index}"
        shard = read_json(directory / "results.json")
        selected = ids[index::count]
        if (shard.get("format") != 1 or shard.get("mode") != "coverage" or shard.get("checked") is not True
                or shard.get("coverage_shard") != {"index": index, "count": count}
                or shard.get("selected_ids") != selected or shard.get("available_entries") != len(ids)
                or shard.get("coverage_complete") is not True
                or shard.get("status") != "PASS" or shard.get("execution_status") != "PASS"):
            raise ValueError(f"分割 {index} の計測が不足しています")
        if any(shard.get("provenance", {}).get(key) != value for key, value in provenance.items()):
            raise ValueError(f"分割 {index} の実行来歴が一致しません")
        source_hashes = shard["all_source_hashes"]
        library_names = {name for entry in entries[index::count] for name in entry["libraries"]}
        if (source_hashes != shard.get("source_hashes_after") or vendor_hashes != shard.get("vendor_source_hashes")
                or shard.get("library_source_hashes") != {name: library_hashes[name] for name in library_names}
                or any(source_hashes.get(name) != library_hashes[name] for name in library_names)):
            raise ValueError(f"分割 {index} のソース来歴が一致しません")
        if ((root / ci.COVERAGE_EXCLUSIONS).is_file() and ci.COVERAGE_EXCLUSIONS not in source_hashes
                or any(entry.get("coverage_test") for entry in entries[index::count])
                and ci.COVERAGE_TESTS not in source_hashes):
            raise ValueError(f"分割 {index} に登録ファイルのSHAがありません")
        merge_maps(hashes, source_hashes)
        merge_maps(tools, shard["tools"])
        if [row["id"] for row in shard["results"]] != selected:
            raise ValueError(f"分割 {index} の結果IDが一致しません")
        for position, row in enumerate(shard["results"]):
            number = index + position * count
            path = f"coverage_entries/entry_{number:04}.json"
            entry = entries[number]
            samples = row["samples"]
            if (row.get("status") != "PASS" or row.get("compile_returncode") != 0
                    or len(samples) != len(entry["samples"]) or not samples
                    or any(sample.get("passed") is not True or sample.get("returncode") != 0
                           or sample.get("profile_written") is not True for sample in samples)
                    or row.get("driver_sha256") != source_hashes[entry["driver"]]
                    or row.get("coverage_path") != path):
                raise ValueError(f"ビルド・実行・計測の記録が不足しています: {row['id']}")
            exported = read_json(directory / path)
            measured_files = [file["filename"] for file in exported["files"]]
            expected_files = {str(root / name) for name in entry["libraries"]}
            if len(set(measured_files)) != len(measured_files) or not set(measured_files) <= expected_files:
                raise ValueError(f"原本の計測範囲が不正です: {row['id']}")
            exports.append(exported)
            results[row["id"]] = row
        commands.extend(shard["commands"])
        input_reports.append(directory)

    for name, checksum in hashes.items():
        file = (root / name).resolve()
        if not file.is_relative_to(root) or file.relative_to(root).as_posix() != name or ci.digest(file) != checksum:
            raise ValueError(f"計測時の原本SHAが現在の内容と一致しません: {name}")

    report = {"format": 1, "mode": "coverage", "checked": True, "execution_status": "PASS",
              "online_status": "not_evaluated", "artifacts_exclude_binaries_and_raw_profiles": True,
              "selected_ids": ids, "results": [results[name] for name in ids], "commands": commands,
              "tools": tools, "available_entries": len(entries),
              "available_local_fixtures": sum(not entry.get("problem_url") for entry in entries),
              "coverage_shards": count, "all_source_hashes": hashes, "source_hashes_after": hashes,
              "library_source_hashes": library_hashes, "vendor_source_hashes": vendor_hashes,
              "provenance": {**provenance, "github_run_attempt": os.getenv("GITHUB_RUN_ATTEMPT"),
                             "github_job": os.getenv("GITHUB_JOB")}}
    output.mkdir(parents=True)
    for directory in input_reports:
        for name in ("sources", "logs", "coverage_entries"):
            shutil.copytree(directory / name, output / name, dirs_exist_ok=True)
    shared = ci.shared_coverage_blocks(root)
    coverage = ci.write_coverage(report, exports, root, output, shared, ci.load_coverage_exclusions(root, shared))
    gate = ci.coverage_gate(coverage, allowed, expected_translation_units=len(entries),
                            measured_translation_units=len(exports))
    report.update(status=gate["status"], coverage_gate=gate, unmapped_sources=gate["unmapped_sources"])
    ci.write_report(output, report)
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--parts", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--shard-count", type=int, required=True)
    args = parser.parse_args()
    try:
        provenance = {key.lower(): os.environ[key] for key in ("GITHUB_SHA", "GITHUB_RUN_ID", "GITHUB_REPOSITORY")
                      if key in os.environ}
        if "SIRKIT_PR_HEAD_SHA" in os.environ:
            provenance["pull_request_head_sha"] = os.environ["SIRKIT_PR_HEAD_SHA"]
        entries = ci.prepare_verify.collect() + ci.coverage_tests(ci.ROOT)
        report = merge(entries, ci.ROOT, args.parts, args.output, args.shard_count, provenance)
        print(ci.summary_markdown(report))
        return int(report["status"] != "PASS")
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
