#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""Moの提出コードを基準commitと現在の作業ツリーで比べる。公式全ケースで交互に実行し、公式チェッカーで照合する。

例: taskset -c 12 python3 Audit/Benchmarks/mo_submissions.py --official 固定revisionのcheckout --output result.json
checkoutは Verify/ci/library_checker.json のrevisionで、対象問題の generate.py を実行済みのものを使う。
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import platform
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
PROBLEMS = {"mo": "static_range_count_distinct", "mo_updates": "point_set_range_frequency",
            "coverage_mo_inversions": "static_range_inversions_query", "mo_rollback_mode": "static_range_mode_query"}
FLAGS = ["-std=c++23", "-O2", "-DNDEBUG"]


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def git(*arguments: str) -> subprocess.CompletedProcess:
    return subprocess.run(["git", "-C", str(ROOT), *arguments], capture_output=True)


def execute(binary: Path, case: Path, output: Path) -> dict:
    with case.open("rb") as stdin, output.open("wb") as stdout:
        start = time.perf_counter()
        process = subprocess.Popen([str(binary)], stdin=stdin, stdout=stdout, stderr=subprocess.DEVNULL)
        _, status, usage = os.wait4(process.pid, 0)
        wall = time.perf_counter() - start
    return {"returncode": os.waitstatus_to_exitcode(status), "cpu_ms": round((usage.ru_utime + usage.ru_stime) * 1e3, 3),
            "wall_ms": round(wall * 1e3, 3)}


def peak_memory(binary: Path, case: Path, work: Path) -> int:
    # Pythonから直接起動するとexec前の親の常駐量も数えるため、小さいGNU timeを介して最大常駐量を測る。
    record = work / "maxrss.txt"
    with case.open("rb") as stdin:
        subprocess.run(["/usr/bin/time", "-f", "%M", "-o", str(record), str(binary)], stdin=stdin,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    return int(record.read_text().split()[-1])


def cpu_model() -> str:
    for line in Path("/proc/cpuinfo").read_text().splitlines():
        if line.startswith("model name"):
            return line.split(":", 1)[1].strip()
    return platform.processor()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--official", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--base", default="HEAD")
    parser.add_argument("--repetitions", type=int, default=5)
    parser.add_argument("--id", action="append", choices=sorted(PROBLEMS))
    args = parser.parse_args()
    pin = json.loads((ROOT / "Verify/ci/library_checker.json").read_text())["revision"]
    official_revision = subprocess.run(["git", "-C", str(args.official), "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
    if official_revision != pin:
        parser.error("公式checkoutのrevisionが固定版と一致しません")
    base = git("rev-parse", args.base).stdout.decode().strip()
    compiler = subprocess.run(["g++", "--version"], capture_output=True, text=True).stdout.splitlines()[0]
    report = {"format": 1, "script": "Audit/Benchmarks/mo_submissions.py", "script_sha256": sha(Path(__file__).read_bytes()),
              "base_revision": base, "head_revision": git("rev-parse", "HEAD").stdout.decode().strip(),
              "official_revision": official_revision, "cpu": cpu_model(), "cpu_affinity": sorted(os.sched_getaffinity(0)),
              "compiler": compiler, "flags": FLAGS, "repetitions": args.repetitions, "warmups": 1,
              "order": "各反復で基準版と現在版を交互に実行。最初の1回は照合用で計測値に含めない。",
              "metric": "子プロセスのuser+sys CPU時間と経過時間は反復の中央値。最大常駐メモリは照合用の1回をGNU time経由で測る。",
              "targets": []}
    with tempfile.TemporaryDirectory(prefix="sirkit-mo-submissions-") as name:
        work = Path(name)
        for identifier in args.id or list(PROBLEMS):
            directory = args.official / "data_structure" / PROBLEMS[identifier]
            path = "Verify/current/submissions/" + identifier + ".cpp"
            sources = {}
            previous = git("show", base + ":" + path)
            if previous.returncode == 0:
                sources["base"] = previous.stdout
            sources["current"] = (ROOT / path).read_bytes()
            binaries = {}
            for label, data in sources.items():
                source = work / (identifier + "_" + label + ".cpp")
                source.write_bytes(data)
                binaries[label] = work / (identifier + "_" + label)
                subprocess.run(["g++", *FLAGS, str(source), "-o", str(binaries[label])], check=True)
            target = {"id": identifier, "problem": PROBLEMS[identifier], "submission": path,
                      "source_sha256": {label: sha(data) for label, data in sources.items()}, "cases": []}
            for case in sorted((directory / "in").glob("*.in")):
                expected = directory / "out" / (case.stem + ".out")
                row = {"case": case.stem, "input_bytes": case.stat().st_size, "input_sha256": sha(case.read_bytes()),
                       "first_line": case.read_text().split("\n", 1)[0][:60], "versions": {}}
                for repetition in range(args.repetitions + 1):
                    for label, binary in binaries.items():
                        actual = work / (identifier + "_" + label + ".out")
                        result = execute(binary, case, actual)
                        version = row["versions"].setdefault(label, {"cpu_ms": [], "wall_ms": []})
                        if repetition == 0:
                            checked = subprocess.run([str(directory / "checker"), str(case), str(actual), str(expected)], capture_output=True)
                            version.update(returncode=result["returncode"], checker_returncode=checked.returncode,
                                           output_sha256=sha(actual.read_bytes()), maxrss_kib=peak_memory(binary, case, work))
                            continue
                        for key in ("cpu_ms", "wall_ms"):
                            version[key].append(result[key])
                for version in row["versions"].values():
                    for key in ("cpu_ms", "wall_ms"):
                        version[key + "_median"] = statistics.median(version[key])
                if "base" in row["versions"]:
                    row["cpu_ratio"] = round(row["versions"]["current"]["cpu_ms_median"] / max(row["versions"]["base"]["cpu_ms_median"], 1e-9), 4)
                target["cases"].append(row)
                print(identifier, case.stem, {label: (v["cpu_ms_median"], v["checker_returncode"]) for label, v in row["versions"].items()}, flush=True)
            report["targets"].append(target)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=1) + "\n")


if __name__ == "__main__":
    main()
