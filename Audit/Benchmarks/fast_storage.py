#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""同一入力・API列で通常版と解放を省く版の費用を比較する。"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import random
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
KINDS = {
    0: "DataStructure/Fenwick_Tree.cpp",
    1: "DataStructure/Segment_Tree.cpp",
    2: "DataStructure/Lazy_Segment_Tree.cpp",
    3: "DataStructure/Persistent_Segment_Tree.cpp",
    5: "String/Aho_Corasick.cpp",
    6: "DataStructure/implicit_treap.cpp",
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def build(directory):
    directory.mkdir(parents=True, exist_ok=True)
    fixture = ROOT / "Audit/Benchmarks/fast_storage.cpp"
    records = []
    for compiler in ("gcc", "clang"):
        for native in (False, True):
            for kind, source in KINDS.items():
                for fast in (False, True):
                    origin = ROOT / ("Fast/Source/" + source if fast else source)
                    original = origin.read_bytes()
                    text = original.decode()
                    if kind == 6:
                        # 木の形だけが測定ラベル間で変わることを避ける。
                        # 本体には反映せず、両測定用スナップショットだけ同じ実行時seedにする。
                        old = "return static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());"
                        if text.count(old) != 1:
                            raise ValueError("Treapのseed設定を再確認してください")
                        text = text.replace(old, "return benchmark_treap_seed;")
                    slug = f"{compiler}_{int(native)}_{kind}_{int(fast)}"
                    header = directory / (slug + ".hpp")
                    header.write_text(text)
                    executable = directory / slug
                    command = (["/usr/bin/g++"] if compiler == "gcc" else
                               ["/usr/bin/clang++", "--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15"])
                    command += ["-std=c++23", "-O2", "-DSKIP_RUNTIME_CHECKS",
                                "-DSTORAGE_KIND=" + str(kind), '-DSTORAGE_LIBRARY="' + str(header) + '"']
                    if native:
                        command += ["-march=native"]
                    command += [str(fixture), "-o", str(executable)]
                    run = subprocess.run(["taskset", "-c", "2", *command], capture_output=True, text=True, timeout=240)
                    record = {"compiler": compiler, "native": native, "kind": kind, "fast": fast,
                              "source": str(origin.relative_to(ROOT)), "source_sha256": sha(original),
                              "snapshot": str(header), "snapshot_sha256": sha(text.encode()),
                              "fixture_sha256": sha(fixture.read_bytes()), "command": command,
                              "build_exit": run.returncode, "diagnostics": run.stdout + run.stderr,
                              "executable": str(executable)}
                    records.append(record)
                    (directory / "build.json").write_text(json.dumps(records, indent=2))
                    print(slug, run.returncode, flush=True)
                    if run.returncode:
                        raise SystemExit(1)
                    record["executable_sha256"] = sha(executable.read_bytes())
    (directory / "build.json").write_text(json.dumps(records, indent=2))


def run(directory, repetitions):
    records = json.loads((directory / "build.json").read_text())
    for entry in records:
        if sha(Path(entry["snapshot"]).read_bytes()) != entry["snapshot_sha256"]:
            raise ValueError("ヘッダーがビルド後に変わりました")
        if sha(Path(entry["executable"]).read_bytes()) != entry["executable_sha256"]:
            raise ValueError("実行ファイルがビルド後に変わりました")
    lookup = {(r["compiler"], r["native"], r["kind"], r["fast"]): r for r in records}
    active = directory / "active"
    order = random.Random(0x20261031)
    results = []
    for compiler in ("gcc", "clang"):
        for native in (False, True):
            for kind in KINDS:
                sizes = [(32, 200000), (4096, 200000), (262144, 300000)]
                if kind == 3:
                    sizes[-1] = (65536, 120000)
                if kind == 5:
                    sizes = [(32, 4096), (1024, 20000), (30000, 500000)]
                for n, q in sizes:
                    for operation in (0, 1):
                        for repeat in range(repetitions):
                            labels = ["normal", "control", "fast"]
                            order.shuffle(labels)
                            checksums = []
                            seed = 734021 + repeat % 3
                            for label in labels:
                                entry = lookup[compiler, native, kind, label == "fast"]
                                shutil.copyfile(entry["executable"], active)
                                active.chmod(0o700)
                                process = subprocess.run(["taskset", "-c", "12", str(active), str(n), str(q),
                                                          str(seed), str(operation)], capture_output=True,
                                                         text=True, timeout=120)
                                if process.returncode:
                                    raise RuntimeError(process.stderr)
                                row = json.loads(process.stdout)
                                checksums.append(row["checksum"])
                                row.update(compiler=compiler, native=native, kind=kind, n=n, q=q,
                                           operation=operation, repeat=repeat, label=label, seed=seed,
                                           executable_sha256=entry["executable_sha256"])
                                results.append(row)
                            if len(set(checksums)) != 1:
                                raise ValueError("出力が一致しません")
                        (directory / "raw.json").write_text(json.dumps(results, indent=2))
                        print(compiler, native, kind, n, operation, "PASS", flush=True)
    return results


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("build", "run"))
    parser.add_argument("directory", type=Path)
    parser.add_argument("--repetitions", type=int, default=7)
    options = parser.parse_args()
    if options.action == "build":
        build(options.directory.resolve())
    else:
        run(options.directory.resolve(), options.repetitions)
