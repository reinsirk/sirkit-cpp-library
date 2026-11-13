#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""複数ライブラリの貼付順を逆転し、共有ガードと公開名の衝突を調べる。"""
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


def verify(output: Path, cpu: int, pause_file: Path | None = None) -> dict:
    os.sched_setaffinity(0, {cpu})
    entries = json.loads((ROOT / "Variants/manifest.json").read_text())["entries"]
    profiles = {"normal": sorted({item["source"] for item in entries if "/" in item["source"]})}
    for prefix in ("Variants/GCC", "Variants/Clang", "Fast/GCC", "Fast/Clang"):
        profiles[prefix] = sorted(item["path"] for item in entries
                                  if item["path"].startswith(prefix + "/") and "/" in item["source"])
    snapshots = {name: [(path, (ROOT / path).read_bytes()) for path in paths]
                 for name, paths in profiles.items()}
    result = {"status": "running", "cpu": cpu, "performance_measurement": False,
              "scope": "全本体の宣言を順方向・逆方向・二重貼付で構文検査し、二つの翻訳単位をリンクする。テンプレートの全組合せ実体化やAPI実行試験ではない。",
              "started_at_unix": time.time(), "sources": {}, "results": []}
    for name, sources in snapshots.items():
        result["sources"][name] = {path: hashlib.sha256(data).hexdigest() for path, data in sources}
    commands = {"GCC": ["/usr/bin/g++"],
                "Clang": ["/usr/bin/clang++", "--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15"]}
    result["compiler_versions"] = {name: subprocess.check_output([cmd[0], "--version"], text=True)
                                   for name, cmd in commands.items()}
    standard = ["-DSIRKIT_DISABLE_SIMD", "-DSIRKIT_FORCE_STANDARD_ARITHMETIC",
                "-DSIRKIT_FORCE_STANDARD_MUL_OVERFLOW", "-DSIRKIT_FRACTION_DISABLE_INT128", "-DSIRKIT_CONVOLUTION_DISABLE_INT128", "-DGEO_DISABLE_INT128"]
    output.parent.mkdir(parents=True, exist_ok=True)
    def pause() -> None:
        while pause_file is not None and pause_file.exists():
            time.sleep(0.25)
    with tempfile.TemporaryDirectory(prefix="sirkit_combined_") as directory:
        source = Path(directory) / "combined.cpp"
        for profile, files in snapshots.items():
            compilers = list(commands) if profile == "normal" else [profile.split("/")[-1]]
            settings = [("default", []), ("native", ["-march=native"]), ("standard", standard)]
            if not profile.startswith("Fast/"):
                settings.append(("skip", ["-DSKIP_RUNTIME_CHECKS"]))
            for order in ("forward", "reverse", "repeated"):
                selected = files if order == "forward" else list(reversed(files)) if order == "reverse" else files + files
                data = b"\n".join(content for _, content in selected) + b"\nint main() {}\n"
                source.write_bytes(data)
                for compiler in compilers:
                    for setting, extra in settings:
                        pause()
                        command = [*commands[compiler], "-std=c++23", "-O2", "-fsyntax-only", *extra, str(source)]
                        run = subprocess.run(command, capture_output=True, text=True, timeout=240)
                        record = {"profile": profile, "compiler": compiler, "setting": setting,
                                  "order": order, "combined_sha256": hashlib.sha256(data).hexdigest(),
                                  "command": command, "exit_code": run.returncode,
                                  "stdout": run.stdout, "stderr": run.stderr}
                        result["results"].append(record)
                        output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
                        print(profile, compiler, order, setting, "PASS" if run.returncode == 0 else "FAIL", flush=True)
                        if run.returncode:
                            raise RuntimeError("ライブラリ併用時の構文エラー: " + profile + "\n" + run.stderr)
            # 同一の公開関数や共有定数を二つの翻訳単位で定義してもリンクできることを確認する。
            header = Path(directory) / "libraries.hpp"
            header.write_bytes(b"\n".join(content for _, content in files) + b"\n")
            first, second = Path(directory) / "first.cpp", Path(directory) / "second.cpp"
            first.write_text('#include "libraries.hpp"\nint sirkit_combination_probe();\nint main(){return sirkit_combination_probe();}\n')
            second.write_text('#include "libraries.hpp"\nint sirkit_combination_probe(){return 0;}\n')
            for compiler in compilers:
                pause()
                command = [*commands[compiler], "-std=c++23", "-O2", str(first), str(second), "-o", str(Path(directory) / "combined")]
                run = subprocess.run(command, capture_output=True, text=True, timeout=240)
                result["results"].append({"profile": profile, "compiler": compiler, "setting": "default", "order": "two_translation_units",
                                          "combined_sha256": hashlib.sha256(header.read_bytes()).hexdigest(), "command": command,
                                          "exit_code": run.returncode, "stdout": run.stdout, "stderr": run.stderr})
                output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
                print(profile, compiler, "two_translation_units", "PASS" if run.returncode == 0 else "FAIL", flush=True)
                if run.returncode:
                    raise RuntimeError("ライブラリ併用時のリンクエラー: " + profile + "\n" + run.stderr)
    result["status"] = "passed"
    result["completed_at_unix"] = time.time()
    output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cpu", type=int, default=2)
    parser.add_argument("--pause-file", type=Path)
    args = parser.parse_args()
    verify(args.output, args.cpu, args.pause_file)
