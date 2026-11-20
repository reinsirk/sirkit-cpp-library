#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""配布版の単独ビルドと動作を検証する。"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time

try:
    from .ci_instrumented import gcc_install_arguments, make_acl_overlay
except ImportError:
    from ci_instrumented import gcc_install_arguments, make_acl_overlay

ROOT = Path(__file__).resolve().parents[1]
COMPILERS = ("GCC", "Clang", "MSVC")
FORCE_STANDARD = ["-DSIRKIT_DISABLE_SIMD", "-DSIRKIT_FORCE_STANDARD_ARITHMETIC",
                  "-DSIRKIT_FORCE_STANDARD_MUL_OVERFLOW", "-DSIRKIT_FRACTION_DISABLE_INT128",
                  "-DGEO_DISABLE_INT128", "-DSIRKIT_CONVOLUTION_DISABLE_INT128"]
RUNTIME_TESTS = {
    "DataStructure/Binary_Trie.cpp": ("binary_trie_sole_test.cpp", "TRIE_SOURCE"),
    "DataStructure/Fenwick_Tree.cpp": ("fenwick_tree_reference_test.cpp", "SIRKIT_FENWICK_TEST_SOURCE"),
    "DataStructure/Hash_map.cpp": ("hash_map_borrowed_key_test.cpp", "HASH_MAP_HEADER"),
    "DataStructure/Hash_set.cpp": ("hash_set_round2_erased_borrowed_key_test.cpp", "HEADER"),
    "DataStructure/Lazy_Segment_Tree.cpp": ("lazy_segment_tree_reference_test.cpp", "SIRKIT_LAZY_SEGMENT_TEST_SOURCE"),
    "DataStructure/Node.cpp": ("node_cycle_test.cpp", "SIRKIT_NODE_SOURCE"),
    "DataStructure/Persistent_Segment_Tree.cpp": ("variant_persistent_segment_tree_test.cpp", "HEADER"),
    "DataStructure/Segment_Tree.cpp": ("segment_tree_reference_test.cpp", "SIRKIT_SEGMENT_TEST_SOURCE"),
    "DataStructure/Wavelet_Matrix.cpp": ("wavelet_matrix_reference_test.cpp", "WAVELET_SOURCE"),
    "DataStructure/implicit_treap.cpp": ("implicit_treap_split_test.cpp", "HEADER"),
    "Graph/SCC.cpp": ("scc_acl_api_test.cpp", "SIRKIT_SCC_TEST_SOURCE"),
    "Math/NTT.cpp": ("ntt_reference_test.cpp", "NTT_HEADER"),
    "String/Aho_Corasick.cpp": ("aho_corasick_reference_test.cpp", "AHO_HEADER"),
}
FAST_RUNTIME_TESTS = {
    "DataStructure/Binary_Trie.cpp": ("fast_binary_trie_boundaries_test.cpp", "FAST_TRIE_SOURCE"),
    "DataStructure/Node.cpp": ("fast_node_reference_test.cpp", "FAST_NODE_SOURCE"),
}


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def acl_fingerprints(include_dirs: list[Path]) -> dict[str, str]:
    hashes = {}
    for path in include_dirs:
        acl = path / "atcoder"
        if acl.is_dir():
            contents = [(file.relative_to(acl).as_posix(), digest(file.read_bytes()))
                        for file in sorted(acl.rglob("*")) if file.is_file()]
            hashes[str(path)] = digest(json.dumps(contents, separators=(",", ":")).encode())
    return hashes


def selected_variants(entries: list[dict], compilers: tuple[str, ...], sources: set[str] | None) -> list[tuple[str, dict]]:
    return [(compiler, entry) for entry in entries if sources is None or entry["source"] in sources
            for compiler in compilers if entry["compiler"] in ("Portable", compiler)]


def build_settings(compiler: str, fast: bool, syntax_only: bool) -> list[tuple[str, list[str]]]:
    settings: list[tuple[str, list[str]]] = [("default", [])]
    if compiler != "MSVC" and not syntax_only:
        settings.extend([("native", ["-march=native"]), ("standard", FORCE_STANDARD)])
    if not fast:
        settings.append(("skip", ["/DSKIP_RUNTIME_CHECKS" if compiler == "MSVC" else "-DSKIP_RUNTIME_CHECKS"]))
    return settings


def compiler_command(compiler: str, include_dirs: list[Path], gcc_install_dir: str | None,
                     *, gcc: str = "g++", clang: str = "clang++") -> list[str]:
    if compiler == "MSVC":
        executable = shutil.which("cl.exe")
        if executable is None:
            raise ValueError("cl.exeが見つかりません。Visual Studioの開発者コマンドプロンプトを使用してください")
        includes = [argument for path in include_dirs for argument in ("/I", str(path))]
        return [executable, "/nologo", "/std:c++23preview", "/permissive-", "/Zc:__cplusplus", "/Zc:preprocessor",
                "/utf-8", "/EHsc", *includes]
    driver = gcc if compiler == "GCC" else clang
    clang_arguments = gcc_install_arguments(driver, gcc_install_dir) if compiler == "Clang" else []
    return [driver, *clang_arguments, "-std=c++23", "-O2", *("-I" + str(path) for path in include_dirs)]


def compiler_version(command: list[str], compiler: str, work: Path) -> str:
    if compiler != "MSVC":
        return subprocess.check_output([command[0], "--version"], text=True, timeout=30)
    probe = work / "probe.cpp"
    probe.write_text(
        "#if !defined(_MSC_VER) || defined(__clang__) || _MSVC_LANG < 202302L\n"
        '#error "Native MSVC with C++23 is required"\n#endif\n', encoding="utf-8")
    run = subprocess.run([*command, "/Bv", "/WX", "/Zs", str(probe)], capture_output=True,
                         text=True, errors="replace", timeout=30)
    if run.returncode:
        raise ValueError("MSVCのバージョン確認に失敗しました:\n" + run.stdout + run.stderr)
    return (run.stdout + run.stderr).strip()


def runtime_source(library: Path, fixture: Path, fixture_text: str, macro: str, fast: bool) -> str:
    library_name = json.dumps(library.as_posix(), ensure_ascii=False)
    fixture_name = json.dumps(fixture.as_posix(), ensure_ascii=False)
    lines = [f"#define {macro} {library_name}",
             '#ifdef NDEBUG\n#error "runtime tests require assertions"\n#endif']
    if fast:
        lines.append('#ifdef SKIP_RUNTIME_CHECKS\n#error "build the fast library without a skip flag"\n#endif')
        lines.append(f"#include {macro}")
        lines.append('#ifdef SKIP_RUNTIME_CHECKS\n#error "the fast library exported a skip flag"\n#endif')
        # ライブラリ本体の後で定義し、共用テストの無効入力だけを省く。
        lines.append("#define SKIP_RUNTIME_CHECKS")
    lines.append(f"#line 1 {fixture_name}")
    return "\n".join(lines) + "\n" + fixture_text


def run_runtime_test(compiler: str, command: list[str], source: Path, binary: Path) -> dict:
    if compiler == "MSVC":
        flags = ["/O2", "/UNDEBUG", "/Fo" + str(binary.with_suffix(".obj")), "/Fe" + str(binary)]
    else:
        flags = ["-UNDEBUG", "-o", str(binary)]
    build = subprocess.run([*command, *flags, str(source)], cwd=binary.parent,
                           capture_output=True, timeout=180)
    build_log = (build.stdout + build.stderr).decode(errors="replace")
    result = {"compile_returncode": build.returncode, "diagnostics": build_log,
              "run_returncode": None, "passed": False}
    if build.returncode == 0:
        run = subprocess.run([str(binary)], cwd=binary.parent, capture_output=True, timeout=60)
        result.update(run_returncode=run.returncode, stdout=run.stdout.decode(errors="replace"),
                      stderr=run.stderr.decode(errors="replace"), passed=run.returncode == 0)
    return result


def verify(output: Path, cpu: int | None, selected: set[str] | None = None, resume: bool = False,
           pause: Path | None = None, *, compilers: tuple[str, ...] = ("GCC", "Clang"),
           syntax_only: bool = False, jobs: int = 1, gcc_install_dir: str | None = None,
           acl_include: Path | None = None, run_tests: bool = False,
           gcc: str = "g++", clang: str = "clang++") -> dict:
    if cpu is not None:
        os.sched_setaffinity(0, {cpu})

    def wait_if_paused() -> None:
        if pause is None or not pause.exists():
            return
        acknowledgment = pause.with_name(pause.name + "_ack")
        acknowledgment.write_text("paused\n", encoding="utf-8")
        while pause.exists():
            time.sleep(0.5)
        acknowledgment.unlink(missing_ok=True)

    manifest_path = ROOT / "Variants/manifest.json"
    manifest_data = manifest_path.read_bytes()
    manifest = json.loads(manifest_data)
    work_items = selected_variants(manifest["entries"], compilers, selected)
    if not work_items:
        raise ValueError("対象の配布版がありません")
    if run_tests:
        test_sources = set(RUNTIME_TESTS) if selected is None else RUNTIME_TESTS.keys() & selected
        expected = {(compiler, edition, fast, source) for compiler in compilers
                    for edition in ("Portable", compiler) for fast in (False, True) for source in test_sources}
        actual = {(compiler, entry["compiler"], entry["fast"], entry["source"])
                  for compiler, entry in work_items if entry["source"] in test_sources}
        if not expected or actual != expected:
            raise ValueError("動作テストに必要な配布版がそろっていません")
    result = {"status": "running", "cpu": cpu, "performance_measurement": False,
              "syntax_only": syntax_only, "run_tests": run_tests, "manifest_sha256": digest(manifest_data),
              "verifier_sha256": digest(Path(__file__).read_bytes()), "results": []}
    output.parent.mkdir(parents=True, exist_ok=True)

    def save() -> None:
        output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    with tempfile.TemporaryDirectory(prefix="sirkit_variants_") as temporary:
        work = Path(temporary)
        overlay = work / "include"
        make_acl_overlay(ROOT, overlay)
        if acl_include is not None:
            acl_include = acl_include.resolve(strict=True)
            if not (acl_include / "atcoder/all").is_file():
                raise ValueError("--acl-includeに atcoder/all を含むディレクトリを指定してください")
        include_dirs = [ROOT / "Verify/current/vendor"]
        if acl_include is not None:
            include_dirs.insert(0, acl_include)
        elif Path("/opt/atcoder/gcc/include/atcoder").is_dir():
            include_dirs.append(Path("/opt/atcoder/gcc/include"))
        result["acl_fingerprints"] = acl_fingerprints(include_dirs)
        result["gcc_install_dir"] = gcc_install_dir
        commands = {name: compiler_command(name, [*include_dirs[:1], overlay, *include_dirs[1:]], gcc_install_dir,
                                           gcc=gcc, clang=clang)
                    for name in compilers}
        result["compiler_versions"] = {name: compiler_version(command, name, work) for name, command in commands.items()}
        cached = {}
        if resume and output.is_file():
            previous = json.loads(output.read_bytes())
            if all(previous.get(key) == result[key] for key in (
                    "compiler_versions", "verifier_sha256", "syntax_only", "run_tests", "acl_fingerprints", "gcc_install_dir")):
                cached = {(item["path"], item["compiler"], item["setting"], item["source_sha256"], item["generated_sha256"]): item
                          for item in previous.get("results", []) if item.get("passed")}

        def check(item: tuple[int, tuple[str, dict]]) -> list[dict]:
            index, (compiler, entry) = item
            original = (ROOT / entry["origin"]).read_bytes()
            generated = (ROOT / entry["path"]).read_bytes()
            if digest(original) != entry["source_sha256"] or digest(generated) != entry["sha256"]:
                raise ValueError("検証前にソースが変わりました。再生成してください: " + entry["path"])
            active = work / f"source_{index}.cpp"
            runtime = None
            if run_tests and entry["source"] in RUNTIME_TESTS:
                test, macro = RUNTIME_TESTS[entry["source"]]
                if entry["fast"]:
                    test, macro = FAST_RUNTIME_TESTS.get(entry["source"], (test, macro))
                fixture = ROOT / "Verify/current/tests" / test
                fixture_bytes = fixture.read_bytes()
                library = work / f"library_{index}.hpp"
                library.write_bytes(generated)
                runtime = work / f"check_{index}.cpp"
                runtime.write_text(runtime_source(library, fixture, fixture_bytes.decode("utf-8"), macro, entry["fast"]),
                                   encoding="utf-8")
            rows = []
            for setting, extra in build_settings(compiler, entry["fast"], syntax_only):
                key = (entry["path"], compiler, setting, entry["source_sha256"], entry["sha256"])
                if runtime is None and key in cached:
                    rows.append(cached[key])
                    continue
                command = [*commands[compiler], *extra]
                hashes = []
                output_logs = []
                if compiler != "MSVC" and not syntax_only:
                    # コンパイラ自身に空白を正規化させ、リテラルを含むトークン列を比較する。
                    whitespace = ["-fminimize-whitespace"] if compiler == "Clang" else []
                    for text, defines in ((original, ["-DSKIP_RUNTIME_CHECKS="] if entry["fast"] else []),
                                          (generated, [])):
                        wait_if_paused()
                        active.write_bytes(text)
                        run = subprocess.run([*command, *defines, "-E", "-P", *whitespace, str(active)],
                                             capture_output=True, timeout=180)
                        output_logs.append(run.stderr.decode(errors="replace"))
                        if run.returncode:
                            break
                        hashes.append(digest(run.stdout))
                compared = compiler != "MSVC" and not syntax_only
                preprocessor_equal = len(hashes) == 2 and hashes[0] == hashes[1] if compared else None
                compile_passed = False
                if preprocessor_equal is not False:
                    wait_if_paused()
                    active.write_bytes(generated)
                    run = subprocess.run([*command, "/Zs" if compiler == "MSVC" else "-fsyntax-only", str(active)],
                                         capture_output=True, timeout=180)
                    output_logs.append((run.stdout + run.stderr).decode(errors="replace"))
                    compile_passed = run.returncode == 0
                row = {"path": entry["path"], "compiler": compiler, "source_sha256": entry["source_sha256"],
                       "generated_sha256": entry["sha256"], "setting": setting, "origin": entry["origin"],
                       "preprocessed_sha256": hashes, "preprocessor_equal": preprocessor_equal,
                       "syntax_checked": preprocessor_equal is not False, "syntax_passed": compile_passed,
                       "runtime_requested": runtime is not None, "diagnostics": output_logs,
                       "command_prefix": command, "passed": compile_passed}
                if runtime is not None and compile_passed:
                    wait_if_paused()
                    binary = work / f"check_{index}_{setting}.exe"
                    execution = run_runtime_test(compiler, command, runtime, binary)
                    row["runtime"] = {"fixture": fixture.relative_to(ROOT).as_posix(),
                                      "fixture_sha256": digest(fixture_bytes), **execution}
                    row["passed"] = execution["passed"]
                    if not execution["passed"]:
                        output_logs.extend(execution[key] for key in ("diagnostics", "stdout", "stderr") if key in execution)
                rows.append(row)
            return rows

        try:
            with ThreadPoolExecutor(max_workers=jobs) as pool:
                for index, rows in enumerate(pool.map(check, enumerate(work_items)), start=1):
                    result["results"].extend(rows)
                    for row in rows:
                        if not row["passed"]:
                            print(row["path"], row["compiler"], row["setting"], "FAIL",
                                  "\n".join(row["diagnostics"]), flush=True)
                    print(rows[0]["path"], rows[0]["compiler"],
                          "PASS" if all(row["passed"] for row in rows) else "FAIL", flush=True)
                    if jobs == 1 or index % 20 == 0:
                        save()
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            result.update(status="failed", error=str(error))
            save()
            raise
    result["status"] = "passed" if all(row["passed"] for row in result["results"]) else "failed"
    save()
    return result


def summary_markdown(report: dict) -> str:
    groups: dict[tuple[str, str], list[dict]] = {}
    for row in report["results"]:
        profile = "/".join(Path(row["path"]).parts[:2])
        groups.setdefault((row["compiler"], profile), []).append(row)
    lines = ["## 配布版の検査", "", "| 処理系 | 版 | 単独構文検査 | 動作テスト |",
             "| --- | --- | ---: | ---: |"]
    for (compiler, profile), rows in sorted(groups.items()):
        syntax = sum(row["syntax_passed"] for row in rows)
        requested = [row for row in rows if row["runtime_requested"]]
        runtime = sum(bool(row.get("runtime", {}).get("passed")) for row in requested)
        lines.append(f"| {compiler} | `{profile}` | {syntax}/{len(rows)} | {runtime}/{len(requested)} |")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--summary", type=Path, help="配布版別の集計をMarkdownで追記")
    parser.add_argument("--cpu", type=int, help="Linuxで使用するCPU番号")
    parser.add_argument("--source", action="append")
    parser.add_argument("--compiler", choices=COMPILERS, action="append")
    parser.add_argument("--syntax-only", action="store_true", help="既定と検査省略の単独構文検査")
    parser.add_argument("--run-tests", action="store_true", help="各配布版で登録済みの動作テストを実行")
    parser.add_argument("--gcc", default="g++", help="GCCの実行ファイル")
    parser.add_argument("--clang", default="clang++", help="Clangの実行ファイル")
    parser.add_argument("--gcc-install-dir", help="Clangが使うGCCのライブラリディレクトリ")
    parser.add_argument("--acl-include", type=Path, help="atcoder/allを含むACLディレクトリ")
    parser.add_argument("--jobs", type=int, default=1)
    parser.add_argument("--resume", action="store_true", help="同じソース・コンパイラの成功済み照合を再利用")
    parser.add_argument("--pause-file", type=Path, help="このファイルがある間は新しいコンパイルを停止")
    options = parser.parse_args()
    if options.jobs < 1 or (options.pause_file and options.jobs != 1):
        parser.error("--jobsは正の整数を指定し、--pause-file使用時は1を指定してください")
    if options.cpu is not None and not hasattr(os, "sched_setaffinity"):
        parser.error("--cpuはLinuxでのみ使用できます")
    compilers = tuple(dict.fromkeys(options.compiler or (["MSVC"] if os.name == "nt" else ["GCC", "Clang"])))
    try:
        report = verify(options.output, options.cpu, set(options.source) if options.source else None, options.resume,
                        options.pause_file, compilers=compilers, syntax_only=options.syntax_only,
                        jobs=options.jobs, gcc_install_dir=options.gcc_install_dir, acl_include=options.acl_include,
                        run_tests=options.run_tests, gcc=options.gcc, clang=options.clang)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        parser.exit(1, str(error) + "\n")
    if options.summary:
        with options.summary.open("a", encoding="utf-8") as summary:
            summary.write(summary_markdown(report))
    counts = sum(row["passed"] for row in report["results"])
    passed = report["status"] == "passed"
    status = "PASS" if passed else "FAIL"
    print(f"{counts}/{len(report['results'])} configurations: {status}")
    return int(not passed)


if __name__ == "__main__":
    raise SystemExit(main())
