#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""体上の消去の単独貼付版へ共通Matrixを同期する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def render(root: Path) -> bytes:
    matrix=unwrap_source((root/'Math/Matrix.cpp').read_text())
    directory=root/'Math/Matrix/components'
    header=(directory/'gaussian_header.inc').read_text().rstrip()
    overview=matrix[:matrix.index('#include ')].split('// API一覧\n',1)[1].strip()
    body=(directory/'gaussian_body.inc').read_text().rstrip()
    return wrap_source(header+'\n//\n// [同梱Matrix]\n'+overview+'\n\n'+matrix[matrix.index('#include '):].rstrip()+'\n\n'+body+'\n').encode()


def render_adjugate(root: Path, gaussian: bytes) -> bytes:
    canonical = unwrap_source(gaussian.decode())
    include_start = canonical.index('#include ')
    overview = canonical[:include_start].split('// API一覧\n', 1)[1].strip()
    directory = root / 'Math/AdjugateMatrix/components'
    header = (directory / 'header.inc').read_text().rstrip()
    body = (directory / 'body.inc').read_text().rstrip()
    data = f"{header}\n//\n// [同梱の消去とMatrix]\n{overview}\n\n{canonical[include_start:].rstrip()}\n\n{body}\n"
    return wrap_source(data).encode()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    gaussian = render(root)
    outputs = {
        root / 'Math/Gaussian_Elimination.cpp': gaussian,
        root / 'Math/Adjugate_Matrix.cpp': render_adjugate(root, gaussian),
    }
    for output, data in outputs.items():
        if args.check:
            if not output.exists() or output.read_bytes() != data:
                raise ValueError(output.name + 'が共通Matrix・部品と同期していません')
        else:
            output.write_bytes(data)
    print('体上の消去・随伴行列: ' + ('同期確認済み' if args.check else '生成済み'))


if __name__ == '__main__':
    main()
