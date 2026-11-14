#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""通常版のchecked VerifyをClangのASan/UBSanまたは分岐coverageで実行する。"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import sys
import tempfile

try:
    from . import prepare_verify
    from .ci_verify_summary import inline_code
except ImportError:
    import prepare_verify
    from ci_verify_summary import inline_code

ROOT = Path(__file__).resolve().parents[1]
LIBRARY_DIRS = {"Algorithm", "DataStructure", "Geometry", "Graph", "Internal", "Math", "String"}
SANITIZER_ENV = {
    "ASAN_OPTIONS": "detect_leaks=1:halt_on_error=1:abort_on_error=1:allocator_may_return_null=0",
    "UBSAN_OPTIONS": "halt_on_error=1:print_stacktrace=1",
    "LSAN_OPTIONS": "exitcode=1",
}
CHECK_GUARD = ('#if defined(SKIP_RUNTIME_CHECKS) || defined(NDEBUG)\n'
               '#error "instrumented Verify requires enabled runtime checks and assertions"\n#endif\n')


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def select_entries(entries: list[dict], requested: list[str] | None) -> list[dict]:
    identifiers = [entry["id"] for entry in entries]
    if len(set(identifiers)) != len(identifiers):
        raise ValueError("Verify IDが重複しています")
    if requested:
        unknown = set(requested) - set(identifiers)
        if unknown:
            raise ValueError("存在しないVerify ID: " + ", ".join(sorted(unknown)))
        return [entry for entry in entries if entry["id"] in requested]
    return list(entries)


def library_paths(entries: list[dict], root: Path) -> list[Path]:
    paths = set()
    root = root.resolve()
    for entry in entries:
        for name in entry["libraries"]:
            relative = PurePosixPath(name)
            if (relative.is_absolute() or ".." in relative.parts or not relative.parts
                    or relative.parts[0] not in LIBRARY_DIRS or relative.suffix != ".cpp"
                    or any(part in {"Fast", "Variants", "vendor", "submissions"} for part in relative.parts)):
                raise ValueError("通常版以外のlibrary pathは対象にできません")
            path = (root / name).resolve(strict=True)
            if not path.is_relative_to(root) or not path.is_file():
                raise ValueError("library pathがリポジトリ外を参照しています")
            paths.add(path)
    return sorted(paths)


def translation_unit(entry: dict, root: Path) -> str:
    library_paths([entry], root)
    # include順は提出版とそろえる。coverageの位置は貼付コードでなく原本へ帰属する。
    paths = [(root / name).resolve() for name in entry["libraries"]]
    driver = (root / entry["driver"]).resolve(strict=True)
    if not driver.is_relative_to(root.resolve()) or not driver.is_file():
        raise ValueError("driverはリポジトリ内の通常fixtureを指定してください")
    if any(part in {"Fast", "Variants", "vendor", "submissions"} for part in driver.relative_to(root.resolve()).parts):
        raise ValueError("配布派生版・連結提出版のdriverは対象外です")
    libraries = "".join('#include ' + json.dumps(str(path), ensure_ascii=False) + "\n" + CHECK_GUARD
                        for path in paths)
    return (CHECK_GUARD + libraries + "namespace sirkit {}\nusing namespace sirkit;\n" +
            '#include ' + json.dumps(str(driver), ensure_ascii=False) + "\n" + CHECK_GUARD)


def runtime_environment(mode: str, profile_pattern: str | None = None,
                        base: dict[str, str] | None = None) -> dict[str, str]:
    environment = dict(os.environ if base is None else base)
    if mode == "sanitizers":
        environment.update(SANITIZER_ENV)
        environment.pop("ASAN_SYMBOLIZER_PATH", None)
    elif mode == "coverage" and profile_pattern:
        environment["LLVM_PROFILE_FILE"] = profile_pattern
    else:
        raise ValueError("計測modeまたはprofile出力先が不正です")
    return environment


def command_version(command: str, clang: bool = False) -> tuple[str, int]:
    result = subprocess.run([command, "--version"], check=True, capture_output=True,
                            text=True, timeout=30)
    text = result.stdout + result.stderr
    match = re.search(r"clang version\s+(\d+)" if clang else r"LLVM version\s+(\d+)", text, re.I)
    if not match:
        raise ValueError("指定されたClang/LLVMの版を確認できません: " + command)
    return text.strip(), int(match[1])


def gcc_install_arguments(compiler: str, explicit: str | None) -> list[str]:
    if explicit:
        path = Path(explicit).resolve(strict=True)
        if not path.is_dir():
            raise ValueError("gcc-install-dirはディレクトリを指定してください")
        return ["--gcc-install-dir=" + str(path)]
    found = subprocess.run([compiler, "-print-file-name=libstdc++.so"], check=True,
                           capture_output=True, text=True, timeout=30).stdout.strip()
    if Path(found).is_absolute() and Path(found).is_file():
        return []
    # Clangの自動選択が未完成のGCCを拾う環境では、利用可能なsystem g++へ合わせる。
    driver = shutil.which("g++", path=os.defpath) or shutil.which("g++")
    if driver:
        found = subprocess.run([driver, "-print-libgcc-file-name"], check=True,
                               capture_output=True, text=True, timeout=30).stdout.strip()
        if Path(found).is_absolute() and Path(found).is_file():
            return ["--gcc-install-dir=" + str(Path(found).parent)]
    return []


def make_acl_overlay(root: Path, destination: Path) -> None:
    for header in (root / "Verify/current/vendor/atcoder").glob("*.hpp"):
        wrapper = destination / "atcoder" / header.stem
        wrapper.parent.mkdir(parents=True, exist_ok=True)
        wrapper.write_text('#include ' + json.dumps(str(header.resolve()), ensure_ascii=False) + "\n", encoding="utf-8")


def filter_coverage(export: dict, allowed: list[Path]) -> dict:
    """原本の許可リストだけを残し、分母もそのfile summaryから再集計する。"""
    selected = {str(path.resolve()) for path in allowed}
    files = {}
    for data in export["data"]:
        for item in data["files"]:
            name = str(Path(item["filename"]).resolve())
            if name not in selected:
                continue
            if name in files:
                raise ValueError("coverageの原本fileが重複しています")
            files[name] = {**item, "filename": name}
    totals = {}
    for item in files.values():
        for name, metric in item["summary"].items():
            total = totals.setdefault(name, {"count": 0, "covered": 0})
            total["count"] += metric["count"]
            total["covered"] += metric["covered"]
    for total in totals.values():
        total["percent"] = 100 * total["covered"] / total["count"] if total["count"] else 0
    return {"type": export["type"], "version": export["version"],
            "data": [{"files": [files[name] for name in sorted(files)], "totals": totals}]}


def source_coverage(export: dict, lcov: str, allowed: list[Path]) -> dict:
    """行はLLVMのDA、分岐は原本regionのtrue/false。TU固有のfile IDは使わない。"""
    filtered = filter_coverage(export, allowed)
    files = {item["filename"]: {"filename": item["filename"], "lines": {}, "branches": {}}
             for item in filtered["data"][0]["files"]}
    current = None
    for record in lcov.splitlines():
        if record.startswith("SF:"):
            current = files.get(str(Path(record[3:]).resolve()))
        elif record == "end_of_record":
            current = None
        elif current is not None and record.startswith("DA:"):
            line, count, *_ = record[3:].split(",")
            line, count = int(line), int(count)
            if line <= 0 or count < 0:
                raise ValueError("coverageの行counterが不正です")
            current["lines"][line] = current["lines"].get(line, False) or count > 0
    for item in filtered["data"][0]["files"]:
        branches = files[item["filename"]]["branches"]
        # expansions内部のbranchは除外する。直接原本に記述されたconditionだけを集計。
        for branch in item.get("branches", []):
            region = tuple(branch[:4])
            old = branches.get(region, (False, False))
            branches[region] = (old[0] or branch[4] > 0, old[1] or branch[5] > 0)
    return {"files": [{"filename": name, "lines": sorted(item["lines"].items()),
                       "branches": [list(region) + list(hits) for region, hits in sorted(item["branches"].items())]}
                      for name, item in sorted(files.items())]}


def coverage_union(exports: list[dict]) -> dict:
    combined = {}
    for export in exports:
        for item in export["files"]:
            target = combined.setdefault(item["filename"], {"lines": {}, "branches": {}})
            for line, hit in item["lines"]:
                target["lines"][line] = target["lines"].get(line, False) or hit
            for *region, true_hit, false_hit in item["branches"]:
                key = tuple(region)
                old = target["branches"].get(key, (False, False))
                target["branches"][key] = (old[0] or true_hit, old[1] or false_hit)
    def metric(count, covered):
        return {"count": count, "covered": covered, "percent": 100 * covered / count if count else 0}
    files = []
    for name, item in sorted(combined.items()):
        summary = {"lines": metric(len(item["lines"]), sum(item["lines"].values())),
                   "branches": metric(2 * len(item["branches"]), sum(sum(hits) for hits in item["branches"].values()))}
        files.append({"filename": name, "lines": sorted(item["lines"].items()),
                      "branches": [list(region) + list(hits) for region, hits in sorted(item["branches"].items())],
                      "summary": summary})
    totals = {key: metric(sum(item["summary"][key]["count"] for item in files),
                          sum(item["summary"][key]["covered"] for item in files))
              for key in ("lines", "branches")}
    return {"format": 1, "type": "sirkit.source-coverage-union", "files": files, "totals": totals,
            "definition": "Per-TU LLVM LCOV DA lines and direct-source condition regions; boolean union across TUs and template instances; macro expansion interiors excluded."}


def write_coverage_html(coverage: dict, root: Path, directory: Path) -> None:
    directory.mkdir()
    style = ('<style>body{font-family:system-ui}table{border-collapse:collapse}td,th{padding:.2em .6em;'
             'border:1px solid #aaa}pre{margin:0}.hit{background:#dfd}.miss{background:#fdd}</style>')
    header = '<!doctype html><meta charset="utf-8">' + style
    index = [header, '<title>原本ソースの到達和集合</title><h1>原本ソースの到達和集合</h1>',
             '<p>TUごとの行・直接記述条件の到達和集合。型別の保証ではなく、マクロ展開内部は対象外。</p>',
             '<table><tr><th>原本</th><th>行</th><th>分岐</th></tr>']
    for number, item in enumerate(coverage["files"]):
        relative = html.escape(str(Path(item["filename"]).relative_to(root)))
        target = f"source_{number:04}.html"
        metrics = item["summary"]
        index.append(f'<tr><td><a href="{target}">{relative}</a></td>' + ''.join(
            f'<td>{metrics[key]["covered"]}/{metrics[key]["count"]}</td>' for key in ("lines", "branches")) + '</tr>')
        lines = dict(item["lines"])
        branches = {}
        for line, column, end_line, end_column, true_hit, false_hit in item["branches"]:
            branches.setdefault(line, []).append(f'{line}:{column}–{end_line}:{end_column} '
                                                  f'T:{"✓" if true_hit else "×"} F:{"✓" if false_hit else "×"}')
        page = [header, f'<title>{relative}</title><p><a href="index.html">一覧</a></p><h1>{relative}</h1>',
                '<table><tr><th>行</th><th>到達</th><th>直接記述条件</th><th>原本</th></tr>']
        for line, text in enumerate(Path(item["filename"]).read_text(encoding="utf-8").splitlines(), 1):
            state = 'hit' if lines.get(line) else 'miss' if line in lines else ''
            hit = '✓' if lines.get(line) else '×' if line in lines else '—'
            conditions = '<br>'.join(html.escape(condition) for condition in branches.get(line, []))
            page.append(f'<tr class="{state}" id="L{line}"><td>{line}</td><td>{hit}</td>'
                        f'<td>{conditions}</td><td><pre>{html.escape(text)}</pre></td></tr>')
        (directory / target).write_text(''.join(page) + '</table>\n', encoding="utf-8")
    (directory / "index.html").write_text(''.join(index) + '</table>\n', encoding="utf-8")


def summary_markdown(report: dict) -> str:
    title = "ASan + UBSan" if report["mode"] == "sanitizers" else "C++ 分岐カバレッジ"
    executed = sum(bool(row.get("samples")) for row in report["results"])
    local = sum(bool(row.get("samples")) and row["local_fixture"] for row in report["results"])
    major = report.get("tools", {}).get("compiler", {}).get("major")
    compiler = f"Clang {major}・" if type(major) is int else ""
    lines = ["### " + title, "", "状態: " + report["status"], "",
             f"checked実行: {executed}/{len(report['selected_ids'])} ID（うちローカル補足 {local}件）。",
             compiler + "通常版・C++23・assert有効。オンライン提出の判定は含みません。"]
    if "coverage_totals" in report:
        lines += ["", "| 原本のみ | 到達/計測 | 率 |", "| --- | ---: | ---: |"]
        for key, name in [("lines", "行"), ("branches", "分岐")]:
            metric = report["coverage_totals"].get(key, {"covered": 0, "count": 0, "percent": 0})
            percent = f"{metric['percent']:.1f}%" if metric["count"] else "計測なし"
            lines.append(f"| {name} | {metric['covered']}/{metric['count']} | {percent} |")
        lines += ["", "HTML: artifact内の `html/index.html`。原本別の値は `coverage.json`。"]
        lines += ["", "各TUで別々に計測し、原本の行番号・条件regionのtrue/false到達を和集合にします。"
                  "同じ行・条件は型やTUをまたいでも1回だけ数えます。"
                  "型ごとの被覆保証ではなく、マクロ展開内部の条件は集計しません。"]
    failed = [row["id"] for row in report["results"] if row.get("status") != "PASS"]
    if failed:
        lines += ["", "未成功ID: " + ", ".join(inline_code(name) for name in failed[:8])]
    lines += ["", "登録サンプルとローカル補足（独立参照を含む）で実行した範囲の計測です。"
              "未実体化template・未選択ライブラリは分母に含みません。"
              "Fast/Variants・vendor・driver/main・連結提出コードも集計対象外です。"]
    return "\n".join(lines) + "\n"


def write_report(output: Path, report: dict) -> None:
    temporary = output / "results.json.tmp"
    temporary.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    temporary.replace(output / "results.json")
    (output / "summary.md").write_text(summary_markdown(report), encoding="utf-8")


def coverage_command(command: list[str], log: Path, timeout: float) -> subprocess.CompletedProcess:
    result = subprocess.run(command, capture_output=True, text=True, errors="replace", timeout=timeout)
    log.write_text(result.stderr + ("\nstdout:\n" + result.stdout if result.returncode else ""), encoding="utf-8")
    result.check_returncode()
    if re.search(r"mismatched data|hash mismatch|profile data may be out of date", result.stderr, re.I):
        raise ValueError("LLVM profileが一致しません: " + str(log))
    return result


def run(entries: list[dict], *, root: Path, output: Path, mode: str, compiler: str,
        llvm_profdata: str = "llvm-profdata", llvm_cov: str = "llvm-cov",
        gcc_install_dir: str | None = None, timeout: float = 120,
        compile_timeout: float = 180, available: list[dict] | None = None) -> dict:
    root, output = root.resolve(), output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    report = {"format": 1, "mode": mode, "status": "running", "checked": True,
              "selected_ids": [entry["id"] for entry in entries], "results": [], "commands": [],
              "available_entries": len(available if available is not None else entries),
              "available_local_fixtures": sum(not entry.get("problem_url")
                                              for entry in (available if available is not None else entries)),
              "online_status": "not_evaluated", "artifacts_exclude_binaries_and_raw_profiles": True}
    if mode == "sanitizers":
        report["runtime_options"] = SANITIZER_ENV
    write_report(output, report)
    try:
        if not entries:
            raise ValueError("計測対象がありません")
        allowed = library_paths(entries, root)
        version, major = command_version(compiler, clang=True)
        report["tools"] = {"compiler": {"command": compiler, "version": version, "major": major}}
        if mode == "coverage":
            for name, tool in [("llvm_profdata", llvm_profdata), ("llvm_cov", llvm_cov)]:
                version, tool_major = command_version(tool)
                report["tools"][name] = {"command": tool, "version": version, "major": tool_major}
                if tool_major != major:
                    raise ValueError("Clang/llvm-profdata/llvm-covは同じmajor版が必要です")
        elif mode != "sanitizers":
            raise ValueError("不明な計測modeです")
        compiler_options = gcc_install_arguments(compiler, gcc_install_dir)
        report["library_source_hashes"] = {str(path.relative_to(root)): digest(path) for path in allowed}
        watched = set(allowed) | {(root / entry["driver"]).resolve() for entry in entries}
        pending = list(watched)
        while pending:
            path = pending.pop()
            for match in prepare_verify.COMMON_INCLUDE.finditer(path.read_text(encoding="utf-8")):
                dependency = (root / match[1]).resolve(strict=True)
                if not dependency.is_relative_to(root):
                    raise ValueError("共通ヘッダーがリポジトリ外を参照しています")
                if dependency not in watched:
                    watched.add(dependency)
                    pending.append(dependency)
        report["all_source_hashes"] = {str(path.relative_to(root)): digest(path) for path in sorted(watched)}
        report["vendor_source_hashes"] = {str(path.relative_to(root)): digest(path)
                                           for path in sorted((root / "Verify/current/vendor").rglob("*"))
                                           if path.is_file()}
        (output / "sources").mkdir()
        (output / "logs").mkdir()
        with tempfile.TemporaryDirectory(prefix="sirkit_instrumented_") as temporary:
            work = Path(temporary)
            overlay = work / "include"
            make_acl_overlay(root, overlay)
            coverage_exports = []
            if mode == "coverage":
                (output / "coverage_entries").mkdir()
            for index, entry in enumerate(entries):
                label = f"entry_{index:04}"
                source = output / "sources" / (label + ".cpp")
                source.write_text(translation_unit(entry, root), encoding="utf-8")
                binary = work / label
                profiles = []
                merged = work / (label + ".profdata")
                flags = (["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                          "-fno-omit-frame-pointer"] if mode == "sanitizers" else
                         ["-O0", "-g", "-fprofile-instr-generate", "-fcoverage-mapping"])
                command = [compiler, *compiler_options, "-std=c++23", *flags,
                           "-UNDEBUG", "-USKIP_RUNTIME_CHECKS", "-I", str(overlay), "-I", str(root),
                           "-I", str(root / "Verify/current/vendor"), str(source), "-o", str(binary)]
                row = {"id": entry["id"], "local_fixture": not entry.get("problem_url"),
                       "status": "compiling", "command": command, "source_sha256": digest(source),
                       "driver": entry["driver"], "driver_sha256": digest(root / entry["driver"]),
                       "samples": []}
                report["results"].append(row)
                write_report(output, report)
                try:
                    built = subprocess.run(command, capture_output=True, text=True, errors="replace",
                                           timeout=compile_timeout)
                    row["compile_returncode"] = built.returncode
                    (output / "logs" / (label + "_compile.txt")).write_text(built.stdout + built.stderr,
                                                                             encoding="utf-8")
                    row["compile_log"] = "logs/" + label + "_compile.txt"
                    if built.returncode:
                        row["status"] = "FAIL"
                        continue
                    for sample_index, sample in enumerate(entry.get("samples", [])):
                        pattern = str(work / f"{label}_{sample_index}-%p.profraw")
                        environment = runtime_environment(mode, pattern)
                        result = subprocess.run([str(binary)], input=sample["input"], capture_output=True,
                                                text=True, errors="replace", env=environment, timeout=timeout)
                        sample_row = {"index": sample_index, "returncode": result.returncode,
                                      "passed": result.returncode == 0 and prepare_verify.matches(entry, sample, result.stdout)}
                        log = f"logs/{label}_sample_{sample_index}.txt"
                        (output / log).write_text("stdout:\n" + result.stdout + "\nstderr:\n" + result.stderr,
                                                  encoding="utf-8")
                        sample_row["log"] = log
                        if mode == "coverage":
                            generated = list(work.glob(f"{label}_{sample_index}-*.profraw"))
                            sample_row["profile_written"] = bool(generated) and all(path.stat().st_size for path in generated)
                            sample_row["passed"] &= sample_row["profile_written"]
                            profiles.extend(generated)
                        row["samples"].append(sample_row)
                        write_report(output, report)
                    row["status"] = "PASS" if row["samples"] and all(item["passed"] for item in row["samples"]) else "FAIL"
                    if mode == "coverage" and row["status"] == "PASS":
                        command = [llvm_profdata, "merge", "-sparse", *(str(path) for path in profiles), "-o", str(merged)]
                        report["commands"].append(command)
                        coverage_command(command, output / "logs" / (label + "_merge.txt"), compile_timeout)
                        entry_sources = library_paths([entry], root)
                        common = [str(binary), "-instr-profile=" + str(merged), "-skip-functions"]
                        command = [llvm_cov, "export", *common, "--sources", *(str(path) for path in entry_sources)]
                        report["commands"].append(command)
                        exported = coverage_command(command, output / "logs" / (label + "_export.txt"), compile_timeout)
                        command = [llvm_cov, "export", *common, "-format=lcov", "--sources", *(str(path) for path in entry_sources)]
                        report["commands"].append(command)
                        lcov = coverage_command(command, output / "logs" / (label + "_lcov.txt"), compile_timeout)
                        measured = source_coverage(json.loads(exported.stdout), lcov.stdout, entry_sources)
                        row["coverage_path"] = "coverage_entries/" + label + ".json"
                        (output / row["coverage_path"]).write_text(json.dumps(measured) + "\n", encoding="utf-8")
                        coverage_exports.append(measured)
                except (OSError, ValueError, subprocess.SubprocessError) as error:
                    row.update(status="FAIL", error=str(error))
                finally:
                    for path in [binary, merged, *profiles]:
                        path.unlink(missing_ok=True)
                    write_report(output, report)
                    print(f"{index + 1}/{len(entries)} {entry['id']}: {row['status']}", flush=True)
            if mode == "coverage":
                coverage = coverage_union(coverage_exports)
                (output / "coverage.json").write_text(json.dumps(coverage) + "\n", encoding="utf-8")
                report["coverage_totals"] = coverage["totals"]
                report["coverage_definition"] = coverage["definition"]
                report["coverage_complete"] = len(coverage_exports) == len(entries)
                report["unmapped_sources"] = sorted(set(map(str, allowed)) -
                                                    {item["filename"] for item in coverage["files"]})
                if not report["coverage_totals"]["lines"]["count"]:
                    raise ValueError("通常版本体の計測可能な行がありません")
                write_coverage_html(coverage, root, output / "html")
        report["source_hashes_after"] = {str(path.relative_to(root)): digest(path) for path in sorted(watched)}
        if report["source_hashes_after"] != report["all_source_hashes"]:
            raise ValueError("計測中に原本またはdriver/共通ヘッダーが変更されました")
        report["status"] = "PASS" if len(report["results"]) == len(entries) and all(row["status"] == "PASS" for row in report["results"]) else "FAIL"
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        report.update(status="FAIL", error=str(error))
    finally:
        if report["status"] == "running":
            report["status"] = "FAIL"
        write_report(output, report)
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("sanitizers", "coverage"), required=True)
    parser.add_argument("--compiler", default="clang++")
    parser.add_argument("--llvm-profdata", default="llvm-profdata")
    parser.add_argument("--llvm-cov", default="llvm-cov")
    parser.add_argument("--gcc-install-dir")
    parser.add_argument("--id", action="append", help="対象ID。省略時はローカル補足を含む全件")
    parser.add_argument("--output", type=Path, required=True, help="未作成の出力ディレクトリ")
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--compile-timeout", type=float, default=180)
    parser.add_argument("--cpu", type=int)
    args = parser.parse_args(argv)
    try:
        if args.timeout <= 0 or args.compile_timeout <= 0:
            raise ValueError("timeoutは正数で指定してください")
        if args.cpu is not None:
            os.sched_setaffinity(0, {args.cpu})
        available = prepare_verify.collect()
        entries = select_entries(available, args.id)
        report = run(entries, root=ROOT, output=args.output, mode=args.mode, compiler=args.compiler,
                     llvm_profdata=args.llvm_profdata, llvm_cov=args.llvm_cov,
                     gcc_install_dir=args.gcc_install_dir, timeout=args.timeout,
                     compile_timeout=args.compile_timeout, available=available)
        return int(report["status"] != "PASS")
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
