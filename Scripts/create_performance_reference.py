#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""手元のCPUでVerify全体を測り、ケース別の初期性能を保存する。"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

try:
    from .ci_compile_profiles import COMPILE_PROFILES
except ImportError:
    from ci_compile_profiles import COMPILE_PROFILES


ROOT = Path(__file__).resolve().parents[1]


def git(*args: str) -> str:
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="g++", help="C++コンパイラ (既定: g++)")
    parser.add_argument("--compile-profile", choices=COMPILE_PROFILES, default="standard")
    parser.add_argument("--cpu", type=int, help="測定先の論理CPU番号。Linuxで指定可能")
    parser.add_argument("--library", action="append", help="ライブラリの相対パス。複数指定可、既定は全件")
    parser.add_argument("--repetitions", type=int, default=3)
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--quick", action="store_true", help="小規模ケースで動作確認する")
    parser.add_argument("--cache", type=Path, help="使い回す場合のリポジトリ外の作業領域")
    args = parser.parse_args()

    if args.cpu is not None:
        if not hasattr(os, "sched_getaffinity") or args.cpu not in os.sched_getaffinity(0):
            parser.error("指定した論理CPUに固定できません")
        os.sched_setaffinity(0, {args.cpu})

    revision = git("rev-parse", "HEAD")
    status_args = ("status", "--porcelain", "--untracked-files=no")
    dirty = bool(git(*status_args))
    started = datetime.now(timezone.utc)
    with tempfile.TemporaryDirectory(prefix="sirkit-performance-reference-") as work:
        workspace = Path(work)
        cache = args.cache.expanduser().resolve() if args.cache is not None else workspace / "cache"
        report = workspace / "report"
        runner = ROOT / "Scripts/ci_performance_all.py"
        settings = ["--compiler", args.compiler, "--compile-profile", args.compile_profile,
                    "--repetitions", str(args.repetitions), "--warmups", str(args.warmups)]
        for library in args.library or []:
            settings.extend(["--library", library])
        if args.quick:
            settings.append("--quick")
        command = [sys.executable, str(runner), *settings, "--cache", str(cache), "--output", str(report)]
        if (code := subprocess.run(command, cwd=ROOT).returncode) != 0:
            if (report / "summary.md").is_file():
                print((report / "summary.md").read_text(), file=sys.stderr)
            return code
        if git("rev-parse", "HEAD") != revision:
            parser.error("測定中にHEADが変わりました")

        stamp = started.strftime("%Y%m%dT%H%M%S%fZ")
        store = ROOT / ".sirkit-local/performance"
        store.mkdir(parents=True, exist_ok=True)
        saved = store / f"{stamp}-{revision[:12]}"
        metadata = {"format": 1, "commit": revision, "started_at_utc": started.isoformat(),
                    "tracked_changes_present": dirty or bool(git(*status_args)), "runner_args": settings}
        with tempfile.TemporaryDirectory(prefix=".reference-", dir=store) as staging:
            pending = Path(staging)
            for name in ("result.json", "summary.md"):
                shutil.copyfile(report / name, pending / name)
            (pending / "reference.json").write_text(json.dumps(metadata, ensure_ascii=False, indent=2) + "\n")
            pending.rename(saved)
    print(f"性能リファレンスを保存しました: {saved}")
    if metadata["tracked_changes_present"]:
        print("追跡ファイルに未コミットの変更があります。実測したソースのSHAはresult.jsonを参照してください。",
              file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
