#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""多点評価・補間の単独貼付版へ共通FPS/NTTを同期する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def render(root: Path) -> dict[Path,bytes]:
    parts=root/'Math/Polynomial/components';fps=unwrap_source((root/'Math/Formal_Power_Series.cpp').read_text())
    overview=fps[:fps.index('#include ')].split('// API一覧\n',1)[1].strip()
    embedded = fps[fps.index('#include '):].rstrip()
    sequences = root / 'Math/CombinatorialSequences/components'
    sums = root / 'Math/PolynomialPrefixSum/components'
    newton_basis = root / "Math/Newton_Basis/components"
    sources = {
        root / 'Math/Polynomial.cpp': (parts / 'polynomial_header.inc', parts / 'polynomial_body.inc'),
        root / 'Math/Combinatorial_Sequences.cpp': (sequences / 'header.inc', sequences / 'body.inc'),
        root / 'Math/Polynomial_Prefix_Sum.cpp': (sums / 'header.inc', sums / 'body.inc'),
        root / 'Math/Newton_Basis.cpp': (newton_basis / 'header.inc', newton_basis / 'body.inc'),
    }
    result = {}
    for output, (header_part, body_part) in sources.items():
        header = header_part.read_text().rstrip() + '\n//\n// [同梱FPS/NTT]\n' + overview
        data = header + '\n\n' + embedded + '\n\n' + body_part.read_text().rstrip() + '\n'
        result[output] = wrap_source(data).encode()
    return result

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    for path,data in render(Path(__file__).resolve().parents[1]).items():
        if args.check:
            if not path.exists()or path.read_bytes()!=data:raise ValueError(str(path)+'が共通部品と同期していません')
        else:path.write_bytes(data)
    print('多項式: '+('同期確認済み'if args.check else'生成済み'))
if __name__=='__main__':main()
