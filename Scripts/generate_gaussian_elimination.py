#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""体上の消去の単独貼付版へ共通Matrixを同期する。"""
from pathlib import Path
import argparse

def render(root: Path) -> bytes:
    matrix=(root/'Math/Matrix.cpp').read_text()
    directory=root/'Math/Matrix/components'
    header=(directory/'gaussian_header.inc').read_text().rstrip()
    overview=matrix[:matrix.index('#include ')].split('// API一覧\n',1)[1].strip()
    body=(directory/'gaussian_body.inc').read_text().rstrip()
    return (header+'\n//\n// [同梱Matrix]\n'+overview+'\n\n'+matrix[matrix.index('#include '):].rstrip()+'\n\n'+body+'\n').encode()

def main() -> None:
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    root=Path(__file__).resolve().parents[1];output=root/'Math/Gaussian_Elimination.cpp';data=render(root)
    if args.check:
        if not output.exists()or output.read_bytes()!=data:raise ValueError('Gaussian_Elimination.cppが共通Matrix・部品と同期していません')
    else:output.write_bytes(data)
    print('体上の消去: '+('同期確認済み'if args.check else'生成済み'))
if __name__=='__main__':main()
