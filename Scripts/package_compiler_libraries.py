#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""明示的に必要な場合だけ、Releases添付用のGCC・Clang別ZIPを作る。"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from deterministic_zip import archive_versions, write_archive

import package_revised_libraries
from flattened_markdown_repair import repair_flattened_markdown, verify_index

ROOT = Path(__file__).resolve().parents[1]


def package(directory: Path, check_generators: bool = True) -> list[dict]:
    if check_generators:
        package_revised_libraries.check_generated(ROOT)
    libraries = json.loads((ROOT / "Variants/manifest.json").read_text())["entries"]
    verifies = json.loads((ROOT / "Variants/verify_manifest.json").read_text())["entries"]
    directory.mkdir(parents=True, exist_ok=True)
    reports = []
    for compiler in ("GCC", "Clang"):
        for fast in (False, True):
            prefix = ("Fast/" if fast else "Variants/") + compiler
            contents: dict[str, bytes] = {}
            for entry in libraries:
                if entry["compiler"] == compiler and entry["fast"] == fast:
                    contents[entry["source"]] = (ROOT / entry["path"]).read_bytes()
            known_sources = set(contents)
            examples = package_revised_libraries.LIBRARIES
            for example, source in examples.items():
                text = (ROOT / "Examples" / example).read_text()
                if fast and source == "DataStructure/Node.cpp":
                    text = text.replace("entry.get()", "entry")
                    text = text.replace("所有は残る", "参照は残る").replace("共有所有の循環を切ってから解放する", "循環を切る。メモリは解放しない")
                contents["Examples/" + example] = text.encode()
                include = '#include "../' + source + '"'
                if text.count(include) != 1:
                    raise ValueError("使用例のincludeが一意でありません: " + example)
                contents["StandaloneExamples/" + example] = package_revised_libraries.expand_acl(package_revised_libraries.expand_example_dependencies(
                    text, "Examples/" + example, known_sources,
                    lambda dependency: contents[dependency].decode())).encode()
            selected = []
            for entry in verifies:
                if entry["compiler"] == compiler and entry["fast"] == fast:
                    target = "Verify/current/submissions/" + entry["id"] + ".cpp"
                    contents[target] = (ROOT / entry["path"]).read_bytes()
                    record = dict(entry)
                    record["path"] = target
                    record["upstream_source_paths"] = dict(record["source_paths"])
                    record["source_paths"] = {origin: origin for origin in record["source_paths"]}
                    selected.append(record)
            contents["Verify/current/manifest.json"] = (json.dumps({"entries": selected}, ensure_ascii=False, indent=2) + "\n").encode()
            for folder in ("Document", "LICENSES"):
                for path in (ROOT / folder).rglob("*"):
                    if path.is_file() and path.suffix in (".md", ".txt"):
                        contents[path.relative_to(ROOT).as_posix()] = path.read_bytes()
            for folder in ("Algorithm", "DataStructure", "Graph", "Math", "String", "Geometry", "Internal"):
                for path in (ROOT / folder).rglob("README.md"):
                    contents[path.relative_to(ROOT).as_posix()] = path.read_bytes()
            for name in ("LICENSE", "Verify/current/vendor/ACL_LICENSE"):
                if (ROOT / name).is_file(): contents[name] = (ROOT / name).read_bytes()
            explanation = ("入力検査を除き、専用の高速実装は領域/格納値の破棄・解放も省きます。各ファイル冒頭の前提を確認してください。"
                           if fast else "このコンパイラで不要な分岐を除いた通常版です。CPU判定と設定マクロは保持します。")
            contents["Readme.md"] = ("# sirkit-cpp-library / " + compiler + (" / Fast" if fast else "") + "\n\n" + explanation +
                "\n\n[使い方と通常版との違い](Document/fast_variants.md)・[資料一覧](Document/README.md)・[使用例](Examples)を参照。\n" +
                "\n提出用コードはVerify/current/submissionsにあります。この配布版へ通常版のオンラインACは転記していません。\n" +
                "\n開発支援: 実装・最適化・検証・ドキュメントの整備に、Codex（OpenAI）とClaude（Anthropic）を利用しています。\n").encode()
            contents["Verify/current/README.md"] = verify_index(selected, compiler, fast)
            repair_flattened_markdown(contents, ROOT, fast=fast)
            slug = ("fast_" if fast else "") + compiler.lower()
            target = directory / ("sirkit-cpp-library_" + slug + ".zip")
            write_archive(target, contents)
            reports.append({"compiler": compiler, "fast": fast, "path": str(target.resolve()), "files": len(contents),
                            "bytes": target.stat().st_size, "sha256": hashlib.sha256(target.read_bytes()).hexdigest(), "crc_checked": True, "zip_environment": archive_versions()})
    return reports


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output_directory", type=Path)
    options = parser.parse_args()
    print(json.dumps(package(options.output_directory), ensure_ascii=False, indent=2))
