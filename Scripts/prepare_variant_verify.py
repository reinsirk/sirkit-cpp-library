#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""同じdriver・共通I/Oへコンパイラ別/速度優先ライブラリを埋め込む。"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re

import prepare_verify

ROOT = Path(__file__).resolve().parents[1]


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def prepare(check: bool = False) -> dict:
    entries = prepare_verify.collect()
    overrides_path = ROOT / "Scripts/fast_driver_overrides.json"
    overrides = json.loads(overrides_path.read_text()) if overrides_path.exists() else {}
    annotation_path = ROOT / "Variants/submission_annotations.json"
    annotations = json.loads(annotation_path.read_text()) if annotation_path.exists() else {}
    manifest = {"format": 1, "entries": [], "omitted_local": []}
    for compiler in ("GCC", "Clang"):
        for fast in (False, True):
            prefix = ("Fast/" if fast else "Variants/") + compiler
            for entry in entries:
                # unchecked版に、例外などの無効入力検査を含む通常版のunit mainを流用しない。
                if fast and not entry.get("problem_url") and entry["id"] not in overrides:
                    manifest["omitted_local"].append({"profile": prefix, "id": entry["id"],
                                                      "reason": "速度優先版は有効入力の専用fixtureで補足する"})
                    continue
                source_paths = {name: prefix + "/" + name for name in entry["libraries"]}
                libraries = {name: (ROOT / path).read_bytes() for name, path in source_paths.items()}
                driver_path = overrides.get(entry["id"], entry["driver"]) if fast else entry["driver"]
                driver = (ROOT / driver_path).read_text()
                target = prefix + "/Verify/current/submissions/" + entry["id"] + ".cpp"
                output = ROOT / target
                annotation_key = prefix + ":" + entry["id"]
                profile_label = "// Profile: " + prefix + "; online judgement is recorded separately."
                urls = annotations.get(annotation_key, [])
                # 旧生成形式の通常版URLを、その配布版自身の提出記録として取り込まない。
                if not check and output.is_file() and profile_label in output.read_text().splitlines():
                    urls = list(dict.fromkeys(urls + re.findall(r"^// Submission: (https?://\S+)\s*$", output.read_text(), re.M)))
                    if urls:
                        annotations[annotation_key] = urls
                profile_entry = {**entry, "submission_urls": urls}
                data, hashes = prepare_verify.render(profile_entry, source_overrides=libraries,
                                                      driver_override=driver,
                                                      skip_runtime_checks=True if fast else None)
                lines = data.decode().splitlines(keepends=True)
                lines.insert(1 if lines[0].startswith("#define SKIP_RUNTIME_CHECKS") else 0, profile_label + "\n")
                data = "".join(lines).encode()
                if len(data) > 512 * 1024:
                    raise ValueError("提出コードが512 KiBを超えています: " + target)
                if check:
                    if not output.is_file() or output.read_bytes() != data:
                        raise ValueError("提出用の生成版が同期していません: " + target)
                else:
                    output.parent.mkdir(parents=True, exist_ok=True)
                    output.write_bytes(data)
                manifest["entries"].append({"id": entry["id"], "compiler": compiler, "fast": fast,
                                            "path": target, "bytes": len(data), "sha256": digest(data),
                                            "source_paths": source_paths, "source_hashes": hashes,
                                            "driver": driver_path, "driver_sha256": digest(driver.encode()),
                                            "normal_driver": entry["driver"],
                                            "driver_adapted": driver_path != entry["driver"],
                                            "support_source_hashes": prepare_verify.common_source_hashes(driver),
                                            "io_source_hashes": {prepare_verify.FAST_IO: digest((ROOT / prepare_verify.FAST_IO).read_bytes())}
                                            if prepare_verify.FAST_IO_INCLUDE in driver.splitlines() else {},
                                            "problem_url": entry.get("problem_url"),
                                            "submission_urls": urls[-1:],
                                            "online_status": "提出URLあり・判定未照合" if urls else "未提出（通常版のACを転記しない）"})
    if not check and annotations:
        annotation_path.write_text(json.dumps(annotations, ensure_ascii=False, indent=2) + "\n")
    path = ROOT / "Variants/verify_manifest.json"
    data = (json.dumps(manifest, ensure_ascii=False, indent=2) + "\n").encode()
    if check:
        if path.read_bytes() != data:
            raise ValueError("生成版のVerify manifestが同期していません")
    else:
        path.write_bytes(data)
    return manifest


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    result = prepare(parser.parse_args().check)
    print(str(len(result["entries"])) + "件のコンパイラ別・速度優先Verify: 同期" )
