#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""凹min-plus畳み込みの単独貼付版へ共通SMAWKを同期する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source


def render(root: Path) -> bytes:
    source = unwrap_source((root / 'Algorithm/Monge_Optimization.cpp').read_text())
    include_start = source.index('#include ')
    notes, overview = source[:include_start].split('// API一覧\n', 1)
    references = '\n'.join(line for line in notes.splitlines() if line.startswith('// 参考:'))
    directory = root / 'Algorithm/Concave_Min_Plus_Convolution/components'
    header = (directory / 'header.inc').read_text().rstrip()
    body = (directory / 'body.inc').read_text().rstrip()
    data = '\n\n'.join((header + '\n//\n// [同梱の行最小値探索]\n' + references + '\n' + overview.strip(),
                        source[include_start:].rstrip(), body)) + '\n'
    return wrap_source(data).encode()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    options = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = root / 'Algorithm/Concave_Min_Plus_Convolution.cpp'
    data = render(root)
    if options.check:
        if not output.is_file() or output.read_bytes() != data:
            raise ValueError(output.name + 'が共通SMAWKと同期していません')
    else:
        output.write_bytes(data)
    print('凹min-plus畳み込み: ' + ('同期確認済み' if options.check else '生成済み'))


if __name__ == '__main__':
    main()
