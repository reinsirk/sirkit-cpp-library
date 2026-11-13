#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""新規ライブラリの検証メタデータを共有一覧へ取り込む。"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def normalize_examples(mapping: dict[str, str]) -> dict[str, str]:
    result = {}
    for first, second in mapping.items():
        first_example = first.removeprefix("Examples/")
        second_example = second.removeprefix("Examples/")
        if (ROOT / "Examples" / first_example).is_file() and (ROOT / second).is_file():
            example, library = first_example, second
        elif (ROOT / "Examples" / second_example).is_file() and (ROOT / first).is_file():
            example, library = second_example, first
        else:
            raise ValueError("使用例と本体の対応を解決できません: " + first + " / " + second)
        include = '#include "../' + library + '"'
        if (ROOT / "Examples" / example).read_text().count(include) != 1:
            raise ValueError("使用例のincludeが一意でありません: " + example)
        result[example] = library
    return result


def integrate(paths: list[Path]) -> None:
    batches = [json.loads(path.read_text()) for path in paths]
    for batch in batches:
        for name, expected in batch.get("source_hashes", {}).items():
            if hashlib.sha256((ROOT / name).read_bytes()).hexdigest() != expected:
                raise ValueError("取り込み前にソースが変わりました: " + name)
    layout_path = ROOT / "Scripts/library_layout.json"
    layout = json.loads(layout_path.read_text())
    group_path = ROOT / "Verify/current/groups/expansion_20261031.json"
    group = json.loads(group_path.read_text()) if group_path.exists() else {"entries": []}
    known = {entry["id"]: entry for entry in group["entries"]}
    additions: list[str] = []
    documents: list[str] = []
    examples = {}
    entries = []
    for batch in batches:
        libraries = batch.get("libraries", batch.get("library_layout", batch.get("library_layout_additions", {})))
        for folder, names in libraries.items():
            for name in names:
                if name not in layout[folder]: layout[folder].append(name)
                additions.append(folder + "/" + name)
        entries.extend(batch.get("entries", batch.get("verify_entries", batch.get("verify_group_entries", []))))
        examples.update(normalize_examples(batch.get("examples", batch.get("package_example_map", {}))))
        documents.extend(batch.get("documents", [name for name in batch.get("source_hashes", {}) if name.startswith("Document/")]))
    for entry in entries:
        old = known.get(entry["id"])
        if old is None:
            group["entries"].append(entry); known[entry["id"]] = entry
        else:
            urls = list(dict.fromkeys(old.get("submission_urls", []) + entry.get("submission_urls", [])))
            old.update(entry)
            if urls: old["submission_urls"] = urls
    layout_path.write_text(json.dumps(layout, ensure_ascii=False, indent=2) + "\n")
    group_path.write_text(json.dumps(group, ensure_ascii=False, indent=2) + "\n")
    package = ROOT / "Scripts/package_revised_libraries.py"
    text = package.read_text()
    for example, library in examples.items():
        if '"' + example + '":' in text: continue
        position = text.index("\n}\n", text.index("LIBRARIES = {"))
        text = text[:position] + '\n    "' + example + '": "' + library + '",' + text[position:]
    package.write_text(text)
    index = ROOT / "Document/README.md"
    text = index.read_text()
    for document in documents:
        name = Path(document).name
        if "](" + name + ")" not in text:
            title = (ROOT / document).read_text().splitlines()[0].removeprefix("# ")
            text += "\n- [" + title + "](" + name + ")\n"
    index.write_text(text)
    for folder in {name.split("/")[0] for name in additions}:
        readme = ROOT / folder / "README.md"
        text = readme.read_text()
        rows = []
        for name in dict.fromkeys(additions):
            if not name.startswith(folder + "/"): continue
            file = Path(name).name
            if "](" + file + ")" in text: continue
            matching = [entry for entry in entries if name in entry["libraries"]]
            links = "・".join("[" + entry["title"] + "](../Verify/current/submissions/" + entry["id"] + ".cpp)" for entry in matching)
            rows.append("| [" + file + "](" + file + ") | " + links + " |")
        if rows:
            lines = text.splitlines(keepends=True)
            begin = next((i for i, line in enumerate(lines) if line.startswith("| ファイル |")), None)
            if begin is not None:
                end = begin
                while end < len(lines) and lines[end].lstrip().startswith("|"): end += 1
                lines[end:end] = [row + "\n" for row in rows]
                text = "".join(lines)
            else:
                text += "\n| ファイル | 対応する検証 |\n| --- | --- |\n" + "\n".join(rows) + "\n"
            readme.write_text(text)
    print(str(len(set(additions))) + "ライブラリ・" + str(len(entries)) + "Verifyを取り込みました。")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("metadata", type=Path, nargs="+")
    integrate(parser.parse_args().metadata)
