#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""生成版と元ソースのプリプロセス結果・単独ビルドを照合する。"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def verify(output: Path, cpu: int, selected: set[str] | None = None, resume: bool = False,
           pause: Path | None = None) -> dict:
    def wait_if_paused() -> None:
        if pause is None or not pause.exists():
            return
        acknowledgment = pause.with_name(pause.name + "_ack")
        acknowledgment.write_text("paused\n")
        while pause.exists():
            time.sleep(0.5)
        acknowledgment.unlink(missing_ok=True)

    os.sched_setaffinity(0, {cpu})
    manifest = json.loads((ROOT / "Variants/manifest.json").read_text())
    result = {"status": "running", "cpu": cpu, "performance_measurement": False,
              "manifest_sha256": digest((ROOT / "Variants/manifest.json").read_bytes()),
              "verifier_sha256": digest(Path(__file__).read_bytes()), "results": []}
    compilers = {"GCC": ["/usr/bin/g++", "-std=c++23", "-O2"],
                 "Clang": ["/usr/bin/clang++", "--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15", "-std=c++23", "-O2"]}
    result["compiler_versions"] = {name: subprocess.check_output([command[0], "--version"], text=True)
                                   for name, command in compilers.items()}
    cached = {}
    if resume and output.is_file():
        previous = json.loads(output.read_text())
        if (previous.get("compiler_versions") == result["compiler_versions"] and
                previous.get("verifier_sha256") == result["verifier_sha256"]):
            cached = {(item["path"], item["setting"], item["source_sha256"], item["generated_sha256"]): item
                      for item in previous.get("results", []) if item.get("passed")}
    force = ["-DSIRKIT_DISABLE_SIMD", "-DSIRKIT_FORCE_STANDARD_ARITHMETIC",
             "-DSIRKIT_FORCE_STANDARD_MUL_OVERFLOW", "-DSIRKIT_FRACTION_DISABLE_INT128", "-DGEO_DISABLE_INT128", "-DSIRKIT_CONVOLUTION_DISABLE_INT128"]
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="sirkit_variants_") as temporary:
        active = Path(temporary) / "active.cpp"
        for entry in manifest["entries"]:
            if selected is not None and entry["source"] not in selected:
                continue
            original = (ROOT / entry["origin"]).read_bytes()
            generated = (ROOT / entry["path"]).read_bytes()
            if digest(original) != entry["source_sha256"] or digest(generated) != entry["sha256"]:
                raise ValueError("検証前にソースが変わりました。再生成してください: " + entry["path"])
            settings = [("default", []), ("native", ["-march=native"]), ("standard", force)]
            if not entry["fast"]:
                settings.append(("skip", ["-DSKIP_RUNTIME_CHECKS"]))
            for setting, extra in settings:
                key = (entry["path"], setting, entry["source_sha256"], entry["sha256"])
                if key in cached:
                    result["results"].append(cached[key])
                    continue
                command = [*compilers[entry["compiler"]], "-I/opt/atcoder/gcc/include", *extra]
                hashes = []
                errors = []
                # 専用高速実装も、その生成元とのコンパイラ分岐の同一性を照合する。
                # 通常版との意味上の差は別の独立参照テストで検証する。
                variants = [(original, ["-DSKIP_RUNTIME_CHECKS="] if entry["fast"] else []), (generated, [])]
                for text, defines in variants:
                    wait_if_paused()
                    active.write_bytes(text)
                    # Clangの空行出力は除いたディレクティブの配置に影響される。
                    # リテラルを壊す独自の空白除去ではなく、コンパイラ自身に正規化させる。
                    whitespace = ["-fminimize-whitespace"] if entry["compiler"] == "Clang" else []
                    run = subprocess.run([*command, *defines, "-E", "-P", *whitespace, str(active)], capture_output=True, timeout=180)
                    if run.returncode:
                        raise ValueError(run.stderr.decode(errors="replace"))
                    hashes.append(digest(run.stdout))
                    errors.append(run.stderr.decode(errors="replace"))
                if hashes[0] != hashes[1]:
                    raise ValueError("プリプロセス結果が異なります: " + entry["path"] + "/" + setting)
                if setting == "default":
                    wait_if_paused()
                    active.write_bytes(generated)
                    run = subprocess.run([*command, "-fsyntax-only", str(active)], capture_output=True, timeout=180)
                    if run.returncode:
                        raise ValueError(entry["path"] + ": " + run.stderr.decode(errors="replace"))
                    errors.append(run.stderr.decode(errors="replace"))
                result["results"].append({"path": entry["path"], "source_sha256": entry["source_sha256"],
                                          "generated_sha256": entry["sha256"], "setting": setting,
                                          "origin": entry["origin"], "preprocessed_sha256": hashes, "preprocessor_equal": True,
                                          "syntax_checked": setting == "default", "diagnostics": errors,
                                          "command_prefix": command, "passed": True})
            output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
            print(entry["path"], "PASS", flush=True)
    result["status"] = "passed"
    output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cpu", type=int, default=2)
    parser.add_argument("--source", action="append")
    parser.add_argument("--resume", action="store_true", help="同じソース・コンパイラの成功済み照合を再利用")
    parser.add_argument("--pause-file", type=Path, help="このファイルがある間は新しいコンパイルを停止")
    options = parser.parse_args()
    report = verify(options.output, options.cpu, set(options.source) if options.source else None, options.resume,
                    options.pause_file)
    print(str(len(report["results"])) + "構成: PASS")


if __name__ == "__main__":
    main()
