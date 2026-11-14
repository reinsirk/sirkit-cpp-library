#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""コンパイラ分岐だけを部分評価し、通常版と検査省略版を生成する。"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
FOLDERS = ("Algorithm", "DataStructure", "Graph", "Math", "String", "Geometry", "Internal")


@dataclass(frozen=True)
class Expression:
    value: bool | None
    text: str


try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def strip_outer(text: str) -> str:
    while text.startswith("(") and text.endswith(")"):
        depth = 0
        for index, char in enumerate(text):
            depth += (char == "(") - (char == ")")
            if depth == 0 and index != len(text) - 1:
                return text
        if depth != 0:
            return text
        text = text[1:-1].strip()
    return text


def split_top(text: str, operator: str) -> list[str]:
    depth = 0
    parts: list[str] = []
    begin = 0
    index = 0
    while index < len(text):
        char = text[index]
        depth += (char == "(") - (char == ")")
        if depth == 0 and text.startswith(operator, index):
            parts.append(text[begin:index].strip())
            index += len(operator)
            begin = index
        else:
            index += 1
    if parts:
        parts.append(text[begin:].strip())
    return parts


def simplify(text: str) -> Expression:
    text = strip_outer(text.strip())
    if text in ("0", "1"):
        return Expression(text == "1", text)
    # 条件演算子は論理和より優先順位が低い。ここでは分解せず保存する。
    if "?" in text:
        return Expression(None, text)
    for operator in ("||", "&&"):
        parts = split_top(text, operator)
        if not parts:
            continue
        items = [simplify(part) for part in parts]
        decisive = operator == "||"
        if any(item.value is decisive for item in items):
            return Expression(decisive, "1" if decisive else "0")
        unknown = [item for item in items if item.value is None]
        if not unknown:
            return Expression(not decisive, "0" if decisive else "1")
        if len(unknown) == 1:
            return unknown[0]
        return Expression(None, f" {operator} ".join("(" + item.text + ")" for item in unknown))
    if text.startswith("!") and not text.startswith("!="):
        operand = text[1:].strip()
        # !A == Bのような式を!(A == B)へ変えてはならない。
        if not (re.fullmatch(r"[A-Za-z_]\w*|[01]", operand) or
                (operand.startswith("(") and strip_outer(operand) != operand)):
            return Expression(None, text)
        item = simplify(operand)
        if item.value is not None:
            return Expression(not item.value, "0" if item.value else "1")
        return Expression(None, "!(" + item.text + ")")
    return Expression(None, text)


def evaluate(text: str, known: dict[str, bool]) -> Expression:
    text = re.sub(r"/\*.*?\*/", " ", text)
    text = text.split("//", 1)[0].strip()
    def defined(match: re.Match[str]) -> str:
        name = match[1] or match[2]
        return str(int(known[name])) if name in known else match[0]
    replaced = re.sub(r"\bdefined\s*(?:\(\s*([A-Za-z_]\w*)\s*\)|([A-Za-z_]\w*))", defined, text)
    if replaced == text:
        return Expression(None, text)
    return simplify(replaced)


@dataclass
class Branch:
    directive: str
    condition: str | None
    body: list


@dataclass
class Conditional:
    branches: list[Branch]
    ending: str


def logical_lines(source: str) -> list[str]:
    result: list[str] = []
    current = ""
    for line in source.splitlines(keepends=True):
        current += line
        if not line.rstrip("\r\n").endswith("\\"):
            result.append(current)
            current = ""
    if current:
        result.append(current)
    return result


def directive(line: str) -> tuple[str, str]:
    match = re.match(r"\s*#\s*(\w+)(.*)", line.replace("\\\n", " ").replace("\\\r\n", " "), re.S)
    return (match[1], match[2].strip()) if match else ("", "")


def parse(source: str) -> list:
    lines = logical_lines(source)
    position = 0
    def body(nested: bool = False) -> list:
        nonlocal position
        result: list = []
        while position < len(lines):
            line = lines[position]
            name, argument = directive(line)
            if name in ("elif", "else", "endif"):
                if not nested:
                    raise ValueError("対応しない条件終端: " + line.strip())
                return result
            position += 1
            if name not in ("if", "ifdef", "ifndef"):
                result.append(line)
                continue
            guard = re.sub(r"/\*.*?\*/", " ", argument).split("//", 1)[0].strip()
            condition = (argument if name == "if" else
                         ("!" if name == "ifndef" else "") + "defined(" + guard + ")")
            branches = [Branch(line, condition, body(True))]
            while position < len(lines):
                next_line = lines[position]
                next_name, next_argument = directive(next_line)
                position += 1
                if next_name == "endif":
                    result.append(Conditional(branches, next_line))
                    break
                if next_name not in ("elif", "else"):
                    raise ValueError("条件分岐の構造が不正です")
                if branches[-1].condition is None:
                    raise ValueError("elseの後に分岐があります")
                branches.append(Branch(next_line, next_argument if next_name == "elif" else None, body(True)))
            else:
                raise ValueError("条件終端がありません")
        return result
    return body()


def specialize(source: str, compiler: str, fast: bool = False) -> str:
    original = source
    source = unwrap_source(source)
    wrapped = original != source
    if compiler not in ("GCC", "Clang"):
        raise ValueError("GCCまたはClangを指定してください")
    known = {"__GNUC__": True, "__GNUG__": True, "__clang__": compiler == "Clang",
             "__llvm__": compiler == "Clang"}
    if fast:
        known["SKIP_RUNTIME_CHECKS"] = True
    for name in known:
        updates = re.findall(r"^\s*#\s*(define|undef)\s+" + re.escape(name) + r"\b([^\n]*)", source, re.M)
        if name == "SKIP_RUNTIME_CHECKS" and all(kind == "define" and not value.strip() for kind, value in updates):
            continue
        if updates:
            raise ValueError("特殊化するマクロをソース内で変更しています: " + name)
    def render(nodes: list) -> str:
        pieces: list[str] = []
        for node in nodes:
            if isinstance(node, str):
                pieces.append(node)
                continue
            emitted = False
            for branch in node.branches:
                expression = Expression(True, "1") if branch.condition is None else evaluate(branch.condition, known)
                if expression.value is False:
                    continue
                if expression.value is True:
                    if emitted:
                        pieces.append("#else\n")
                    pieces.append(render(branch.body))
                    break
                if emitted:
                    pieces.append("#elif " + expression.text + "\n")
                elif expression.text == branch.condition and directive(branch.directive)[0] != "elif":
                    pieces.append(branch.directive)
                else:
                    pieces.append("#if " + expression.text + "\n")
                pieces.append(render(branch.body))
                emitted = True
            if emitted:
                pieces.append(node.ending)
        return "".join(pieces)
    result = render(parse(source))
    return wrap_source(result) if wrapped else result


def sources(root: Path) -> list[Path]:
    return sorted([path for folder in FOLDERS for path in (root / folder).rglob("*.cpp")]
                  + [root / name for name in ("temp.cpp", "temp2.cpp")])


def generate(root: Path, check: bool = False) -> dict:
    manifest = {"format": 1, "generated_by": "Scripts/generate_variants.py", "entries": []}
    overrides_path = root / "Scripts/fast_overrides.json"
    overrides = json.loads(overrides_path.read_text()) if overrides_path.exists() else {}
    expected: set[str] = set()
    for path in sources(root):
        relative = path.relative_to(root).as_posix()
        for compiler in ("GCC", "Clang"):
            for fast in (False, True):
                origin = root / overrides[relative] if fast and relative in overrides else path
                data = specialize(origin.read_text(), compiler, fast)
                note = "// 生成版: " + compiler + ("・速度優先（入力検査を除外）。" if fast else "用。")
                if fast:
                    note += " 所有・解放の省略は各APIの前提を参照。"
                lines = data.splitlines(keepends=True)
                index = 1 if lines and lines[0].startswith("// SPDX-License-Identifier:") else 0
                lines.insert(index, note + "\n// 生成元: " + origin.relative_to(root).as_posix() + "\n")
                encoded = "".join(lines).encode()
                target = ("Fast/" if fast else "Variants/") + compiler + "/" + relative
                output = root / target
                expected.add(target)
                if check:
                    if not output.is_file() or output.read_bytes() != encoded:
                        raise ValueError("生成版が同期していません: " + target)
                else:
                    output.parent.mkdir(parents=True, exist_ok=True)
                    output.write_bytes(encoded)
                manifest["entries"].append({"source": relative, "origin": origin.relative_to(root).as_posix(),
                                            "compiler": compiler, "fast": fast, "path": target,
                                            "source_sha256": hashlib.sha256(origin.read_bytes()).hexdigest(),
                                            "sha256": hashlib.sha256(encoded).hexdigest(), "bytes": len(encoded)})
    for folder in ("Variants/GCC", "Variants/Clang", "Fast/GCC", "Fast/Clang"):
        candidates = [path for category in FOLDERS for path in (root / folder / category).rglob("*.cpp")]
        candidates.extend((root / folder).glob("*.cpp"))
        for output in candidates:
            if output.relative_to(root).as_posix() not in expected:
                raise ValueError("元にない生成版があります（自動削除しません）: " + str(output))
    manifest_path = root / "Variants/manifest.json"
    encoded = (json.dumps(manifest, ensure_ascii=False, indent=2) + "\n").encode()
    if check:
        if manifest_path.read_bytes() != encoded:
            raise ValueError("生成版manifestが同期していません")
    else:
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_bytes(encoded)
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    options = parser.parse_args()
    result = generate(ROOT, options.check)
    print(f"{len(result['entries'])}件のコンパイラ別・速度優先版: " + ("同期確認" if options.check else "生成"))


if __name__ == "__main__":
    main()
