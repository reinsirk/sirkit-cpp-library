#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""形式的冪級数の単独貼付版へ共通NTTを同期する。"""
from __future__ import annotations
import argparse
from pathlib import Path

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def render(root: Path) -> bytes:
    directory = root / 'Math/FPS/components'
    header = (directory / 'formal_power_series_header.inc').read_text()
    ntt = unwrap_source((root / 'Math/NTT.cpp').read_text())
    payload = ntt[ntt.index('#include '):]
    overview = ntt[:ntt.index('#include ')].split('// API一覧\n', 1)[1].strip()
    body = (directory / 'formal_power_series_body.inc').read_text()
    return wrap_source(header.rstrip() + '\n//\n// [同梱NTT]\n' + overview + '\n\n' +
            payload.rstrip() + '\n\n' + body.rstrip() + '\n').encode()

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = root / 'Math/Formal_Power_Series.cpp'
    data = render(root)
    if args.check:
        if not output.is_file() or output.read_bytes() != data:
            raise ValueError('Formal_Power_Series.cppが共通NTT・部品と同期していません')
    else:
        output.write_bytes(data)
    print('形式的冪級数: ' + ('同期確認済み' if args.check else '生成済み'))

if __name__ == '__main__':
    main()
