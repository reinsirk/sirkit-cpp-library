#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""BinaryMatrixの単独貼付版へDynamicBitsetの内部word kernelを同期する。"""
from __future__ import annotations
import argparse
from pathlib import Path

def render(root: Path) -> bytes:
    directory = root / 'Math/BinaryMatrix/components'
    header = (directory / 'binary_matrix_header.inc').read_text()
    bitset = (root / 'DataStructure/Dynamic_Bitset.cpp').read_text()
    start = bitset.index('#include ')
    end = bitset.index('#ifndef SIRKIT_DYNAMIC_BITSET_DEFINED')
    payload = bitset[start:end]
    if 'SIRKIT_DYNAMIC_BITSET_WORD_OPERATIONS_DEFINED' not in payload:
        raise ValueError('共有bit kernelのguardが見つかりません')
    body = (directory / 'binary_matrix_body.inc').read_text()
    return (header.rstrip() + '\n\n' + payload.rstrip() + '\n\n' + body.rstrip() + '\n').encode()

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = root / 'Math/Binary_Matrix.cpp'
    data = render(root)
    if args.check:
        if not output.is_file() or output.read_bytes() != data:
            raise ValueError('Binary_Matrix.cppが共有bit kernel・部品と同期していません')
    else:
        output.write_bytes(data)
    print('GF(2)行列: ' + ('同期確認済み' if args.check else '生成済み'))

if __name__ == '__main__':
    main()
