# SPDX-License-Identifier: CC0-1.0
"""登録済みライブラリの通常のquoted includeを、選んだprofileの内容へ展開する。"""
from __future__ import annotations
import re
from collections.abc import Callable, Collection


def expand_example_dependencies(text: str, owner: str, known_libraries: Collection[str],
                                load_source: Callable[[str], str], *,
                                max_output_chars: int = 16 * 1024 * 1024) -> str:
    """filesystemは読まない。各includeを保持して展開し、macro条件とguardはC++に任せる。"""
    def normalize(path: str) -> str:
        if not path or path.startswith('/') or '\\' in path or ':' in path or '\x00' in path:
            raise ValueError('許可しないinclude path: ' + repr(path))
        parts = []
        for part in path.split('/'):
            if part in ('', '.'):
                continue
            if part == '..':
                if not parts:
                    raise ValueError('project外のinclude path: ' + repr(path))
                parts.pop()
            else:
                parts.append(part)
        return '/'.join(parts)

    known = set(known_libraries)
    if any(normalize(name) != name for name in known):
        raise ValueError('library pathが正規形ではありません')
    owner = normalize(owner)
    size = len(text)
    if size > max_output_chars:
        raise ValueError('Example展開の容量上限')
    cache: dict[str, str] = {}

    def lexical_mask(source: str) -> str:
        # コメントは空白、文字/文字列はxにする。raw文字列内の#includeも指令にしない。
        mask = list(source)
        def hide(begin: int, end: int, marker: str) -> None:
            for index in range(begin, end):
                if source[index] not in '\r\n':
                    mask[index] = marker
        i, n = 0, len(source)
        while i < n:
            if source.startswith('//', i):
                end = source.find('\n', i + 2)
                while end >= 0 and (source[end-1:end] == '\\' or source[end-2:end] == '\\\r'):
                    end = source.find('\n', end + 1)
                if end < 0:
                    end = n
                hide(i, end, ' '); i = end; continue
            if source.startswith('/*', i):
                end = source.find('*/', i + 2)
                end = n if end < 0 else end + 2
                hide(i, end, ' '); i = end; continue
            if source.startswith('R"', i):
                opening = source.find('(', i + 2, min(n, i + 19))
                if opening >= 0:
                    delimiter = source[i+2:opening]
                    if not any(c.isspace() or c in '()\\' for c in delimiter):
                        closing = source.find(')' + delimiter + '"', opening + 1)
                        end = n if closing < 0 else closing + len(delimiter) + 2
                        hide(i, end, 'x'); i = end; continue
            if source[i] in '\'"':
                if source[i] == "'":
                    begin = i
                    while begin and (source[begin-1].isalnum() or source[begin-1] in "_.'"):
                        begin -= 1
                    token = source[begin:i]
                    if token and (token[0].isdigit() or token.startswith('.')):
                        i += 1; continue
                quote, end = source[i], i + 1
                while end < n:
                    if source[end] == '\\':
                        end += 2
                    elif source[end] == quote:
                        end += 1; break
                    else:
                        end += 1
                end = min(end, n)
                hide(i, end, 'x'); i = end; continue
            i += 1
        return ''.join(mask)

    def walk(source: str, current: str, stack: tuple[str, ...]) -> str:
        nonlocal size
        result = []
        physical_lines = source.split('\n')
        masked_lines = lexical_mask(source).split('\n')
        for index, (line, masked) in enumerate(zip(physical_lines, masked_lines)):
            if index + 1 < len(physical_lines):
                line += '\n'
                masked += '\n'
            directive = re.match(r'^([ \t]*)#[ \t]*include\b', masked)
            if not directive:
                result.append(line); continue
            literal = re.match(r'[ \t]*"([^"\r\n]+)"', line[directive.end():])
            if not literal:
                result.append(line); continue
            end = directive.end() + literal.end()
            if masked[end:].strip():
                result.append(line); continue
            name = literal.group(1)
            # 正規化をjoin前に行わず、親のdirectoryに対する..を解決する。
            if name.startswith('/') or '\\' in name or ':' in name or '\x00' in name:
                raise ValueError('許可しないinclude path: ' + repr(name))
            parent = current.rpartition('/')[0]
            target = normalize((parent + '/' if parent else '') + name)
            if target not in known:
                result.append(line); continue
            if target in stack:
                raise ValueError('循環するlibrary include: ' + ' -> '.join((*stack, target)))
            if target not in cache:
                cache[target] = load_source(target)
                if not isinstance(cache[target], str):
                    raise TypeError('load_sourceはstrを返す必要があります')
            body = cache[target]
            begin = len(directive.group(1))
            size += len(body) - (end - begin)
            if size > max_output_chars:
                raise ValueError('Example展開の容量上限')
            result.append(line[:begin] + walk(body, target, (*stack, target)) + line[end:])
        return ''.join(result)

    return walk(text, owner, ())
