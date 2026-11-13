# SPDX-License-Identifier: CC0-1.0
"""Keep flattened distribution documentation links usable."""
from __future__ import annotations

import pathlib
import posixpath
import re
from urllib.parse import quote, unquote, urlsplit, urlunsplit


LINK = re.compile(r'\[([^\]\n]+)\]\((<[^>\n]+>|(?:\\.|[^()\n]|\([^()\n]*\))+)\)')


def verify_index(entries, compiler, fast):
    title = compiler + (' / Fast' if fast else '')
    lines = ['# Verify / ' + title, '', 'この配布版に含まれる提出用コードの一覧です。',
             '通常版のオンライン判定をこの版の結果へ転記していません。', '']
    for entry in entries:
        name = entry['id']
        if not re.fullmatch(r'[A-Za-z0-9_]+', name):
            raise ValueError('invalid Verify ID')
        lines.append('- [' + name + '](submissions/' + name + '.cpp)')
    return ('\n'.join(lines) + '\n').encode()


def repair_flattened_markdown(contents, root, *, fast=False,
                              repository='https://github.com/reinsirk/sirkit-cpp-library'):
    """Present relative links stay local; omitted normal material links upstream.

    This helper handles the inline destination forms currently used in the
    repository, including one nested pair of parentheses in a filename. Other
    forms stay unchanged and the independent ZIP validator can reject them.
    """
    root = pathlib.Path(root)
    names = set(contents)
    repaired = []
    for owner in list(contents):
        if not owner.endswith('.md'):
            continue
        source = contents[owner].decode()
        if owner in ('Document/fast_variants.md', 'Document/bundling.md'):
            heading, separator, rest = source.partition('\n')
            note = '\nこのZIPは選択した版を直下に配置しています。以下のツール・生成コマンドは[元リポジトリ全体](' + repository + ')で実行します。\n'
            source = heading + separator + note + rest
        lines = []
        fence = None
        for line in source.splitlines(keepends=True):
            marker = re.match(r'^ {0,3}(`{3,}|~{3,})', line)
            if marker:
                value = marker.group(1)
                if fence is None:
                    fence = value
                elif value[0] == fence[0] and len(value) >= len(fence):
                    fence = None
                lines.append(line)
                continue
            if fence is not None:
                lines.append(line)
                continue
            protected = [m.span() for m in re.finditer(r'(`+).*?\1', line)]
            def replace(match):
                if any(begin < match.end() and match.start() < end for begin,end in protected):
                    return match.group()
                label, destination = match.groups()
                destination = destination.strip('<>')
                parsed = urlsplit(destination)
                if parsed.scheme or parsed.netloc or not parsed.path:
                    return match.group()
                target = posixpath.normpath(posixpath.join(posixpath.dirname(owner), unquote(parsed.path).replace('\\(', '(').replace('\\)', ')')))
                if target.startswith('../') or target.startswith('/'):
                    raise ValueError('Markdown target escapes package: ' + destination)
                if target in names or any(name.startswith(target.rstrip('/') + '/') for name in names):
                    return match.group()
                original = root / target
                if not original.exists():
                    raise FileNotFoundError('missing Markdown target in repository: ' + target)
                kind = 'tree' if original.is_dir() else 'blob'
                path = '/' + kind + '/main/' + quote(target, safe='/')
                remote = repository + urlunsplit(('', '', path, parsed.query, parsed.fragment))
                if target.startswith('Verify/'):
                    label += '（通常版の補足・元リポジトリ）'
                else:
                    label += '（元リポジトリ）'
                repaired.append(dict(owner=owner, target=target, remote=remote))
                return '[' + label + '](' + remote + ')'
            lines.append(LINK.sub(replace, line))
        contents[owner] = ''.join(lines).encode()
    return repaired
