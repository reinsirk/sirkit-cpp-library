#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""代表APIの正常系・期待した例外・失敗後の状態をcheckedで照合する。"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = "Verify/ci/contracts.cpp"
LIBRARIES = (
    "DataStructure/Fenwick_Tree.cpp", "DataStructure/Segment_Tree.cpp",
    "DataStructure/Lazy_Segment_Tree.cpp", "DataStructure/union_find.cpp",
    "Graph/Low_Link.cpp", "String/RollingHash.cpp",
)
FLAGS = ["-std=c++23", "-O2", "-UNDEBUG", "-USKIP_RUNTIME_CHECKS"]
FALLBACK_FLAGS = ["-U__SIZEOF_INT128__", "-DSIRKIT_FORCE_STANDARD_ARITHMETIC",
                  "-DSIRKIT_LINE_CONTAINER_FORCE_PORTABLE", "-DSIRKIT_DISABLE_SIMD"]
EXCEPTIONS = {"std::out_of_range", "std::invalid_argument", "std::logic_error", "std::length_error"}


def now() -> str:
    return datetime.now(timezone.utc).isoformat()


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def snapshot(root: Path) -> dict[str, str]:
    root = root.resolve()
    result = {}
    for relative in (*LIBRARIES, FIXTURE):
        path = (root / relative).resolve(strict=True)
        if not path.is_relative_to(root) or not path.is_file():
            raise ValueError("検査対象がリポジトリ外です: " + relative)
        result[relative] = sha(path)
    return result


def unique_pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("結果JSONのキーが重複しています")
        result[key] = value
    return result


def parse_results(text: str) -> list[dict]:
    value = json.loads(text, object_pairs_hook=unique_pairs)
    if not isinstance(value, dict) or type(value.get("format")) is not int or value["format"] != 1:
        raise ValueError("契約テストの結果形式が不正です")
    rows = value.get("results")
    if not isinstance(rows, list) or len(rows) != len(LIBRARIES):
        raise ValueError("契約テストの対象が欠落しています")
    seen = set()
    for row in rows:
        if not isinstance(row, dict) or row.get("library") not in LIBRARIES or row["library"] in seen:
            raise ValueError("契約テストの原本が不明または重複しています")
        seen.add(row["library"])
        if row.get("status") != "PASS":
            raise ValueError("契約テストが成功していません")
        for key in ("normal_checks", "expected_failures"):
            if type(row.get(key)) is not int or row[key] <= 0:
                raise ValueError("正常系・失敗系の実行件数がありません")
        cases = row.get("cases")
        if not isinstance(cases, list) or len(cases) != row["expected_failures"]:
            raise ValueError("期待した例外の内訳が一致しません")
        names = set()
        for case in cases:
            if (not isinstance(case, dict) or not isinstance(case.get("name"), str)
                    or not re.fullmatch(r"[a-z][a-z0-9_]*", case["name"])
                    or case["name"] in names or not isinstance(case.get("expected_exception"), str)
                    or case["expected_exception"] not in EXCEPTIONS
                    or case.get("status") != "PASS" or case.get("post_failure_state_checked") is not True):
                raise ValueError("例外型・失敗後の状態の照合が不正です")
            names.add(case["name"])
    return rows


def provenance(root: Path, environment: dict[str, str] | None = None) -> dict:
    env = os.environ if environment is None else environment
    try:
        result = subprocess.run(["git", "rev-parse", "HEAD"], cwd=root, capture_output=True,
                                text=True, timeout=15, check=True).stdout.strip()
        revision = result if re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", result) else None
    except (OSError, subprocess.SubprocessError):
        revision = None
    return {"revision": revision, "github_sha": env.get("GITHUB_SHA") or None,
            "repository": env.get("GITHUB_REPOSITORY") or None,
            "run_id": env.get("GITHUB_RUN_ID") or None,
            "run_attempt": env.get("GITHUB_RUN_ATTEMPT") or None,
            "event_name": env.get("GITHUB_EVENT_NAME") or None,
            "scope": "source hashes bind the tested working tree; commit alone is not a source-match proof"}


def compiler_command(compiler: str, gcc_install_dir: str | None) -> list[str]:
    found = shutil.which(compiler)
    if not found:
        raise ValueError("コンパイラがありません: " + compiler)
    command = [found]
    if gcc_install_dir:
        if "clang" not in Path(found).name:
            raise ValueError("gcc-install-dirはClangだけで指定してください")
        directory = Path(gcc_install_dir).resolve(strict=True)
        if not directory.is_dir():
            raise ValueError("gcc-install-dirはディレクトリを指定してください")
        command.append("--gcc-install-dir=" + str(directory))
    elif "clang" in Path(found).name:
        gcc = shutil.which("g++")
        if gcc:
            result = subprocess.run([gcc, "-print-libgcc-file-name"], capture_output=True,
                                    text=True, check=True, timeout=15)
            library = Path(result.stdout.strip())
            if library.is_absolute() and library.is_file():
                command.append("--gcc-install-dir=" + str(library.parent))
    return command


def summary(report: dict) -> str:
    lines = ["## checked APIの正常系・失敗系", "",
             "代表6本の公開APIを、入力検査とassertを有効にして照合します。全ライブラリの全APIを網羅する検査ではありません。",
             "失敗系は期待したC++例外の型と、その後の状態・再利用を照合します。異常終了・別の例外は不合格です。", "",
             "状態: " + report["status"], "", "| 原本 | 正常系・状態照合 | 期待した例外 |",
             "| --- | ---: | ---: |"]
    for row in report["results"]:
        lines.append(f"| {row['library']} | {row['normal_checks']} | {row['expected_failures']} |")
    if report["status"] != "PASS":
        lines += ["", "ビルド・実行・件数・ソース一致のいずれかを確認できません。詳細はresult.jsonを参照してください。"]
    lines += ["", "オンラインAC・性能測定とは別の結果です。"]
    return "\n".join(lines) + "\n"


def write_report(output: Path, report: dict) -> None:
    output.mkdir(parents=True, exist_ok=True)
    temporary = output / "result.json.tmp"
    temporary.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    temporary.replace(output / "result.json")
    (output / "summary.md").write_text(summary(report), encoding="utf-8")


def run(*, root: Path, output: Path, compiler: str, fallback: bool = False,
        gcc_install_dir: str | None = None, timeout: float = 120) -> dict:
    root, output = root.resolve(), output.resolve()
    report = {"format": 1, "type": "sirkit.contracts", "status": "RUNNING", "checked": True,
              "online_status": "not_evaluated", "started_at_utc": now(), "completed_at_utc": None,
              "standard_fallback": fallback, "flags": FLAGS + (FALLBACK_FLAGS if fallback else []),
              "driver": FIXTURE, "source_hashes": {}, "source_hashes_after": {}, "results": [],
              "provenance": provenance(root), "scope": "representative_checked_api_contracts"}
    write_report(output, report)
    try:
        if not math.isfinite(timeout) or timeout <= 0:
            raise ValueError("timeoutは正数を指定してください")
        report["source_hashes"] = snapshot(root)
        tool = compiler_command(compiler, gcc_install_dir)
        version = subprocess.run([tool[0], "--version"], capture_output=True, text=True,
                                 check=True, timeout=30).stdout.splitlines()[0]
        report["compiler"] = {"command": tool, "version": version}
        with tempfile.TemporaryDirectory(prefix="sirkit-contracts-") as directory:
            binary = Path(directory) / "contracts"
            command = [*tool, *report["flags"], "-I", str(root), str(root / FIXTURE), "-o", str(binary)]
            report["command"] = command
            built = subprocess.run(command, capture_output=True, text=True, errors="replace", timeout=timeout)
            report["compile_returncode"] = built.returncode
            report["compile_diagnostics"] = (built.stdout + built.stderr)[-16000:]
            if built.returncode:
                raise ValueError("checked契約テストのビルドが失敗しました")
            executed = subprocess.run([str(binary)], capture_output=True, text=True, errors="replace", timeout=timeout)
            report["returncode"] = executed.returncode
            report["stderr"] = executed.stderr[-16000:]
            if executed.returncode:
                raise ValueError("契約テストが異常終了しました（期待した失敗として数えません）")
            report["results"] = parse_results(executed.stdout)
        report["source_hashes_after"] = snapshot(root)
        if report["source_hashes_after"] != report["source_hashes"]:
            raise ValueError("契約テスト中に原本・fixtureが変わりました")
        report["status"] = "PASS"
    except (OSError, ValueError, IndexError, subprocess.SubprocessError) as error:
        report.update(status="FAIL", error=str(error))
        try:
            report["source_hashes_after"] = snapshot(root)
        except (OSError, ValueError):
            pass
    finally:
        report["completed_at_utc"] = now()
        write_report(output, report)
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--fallback", action="store_true")
    parser.add_argument("--gcc-install-dir")
    parser.add_argument("--timeout", type=float, default=120)
    args = parser.parse_args(argv)
    report = run(root=ROOT, output=args.output, compiler=args.compiler, fallback=args.fallback,
                 gcc_install_dir=args.gcc_install_dir, timeout=args.timeout)
    print(report["status"] + ": " + str(args.output / "result.json"))
    return int(report["status"] != "PASS")


if __name__ == "__main__":
    raise SystemExit(main())
