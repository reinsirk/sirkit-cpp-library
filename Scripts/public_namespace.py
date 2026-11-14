#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""貼付用のraw C++とsirkit名前空間版を、専用markerで可逆に変換する。"""
from __future__ import annotations

import base64
from dataclasses import dataclass
import re


_API_LINE = "// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。"
_HEADER = "\n" + _API_LINE + "\n"
_OPEN_LINE = "// SIRKIT_NAMESPACE_OPEN_V1"
_OPEN = "\n" + _OPEN_LINE + "\nnamespace sirkit {\n"
_CLOSE_LINE = "// SIRKIT_NAMESPACE_CLOSE_V1"
_CLOSE = "\n} // namespace sirkit\n" + _CLOSE_LINE + "\n"
_END_LINE = "// SIRKIT_NAMESPACE_END_V1"
_END = _END_LINE + "\n"
_REWRITE_START = "/*SIRKIT_NS_REWRITE_V1:"
_REWRITE_END = "/*SIRKIT_NS_REWRITE_END_V1*/"
_RAW_START = re.compile(r'(?:u8|u|U|L)?R"([^\s()\\]{0,16})\(')
_QUOTED_START = re.compile(r'(?:u8|u|U|L)?["\']')


@dataclass(frozen=True)
class _Token:
    kind: str
    start: int
    end: int
    text: str


@dataclass(frozen=True)
class _Directive:
    start: int
    end: int
    kind: str


def _line_end(text: str, position: int) -> int:
    """backslash-newlineを含む論理行の末尾を返す。"""
    while True:
        newline = text.find("\n", position)
        if newline < 0:
            return len(text)
        before = newline - 1
        if before >= 0 and text[before] == "\r":
            before -= 1
        if before < 0 or text[before] != "\\":
            return newline + 1
        position = newline + 1


def _tokens(text: str) -> list[_Token]:
    """コメント・文字列・数値内のnamespaceや#をコードと混同しない。"""
    result = []
    at = 0
    while at < len(text):
        if text[at].isspace():
            at += 1
            continue
        start = at
        if text.startswith("\\\n", at) or text.startswith("\\\r\n", at):
            at += 2 if text.startswith("\\\n", at) else 3
            continue
        if text.startswith("//", at):
            at = _line_end(text, at)
            end = at
            if end and text[end - 1] == "\n":
                end -= 1
                if end and text[end - 1] == "\r":
                    end -= 1
            result.append(_Token("comment", start, end, text[start:end]))
            continue
        if text.startswith("/*", at):
            end = text.find("*/", at + 2)
            if end < 0:
                raise ValueError("閉じていないC++コメントがあります")
            at = end + 2
            result.append(_Token("comment", start, at, text[start:at]))
            continue
        raw = _RAW_START.match(text, at)
        if raw:
            ending = ")" + raw[1] + '"'
            end = text.find(ending, raw.end())
            if end < 0:
                raise ValueError("閉じていないC++ raw文字列があります")
            at = end + len(ending)
            result.append(_Token("literal", start, at, text[start:at]))
            continue
        quoted = _QUOTED_START.match(text, at)
        if quoted:
            quote = quoted[0][-1]
            at = quoted.end()
            while at < len(text):
                if text[at] == "\\":
                    at += 2
                elif text[at] == quote:
                    at += 1
                    break
                else:
                    at += 1
            else:
                raise ValueError("閉じていないC++文字列があります")
            result.append(_Token("literal", start, at, text[start:at]))
            continue
        if text[at].isdigit() or (text[at] == "." and at + 1 < len(text) and text[at + 1].isdigit()):
            at += 1
            while at < len(text):
                char = text[at]
                if char.isalnum() or char in "_'." or (char in "+-" and text[at - 1] in "eEpP"):
                    at += 1
                else:
                    break
            result.append(_Token("number", start, at, text[start:at]))
            continue
        if text[at].isalpha() or text[at] == "_":
            at += 1
            while at < len(text) and (text[at].isalnum() or text[at] == "_"):
                at += 1
            result.append(_Token("identifier", start, at, text[start:at]))
            continue
        at += 2 if text.startswith("::", at) else 1
        result.append(_Token("symbol", start, at, text[start:at]))
    return result


def _directives(text: str, tokens: list[_Token]) -> list[_Directive]:
    result = []
    covered = last_code_end = 0
    for index, token in enumerate(tokens):
        if token.kind == "comment":
            continue
        line = text.rfind("\n", 0, token.start) + 1
        if token.text == "#" and token.start >= covered and last_code_end <= line:
            end = _line_end(text, token.start)
            next_code = None
            following = index + 1
            while following < len(tokens) and tokens[following].start < end:
                if tokens[following].kind != "comment":
                    next_code = tokens[following]
                    break
                following += 1
            kind = next_code.text if next_code else ""
            begin = line if not text[line:token.start].strip() else token.start
            result.append(_Directive(begin, end, kind))
            covered = end
        last_code_end = token.end
    return result


def _inside(position: int, ranges: list[_Directive]) -> bool:
    return any(d.start <= position < d.end for d in ranges)


def _open_position(text: str, tokens: list[_Token], directives: list[_Directive]) -> int:
    first = next((t for t in tokens if t.kind != "comment" and not _inside(t.start, directives)), None)
    if first is None:
        return len(text)
    stack = []
    for item in directives:
        if item.start >= first.start:
            break
        if item.kind in {"if", "ifdef", "ifndef"}:
            stack.append(item.start)
        elif item.kind == "endif":
            if not stack:
                raise ValueError("対応する#ifのない#endifがあります")
            stack.pop()
    if stack:
        return stack[0]
    line = text.rfind("\n", 0, first.start) + 1
    return line if not text[line:first.start].strip() else first.start


def _translate_fragment(fragment: str, kind: str) -> str:
    significant = [t for t in _tokens(fragment) if t.kind != "comment"]
    leading = bool(significant and significant[0].text == "::")
    if leading:
        significant = significant[1:]
    if [t.text for t in significant] != ["reinsirk", "::", "detail"] or kind not in {"D", "R"}:
        raise ValueError("namespace変換markerの元断片が不正です")
    a, scope, detail = significant
    gap0 = fragment[2:a.start] if leading else fragment[:a.start]
    gap1, gap2 = fragment[a.end:scope.start], fragment[scope.end:detail.start]
    if kind == "D":
        if leading:
            raise ValueError("namespace宣言の断片が不正です")
        return gap0 + gap1 + gap2 + "detail"
    return gap0 + "::sirkit" + gap1 + "::" + gap2 + "detail"


def _namespace_edits(text: str, tokens: list[_Token], directives: list[_Directive]) -> list[tuple[int, int, str]]:
    code = [t for t in tokens if t.kind != "comment"]
    includes = [d for d in directives if d.kind in {"include", "include_next"}]
    edits = []
    for index in range(len(code) - 2):
        first, scope, detail = code[index:index + 3]
        if [first.text, scope.text, detail.text] != ["reinsirk", "::", "detail"] or _inside(first.start, includes):
            continue
        previous = code[index - 1] if index else None
        before_namespace = code[index - 2] if index > 1 else None
        declaration = previous and previous.text == "namespace" and not (before_namespace and before_namespace.text == "using")
        kind = "D" if declaration else "R"
        start = first.start
        if previous and previous.text == "::":
            before = code[index - 2] if index > 1 else None
            # 別のnamespace内のns::reinsirk::detailは、旧global helperとは区別する。
            keywords = {"return", "co_return", "throw", "typename", "sizeof", "alignof", "new", "delete"}
            if before and ((before.kind == "identifier" and before.text not in keywords) or before.text in {">", ")", "]"}):
                continue
            start = previous.start
        original = text[start:detail.end]
        replacement = _translate_fragment(original, kind)
        payload = base64.b64encode(original.encode("utf-8")).decode("ascii")
        marked = _REWRITE_START + kind + ":" + payload + "*/" + replacement + _REWRITE_END
        edits.append((start, detail.end, marked))
    return edits


def _apply(text: str, edits: list[tuple[int, int, str]]) -> str:
    edits.sort(key=lambda item: (item[0], item[1]))
    result = []
    cursor = 0
    for begin, end, replacement in edits:
        if begin < cursor or not (begin <= end <= len(text)):
            raise ValueError("namespace編集の位置が重複しています")
        result += [text[cursor:begin], replacement]
        cursor = end
    result.append(text[cursor:])
    return "".join(result)


def _reserved(token: _Token) -> bool:
    return token.kind == "comment" and (token.text in {_API_LINE, _OPEN_LINE, _CLOSE_LINE, _END_LINE, _REWRITE_END}
                                         or token.text.startswith(_REWRITE_START))


def unwrap_source(text: str) -> str:
    """専用markerだけを除去し、rawをbyte単位で復元する。marker編集は失敗とする。"""
    tokens = _tokens(text)
    headers = [t for t in tokens if t.kind == "comment" and t.text == _API_LINE]
    markers = [t for t in tokens if _reserved(t)]
    if not headers:
        if markers:
            raise ValueError("不完全なnamespace markerがあります")
        return text
    if len(headers) != 1:
        raise ValueError("一つのソースに複数のnamespace wrapperがあります")
    ends = [t for t in markers if t.text == _END_LINE]
    if len(ends) != 1 or text[ends[0].start:] != _END:
        raise ValueError("namespace wrapperの終端が不正です")
    edits = []
    opens = closes = 0
    index = 0
    while index < len(tokens):
        token = tokens[index]
        if token.kind != "comment":
            index += 1
            continue
        if token.text == _API_LINE:
            begin, end = token.start - 1, token.end + 1
            if text[begin:end] != _HEADER:
                raise ValueError("namespace API markerが編集されています")
            edits.append((begin, end, ""))
        elif token.text == _OPEN_LINE:
            begin = token.start - 1
            end = begin + len(_OPEN)
            if text[begin:end] != _OPEN:
                raise ValueError("namespace open markerが編集されています")
            edits.append((begin, end, ""))
            opens += 1
        elif token.text == _CLOSE_LINE:
            end = token.end + 1
            begin = end - len(_CLOSE)
            if text[begin:end] != _CLOSE:
                raise ValueError("namespace close markerが編集されています")
            edits.append((begin, end, ""))
            closes += 1
        elif token.text == _END_LINE:
            edits.append((token.start, token.end + 1, ""))
        elif token.text.startswith(_REWRITE_START):
            payload = token.text[len(_REWRITE_START):-2]
            try:
                kind, encoded = payload.split(":", 1)
                original = base64.b64decode(encoded, validate=True).decode("utf-8")
                expected = _translate_fragment(original, kind)
            except (ValueError, UnicodeError) as error:
                raise ValueError("namespace rewrite markerが不正です") from error
            finish = index + 1
            while finish < len(tokens) and not (tokens[finish].kind == "comment" and tokens[finish].text == _REWRITE_END):
                if _reserved(tokens[finish]):
                    raise ValueError("namespace rewrite markerが入れ子です")
                finish += 1
            if finish == len(tokens) or text[token.end:tokens[finish].start] != expected:
                raise ValueError("変換済みnamespace断片が編集されています")
            edits.append((token.start, tokens[finish].end, original))
            index = finish
        elif token.text == _REWRITE_END:
            raise ValueError("対応する開始のないnamespace rewrite markerがあります")
        index += 1
    if not opens or opens != closes:
        raise ValueError("namespace open/close markerが対応しません")
    return _apply(text, edits)


def wrap_source(text: str) -> str:
    """公開宣言をsirkitへ入れ、標準includeと元の共有guardを維持する。"""
    tokens = _tokens(text)
    if any(_reserved(t) for t in tokens):
        unwrap_source(text)  # 不完全なwrapperを正常扱いしない。
        return text
    directives = _directives(text, tokens)
    opening = _open_position(text, tokens, directives)
    first = next((t for t in tokens if t.kind != "comment"), None)
    header = first.start if first else len(text)
    if first:
        line = text.rfind("\n", 0, first.start) + 1
        if not text[line:first.start].strip():
            header = line
    edits = _namespace_edits(text, tokens, directives)
    # 同じ位置ではAPI説明→namespace openの順にする。
    edits += [(header, header, _HEADER), (opening, opening, _OPEN)]
    for item in directives:
        if item.kind in {"include", "include_next"} and item.start >= opening:
            edits += [(item.start, item.start, _CLOSE), (item.end, item.end, _OPEN)]
    return _apply(text, edits) + _CLOSE + _END
