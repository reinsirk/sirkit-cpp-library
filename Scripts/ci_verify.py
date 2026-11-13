#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""変更に関係する現在版のVerifyを、提出せずC++23で入力例照合する。"""
from __future__ import annotations

import argparse
import json
from pathlib import Path, PurePosixPath
import subprocess
import sys
import tempfile

try:
    from . import package_revised_libraries, prepare_verify
except ImportError:
    import package_revised_libraries
    import prepare_verify

ROOT = Path(__file__).resolve().parents[1]
LIBRARY_DIRS = {"Algorithm", "DataStructure", "Geometry", "Graph", "Internal", "Math", "String"}
GLOBAL_INPUTS = {
    "Scripts/prepare_verify.py", "Scripts/verify_graph_samples.py", "Scripts/library_layout.json",
    ".github/workflows/verify.yml", "Scripts/ci_verify.py",
}
EMBEDDED_DEPENDENTS = {
    "DataStructure/Dynamic_Bitset.cpp": {"Math/Binary_Matrix.cpp"},
    "Math/Matrix.cpp": {"Math/Gaussian_Elimination.cpp"},
    "Math/NTT.cpp": {"Math/Convolution.cpp", "Math/Formal_Power_Series.cpp",
                     "Math/Polynomial.cpp", "Math/Linear_Recurrence.cpp"},
    "Math/Formal_Power_Series.cpp": {"Math/Polynomial.cpp"},
}


def git_changed_paths(base: str, root: Path = ROOT) -> list[str]:
    """比較元をcommitへ解決し、未コミット・新規ファイルも含める。"""
    revision = subprocess.run(
        ["git", "rev-parse", "--verify", "--end-of-options", base + "^{commit}"],
        cwd=root, check=True, capture_output=True, text=True).stdout.strip()
    changed = subprocess.run(
        ["git", "diff", "--name-only", "--no-renames", "-z", revision, "--"],
        cwd=root, check=True, capture_output=True).stdout
    untracked = subprocess.run(
        ["git", "ls-files", "--others", "--exclude-standard", "-z"],
        cwd=root, check=True, capture_output=True).stdout
    return sorted({p.decode("utf-8", "surrogateescape") for p in (changed + untracked).split(b"\0") if p})


def dependencies(entry: dict, root: Path = ROOT) -> set[str]:
    result = set(entry["libraries"]) | {entry["driver"]}
    result.add("Verify/current/submissions/" + entry["id"] + ".cpp")
    pending = [entry["driver"]]
    while pending:
        path = pending.pop()
        source = (root / path).read_text()
        for match in prepare_verify.COMMON_INCLUDE.finditer(source):
            name = match[1]
            if name not in result:
                result.add(name)
                pending.append(name)
    return result


def select_entries(entries: list[dict], changed: list[str], *, requested: list[str] | None = None,
                   all_entries: bool = False, root: Path = ROOT) -> dict:
    ids = {entry["id"] for entry in entries}
    if len(ids) != len(entries):
        raise ValueError("Verify IDが重複しています")
    if requested:
        unknown = set(requested) - ids
        if unknown:
            raise ValueError("存在しないVerify ID: " + ", ".join(sorted(unknown)))
    paths = set(changed)
    reasons: dict[str, set[str]] = {identifier: set() for identifier in ids}
    if all_entries or requested:
        for identifier in (ids if all_entries else set(requested)):
            reasons[identifier].add("--all" if all_entries else "--id")
    else:
        universal = {p for p in paths if p in GLOBAL_INPUTS or
                     p.startswith("Verify/current/vendor/") or
                     (p.startswith("Scripts/generate_") and p.endswith(".py"))}
        if universal:
            for reason in reasons.values():
                reason.update(universal)
        expanded = set(paths)
        for original in paths:
            expanded.update(EMBEDDED_DEPENDENTS.get(original, set()))
        # 分割部品の参照が複数の単独版へ波及するため、同じ用途群を保守的に確認する。
        math_parts = {p for p in paths if p.startswith("Math/") and "/components/" in p}
        geometry_parts = {p for p in paths if p.startswith("Geometry/components/")}
        group_paths = {p for p in paths if p.startswith("Verify/current/groups/") and p.endswith(".json")}
        current_groups = {"Verify/current/groups/" + e["group"] + ".json" for e in entries}
        deleted_groups = group_paths - current_groups
        if deleted_groups:
            for reason in reasons.values():
                reason.update(deleted_groups)
        for entry in entries:
            relevant = dependencies(entry, root) & expanded
            if math_parts and any(p.startswith("Math/") for p in entry["libraries"]):
                relevant.update(math_parts)
            if geometry_parts and any(p.startswith("Geometry/") for p in entry["libraries"]):
                relevant.update(geometry_parts)
            group = "Verify/current/groups/" + entry["group"] + ".json"
            if group in group_paths:
                relevant.add(group)
            reasons[entry["id"]].update(relevant)
        covered = {name for entry in entries for name in entry["libraries"]}
        for path in paths:
            name = PurePosixPath(path)
            if name.parts and name.parts[0] in LIBRARY_DIRS and name.suffix == ".cpp" and path not in covered:
                # 削除済みのライブラリもあり得る。新規・現存の未割当は見逃さない。
                if (root / path).exists():
                    raise ValueError("Verifyへ未割当のライブラリ: " + path)
                for reason in reasons.values():
                    reason.add(path)
    selected = sorted(identifier for identifier, why in reasons.items() if why)
    return {"format": 1, "changed_paths": sorted(paths), "selected_ids": selected,
            "reasons": {identifier: sorted(reasons[identifier]) for identifier in selected},
            "coverage": "local_samples_checked_and_skip", "online_status": "not_evaluated"}


def write_report(path: Path, report: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    temporary.replace(path)


def run_selected(entries: list[dict], plan: dict, compiler: str, fallback: bool, output: Path) -> None:
    selected_ids = set(plan["selected_ids"])
    selected = [entry for entry in entries if entry["id"] in selected_ids]
    if len(selected) != len(selected_ids):
        raise ValueError("選択後にVerify IDが変化しました")
    report = {"format": 1, "plan": plan, "compiler": compiler, "standard_fallback": fallback,
              "online_status": "not_evaluated", "status": "running", "results": []}
    write_report(output, report)
    try:
        if selected:
            with tempfile.TemporaryDirectory(prefix="sirkit_ci_results_") as directory:
                raw = Path(directory) / "samples.json"
                try:
                    prepare_verify.run_tests(selected, compiler, fallback, raw)
                finally:
                    if raw.is_file():
                        report["results"] = json.loads(raw.read_text())["results"]
            actual = {(r["id"], r["skip_runtime_checks"]) for r in report["results"]}
            expected = {(identifier, skip) for identifier in selected_ids for skip in (False, True)}
            if actual != expected or len(report["results"]) != len(expected) or not all(
                    row.get("passed") for row in report["results"]):
                raise ValueError("Verify結果が選択対象・検査設定と一致しません")
        report["status"] = "PASS"
    except (Exception, SystemExit) as error:
        report["status"] = "FAIL"
        report["error"] = str(error)
        raise
    finally:
        write_report(output, report)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", default="origin/main", help="差分の比較元commit/ref")
    parser.add_argument("--id", action="append", help="Verify IDを明示指定する。複数指定可")
    parser.add_argument("--all", action="store_true", help="全件を確認する")
    parser.add_argument("--list", action="store_true", help="選択計画のJSONだけを出力する")
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--fallback", action="store_true")
    parser.add_argument("--skip-sync", action="store_true", help="既に照合済みの場合だけ同期確認を省く")
    parser.add_argument("--results", type=Path)
    args = parser.parse_args(argv)
    if args.all and args.id:
        parser.error("--allと--idは同時に指定できません")
    try:
        changed = [] if args.all or args.id else git_changed_paths(args.base)
        entries = prepare_verify.collect()
        plan = select_entries(entries, changed, requested=args.id, all_entries=args.all)
        if args.list:
            print(json.dumps(plan, ensure_ascii=False, indent=2))
            return 0
        if not args.skip_sync:
            package_revised_libraries.check_generated(ROOT)
        # render元と記録が同期していることを、テスト直前にも照合する。
        entries = prepare_verify.prepare(check=True)
        output = args.results or Path(tempfile.gettempdir()) / "sirkit-ci-verify" / (Path(args.compiler).name + ".json")
        run_selected(entries, plan, args.compiler, args.fallback, output)
        print(f"{len(plan['selected_ids'])}件のローカルVerify: PASS。結果: {output}")
        return 0
    except (OSError, ValueError, subprocess.SubprocessError, SystemExit) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
